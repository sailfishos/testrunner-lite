/* * This file is part of testrunner-lite *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 *
 * Contact: Sami Lahtinen <ext-sami.t.lahtinen@nokia.com>
 *
 * This software, including documentation, is protected by copyright
 * controlled by Nokia Corporation. All rights are reserved. Copying,
 * including reproducing, storing, adapting or translating, any or all of
 * this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be
 * disclosed to others without the prior written consent of Nokia.
 *
 */

/* ------------------------------------------------------------------------- */
/* INCLUDE FILES */
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500	/* required by getpgid() in unistd.h */
#endif
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>  		/* dup, pipe, fork, close, execvp */
#include <string.h>		/* strtok, strncpy */
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include "remote_executor.h"
#include "executor.h"
#include "log.h"

/* ------------------------------------------------------------------------- */
/* EXTERNAL DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* EXTERNAL GLOBAL VARIABLES */
/* None */

/* ------------------------------------------------------------------------- */
/* EXTERNAL FUNCTION PROTOTYPES */
/* None */

/* ------------------------------------------------------------------------- */
/* GLOBAL VARIABLES */
/* None */

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL GLOBAL VARIABLES */
static volatile sig_atomic_t timer_value = 0;
static struct sigaction default_alarm_action = { .sa_handler = NULL };
static char *remote_host = NULL;

/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
#if 0
static void parse_command_args(const char* command, char* argv[], int max_args);
static void free_args(char* argv[]);
#endif
static pid_t fork_process_redirect(int* stdout_fd, int* stderr_fd, 
				   const char *command);
static pid_t fork_process(const char *command);
static void* stream_data_realloc(stream_data* data, int size);
static void stream_data_free(stream_data* data);
static void stream_data_append(stream_data* data, char* src);
static int read_and_append(int fd, stream_data* data);
static void timer_handler(int signum);
static int set_timer(long secs);
static void reset_timer();
static int execution_terminated(exec_data* data);
static void process_output_streams(int stdout_fd, int stderr_fd, 
				   exec_data* data);
static void communicate(int stdout_fd, int stderr_fd, exec_data* data);
static void strip_ctrl_chars (stream_data* data);
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
#if 0
/** Parse a command
 * @param command Command string to parse
 * @param argv Array of strings to store parsed arguments
 * @param max_args Maximum number of arguments to parse
 */
static void parse_command_args(const char* command, char* argv[], int max_args)
{
	int n = 0;
	int size = strlen(command) + 1;
	char* buff = NULL;
	char* str = NULL;

	buff = (char*)malloc(size);

	if (buff != NULL) {
		/* command must be copied because strtok modifies its
		 * first argument 
		 */
		strncpy(buff, command, size);
		buff[size - 1] = '\0';

		/* split command to tokens delimited by space character */
		str = strtok(buff, " ");

		for ( ;str != NULL && n < max_args; n++) {
			argv[n] = (char*)malloc(strlen(str) + 1);
			if (argv[n]) {
				strcpy(argv[n], str);
			}
			str = strtok(NULL, " ");
		}
	}
	argv[n] = NULL;

	free(buff);
}

/** Free argument strings
 * @param argv Array of argument strings to free
 */
static void free_args(char* argv[]) {
	while(*argv != NULL) {
		free(*argv);
		argv++;
	}
}
#endif
/** Executes a command using /bin/sh -c
 * @param command Command to execute
 * @return Does not return in success, error code from exec in case of error
 */
static int exec_wrapper(const char *command) 
{
	int ret = 0;

	if (remote_host)
		ret = ssh_execute (remote_host, command);
	else
		/* on success, execvp does not return */
		ret = execl(SHELLCMD, SHELLCMD, SHELLCMD_ARG1, 
			    command, (char*)NULL);

	return ret;
}

/** Create new process with new session ID and redirect its output
 * @param stdout_fd Pointer to a file descriptor used to read stdout of
 *        executed command
 * @param stderr_fd Pointer to a file descriptor used to read stderr of 
 *        executed command
 * @param command Command to execute
 * @return PID of process on success, -1 in error
 */
static pid_t fork_process_redirect(int* stdout_fd, int* stderr_fd, const char *command) {
	int out_pipe[2];
	int err_pipe[2];
	pid_t pid;

	if (pipe(out_pipe) < 0)
		goto error_out;

	if (pipe(err_pipe) < 0)
		goto error_err;

	pid = fork();
	if (pid > 0) { /* parent */
		LOG_MSG(LOG_DEBUG, "Forked new process %d", pid);
		/* close the write end of the pipes */
		close(out_pipe[1]);
		close(err_pipe[1]);
		*stdout_fd = out_pipe[0];
		*stderr_fd = err_pipe[0];
	} else if (pid == 0) { /* child */
		/* Create new session id.
		 * Process group ID and session ID
		 * are set to PID (they were PPID) */
		setsid();

		/* close the read end of the pipes */
		close(out_pipe[0]);
		close(err_pipe[0]);

		/* redirect stdout to the pipe */
		close(1);
		if (dup(out_pipe[1]) < 0) {
			LOG_MSG (LOG_ERROR, "%s dup() failed %s\n",
				 __FUNCTION__, strerror(errno));
		}
		
		/* redirect stderr to the pipe */
		close(2);

		if (dup(err_pipe[1]) < 0) {
			LOG_MSG (LOG_ERROR, "%s dup() failed %s\n",
				 __FUNCTION__, strerror(errno));
		}
		
		exec_wrapper(command);
		/* execution should never reach this point */
		exit(1);
	} else {
		LOG_MSG(LOG_ERROR, "Fork failed: %s", strerror(errno));
		goto error_fork;
	}

	return pid;

error_fork:
	close(err_pipe[0]);
	close(err_pipe[1]);
error_err:
	close(out_pipe[0]);
	close(out_pipe[1]);
error_out:
	return -1;
}

/** Create new process with new session ID
 * @param command Command to execute
 * @return PID of process on success, -1 in error
 */
static pid_t fork_process(const char *command) {
	pid_t pid = fork();

	if (pid > 0) {		/* parent */
		LOG_MSG(LOG_DEBUG, "Forked new process %d", pid);
	} else if (pid == 0) {	/* child */
		/* Create new session id.
		 * Process group ID and session ID
		 * are set to PID (they were PPID) */
		setsid();
		
		exec_wrapper(command);
		/* execution should never reach this point */
		exit(1);
	} else {
		LOG_MSG(LOG_ERROR, "Fork failed: %s", strerror(errno));
	}

	return pid;
}

/** Allocate memory for stream_data
 * @param data Pointer to stream_data structure
 * @param size Number of bytes to be allocated
 * @return Non NULL on success, NULL in error
 */
static void* stream_data_realloc(stream_data* data, int size) {
	unsigned char* newptr = (unsigned char*)malloc(size);
	unsigned char* oldptr = data->buffer;
	int length = data->size <= size ? data->size : size;

	if (newptr) {
		if (data->buffer != NULL) {
			memcpy(newptr, oldptr, length);
		}
		data->buffer = newptr;
		data->size = size;
		free(oldptr);
	} else {
		LOG_MSG(LOG_ERROR, "Stream data memory allocation failed");
	}

	return (void*)newptr;
}

/** Free memory allocated for stream_data
 * @param data Pointer to stream_data structure
 */
static void stream_data_free(stream_data* data) {
	free(data->buffer);
	data->buffer = NULL;
	data->size = 0;
	data->length = 0;
}

/** Append data to stream_data and reallocate memory if necessary
 * @param data Pointer to stream_data structure
 * @param src Data to append
 */
static void stream_data_append(stream_data* data, char* src) {
	int length = strlen(src);

	if (data->size - data->length >= length + 1 || 
	    stream_data_realloc(data, data->length + length + 1) != NULL) {
		strcpy((char *)&data->buffer[data->length], src);
		data->length += length;
	}

}

/** Read data from file descriptor and append to stream_data. Reallocates 
 * memory if necessary
 * @param fd File descriptor to read
 * @param data Pointer to stream_data structure
 * @return Value returned by read
 */
static int read_and_append(int fd, stream_data* data) {
	const int read_size = 255;	/* number of bytes to read */
	int ret = 0;

	do {
		/*
		 * is there allocated memory left for read_size bytes + 
		 * terminating null ? 
		 */
		if (data->size - data->length < read_size + 1) {
			if (stream_data_realloc(data, data->size + 1024) 
			    == NULL) {
				/* memory allocation failed */
				return -3;
			}
		}

		ret = read(fd, &data->buffer[data->length], read_size);

		if (ret > 0) {
			/* read was successful, update stream_data */
			data->length += ret;
			data->buffer[data->length] = '\0';
		}
	}
	while (ret > 0);

	return ret;
}

/** Signal handler for SIGALRM (timer)
 * @param signum Identifier of received signal
 */
static void timer_handler(int signum) {
	if (signum == SIGALRM) {
		timer_value = 1;
	}
}

/** Initialize timer and set signal action for SIGALRM
 * @param secs Value in seconds after which global variable timer_value
 * will be set
 * @return 0 in success, -1 in error
 */
static int set_timer(long secs) {
	struct sigaction act;
	struct itimerval timer;

	act.sa_handler = timer_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	timer.it_value.tv_sec = secs;
	timer.it_value.tv_usec = 0;

	timer_value = 0;

	/* set signal action. original action is stored in global 
	 * default_alarm_action */
	if (sigaction(SIGALRM, &act, &default_alarm_action) < 0) {
		perror("sigaction");
		goto erraction;
	}

	if (setitimer(ITIMER_REAL, &timer, NULL) < 0) {
		perror("setitimer");
		goto errtimer;
	}

	LOG_MSG(LOG_DEBUG, "Set timeout timer to %lu seconds", secs);

	return 0;

 errtimer:
	if (sigaction(SIGALRM, &default_alarm_action, NULL) < 0) {
		perror("sigaction");
	}
 erraction:
	return -1;
}

/** Reset timer and restore signal action of SIGALRM
 */
static void reset_timer() {
	struct itimerval timer;

	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 0;

	if (setitimer(ITIMER_REAL, &timer, NULL) < 0) {
		perror("setitimer");
	}

	/* restore original action for signal SIGALRM */
	if (default_alarm_action.sa_handler != NULL && 
	    sigaction(SIGALRM, &default_alarm_action, NULL) < 0) {
		perror("sigaction");
	}

	timer_value = 0;

	LOG_MSG(LOG_DEBUG, "Reset timeout timer");
}

/** Do unblocking wait for state change of process(es)
 * belonging to process group
 * @param data Input and output data of the process
 * @return 1 if process(es) has terminated, 0 otherwise
 */
static int execution_terminated(exec_data* data) {
	pid_t pid = 0;
	pid_t pgroup = getpgid(data->pid);
	int ret = 0;
	int status = 0;

	pid = waitpid(-pgroup, &status, WNOHANG);

	switch (pid) {
	case -1:
		if (errno == ECHILD) {
			/* no more children */
			LOG_MSG(LOG_DEBUG, "waitpid reported no more children");
			ret = 1;
		} else {
			LOG_MSG(LOG_ERROR, "waitpid: %s", strerror (errno));
		}
		
		break;
	case 0:
		/* WNOHANG was specified and unterminated child exists */
		break;
	default:
		/* we are only interested on return value of process
		   data->pid not its children's */
		if (pid != data->pid) {
			break;
		}

		if (WIFEXITED(status)) {
			/* child exited normally */
			data->result = WEXITSTATUS(status);
			LOG_MSG(LOG_DEBUG,
				"Process %d exited with status %d",
				pid, WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)) {
			/* child terminated by a signal */
			data->result = WTERMSIG(status);
			stream_data_append(&data->failure_info,
					   FAILURE_INFO_TIMEOUT);
			LOG_MSG(LOG_DEBUG,
				"Process %d was terminated by signal %d",
				pid, WTERMSIG(status));
		} else {
			data->result = -1;
			LOG_MSG(LOG_ERROR,
				"Unexpected return status %d from process %d",
				status, pid);
		}
		break;
	}

	return ret;
}

/** Read output streams from executed process
 * @param stdout_fd File descriptor to read stdout stream
 * @param stderr_fd File descriptor to read stderr stream
 * @param data Input and output data controlling execution
 */
static void process_output_streams(int stdout_fd, int stderr_fd, 
				   exec_data* data) 
{
	struct pollfd fds[2];
	int i = 0;

	fds[0].fd = stdout_fd;
	fds[0].events = POLLIN;
	fds[1].fd = stderr_fd;
	fds[1].events = POLLIN;

	switch(poll(fds, 2, POLL_TIMEOUT_MS)) {
	case 0:
		/* poll timeout */
		break;
	case -1:
		/* error */
		break;
	default:
		for(i = 0; i < 2; i++) {
			if (fds[i].revents & POLLIN) {
				if (fds[i].fd == stdout_fd) {
					LOG_MSG(LOG_DEBUG, 
						"Reading stdout of process %d",
						data->pid);
					read_and_append(stdout_fd, 
							&data->stdout_data);
				} else if (fds[i].fd == stderr_fd) {
					LOG_MSG(LOG_DEBUG, 
						"Reading stderr of process %d",
						data->pid);
					read_and_append(stderr_fd,
							&data->stderr_data);
				}
			}
		}
		break;
	}

}

/** Set timeout timers, read output, and control execution of test step
 * @param stdout_fd File descriptor to read stdout stream
 * @param stderr_fd File descriptor to read stderr stream
 * @param data Input and output data controlling execution
 */
static void communicate(int stdout_fd, int stderr_fd, exec_data* data) {
	pid_t pgroup = 0;
	int terminated = 0;
	int killed = 0;
	int ready = 0;

	LOG_MSG(LOG_DEBUG, "Communicating with process %d", data->pid);

	if (data->redirect_output == REDIRECT_OUTPUT) {
		/* Use non blocking mode such that read will not block */
		fcntl(stdout_fd, F_SETFL, O_NONBLOCK);
		fcntl(stderr_fd, F_SETFL, O_NONBLOCK);
	}

	set_timer(data->soft_timeout);

	while (!ready) {
		ready = execution_terminated(data);

		if (data->redirect_output == REDIRECT_OUTPUT) {
			/* read output streams. sleeps in poll */
			process_output_streams(stdout_fd, stderr_fd, data);
		} else {
			usleep(POLL_TIMEOUT_US);
		}

		if (timer_value && !terminated) {
			/* try to terminate */
			pgroup = getpgid(data->pid);
			LOG_MSG(LOG_DEBUG, "Timeout, terminating process %d", 
				data->pid);
			if (remote_host)
				ssh_kill (remote_host);
			else if (killpg(pgroup, SIGTERM) < 0) {
				perror("killpg");
			}
			terminated = 1;
			reset_timer();
			set_timer(data->hard_timeout);
		} else if (timer_value && !killed) {
			/* try to kill */
			pgroup = getpgid(data->pid);
			LOG_MSG(LOG_DEBUG, "Timeout, killing process %d", 
				data->pid);

			if (remote_host)
				ssh_kill (remote_host);
			else if (killpg(pgroup, SIGKILL) < 0) {
				perror("killpg");
			}
			killed = 1;
		}
	}

	/* ensure that test process' children which have not terminated by
	   SIGTERM are terminated now. */
	if (pgroup > 0 && pgroup != getpgid(0)) {
		killpg(pgroup, SIGKILL);
	}

	reset_timer();

	if (data->redirect_output == REDIRECT_OUTPUT) {
		close(stdout_fd);
		close(stderr_fd);
	}
}

/* ------------------------------------------------------------------------- */
/** Replace control characters with space 
 * @param data stream data to mangle
 */
static void strip_ctrl_chars (stream_data* data)
{
	size_t clean_len = 0, tmp;
	char *p = (char *)data->buffer, *endp;
	
	/* \x01-\x09\x0B\x0C\x0E-\x1F\x7F */
	const char rej[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
			    0x0B,
			    0x0C,
			    0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
			    0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F, 
			    0x7F,
			    '\0'};

	
	do {
		tmp = strcspn(p, rej);
		clean_len += tmp;
		p += tmp;
		if (clean_len < data->length)
			*p = ' ';
	} while (clean_len < data->length);

	/* \0 needs special handling */
	endp = (char *)&data->buffer [data->length];
	do {
		p = rawmemchr(data->buffer, '\0');

		if (p && p != endp)
			*p = ' ';
	} while (p != endp);
	
}

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */

/** Execute a test step command
 * @param command Command to execute
 * @param data Input and output data controlling execution
 * @return 0 in success
 */
int execute(const char* command, exec_data* data) {
	int stdout_fd = -1;
	int stderr_fd = -1;

	data->start_time = time(NULL);

	if (command != NULL) {
	  LOG_MSG(LOG_DEBUG, "Executing command \'%s\'", command);
	}

	if (data->redirect_output == REDIRECT_OUTPUT) {
		data->pid = fork_process_redirect(&stdout_fd, 
						  &stderr_fd, 
						  command);
	} else {
		data->pid = fork_process(command);
	}

	if (data->pid > 0) {
		communicate(stdout_fd, stderr_fd, data);
	}

	data->end_time = time(NULL);
	if (data->stdout_data.length) strip_ctrl_chars (&data->stdout_data);
	if (data->stderr_data.length) strip_ctrl_chars (&data->stderr_data);
	
	return 0;
}

/** Initialize exec_data structure
 * @param data Pointer to data
 */
void init_exec_data(exec_data* data) {
	/* Initialize with default values */
	data->redirect_output = REDIRECT_OUTPUT;
	data->soft_timeout = COMMON_SOFT_TIMEOUT;
	data->hard_timeout = COMMON_HARD_TIMEOUT;
	data->pid = 0;
	init_stream_data(&data->stdout_data, 1024);
	init_stream_data(&data->stderr_data, 1024);
	init_stream_data(&data->failure_info, 0);
}

void clean_exec_data(exec_data* data) {
	clean_stream_data(&data->stdout_data);
	clean_stream_data(&data->stderr_data);
	clean_stream_data(&data->failure_info);
}

/** Initialize stream_data structure
 * @param data Pointer to data
 * @param allocate Number of bytes to allocate for string buffer
 */
void init_stream_data(stream_data* data, int allocate) {
	data->buffer = NULL;
	data->size = 0;
	data->length = 0;

	/* try to allocate memory for stream data */
	if (allocate && stream_data_realloc(data, allocate)) {
		/* initialize with an empty string */
		data->buffer[0] = '\0';
	}
}

/** Clean stream_data structure
 * @param data Pointer to data
 */
void clean_stream_data(stream_data* data) {
	stream_data_free(data);
}

/** Sets the verbosity level
 * @param opts testrunner lite options
 */
void executor_init (testrunner_lite_options *opts)
{
	
	remote_host = opts->target_address;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

