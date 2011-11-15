/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Contains changes by Wind River Systems, 2011-03-09
 *
 * Contact: Sami Lahtinen <ext-sami.t.lahtinen@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
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
#include <sched.h>
#include <libxml/xmlstring.h>

#include "remote_executor.h"
#ifdef ENABLE_LIBSSH2
#include "remote_executor_libssh2.h"
#endif
#include "executor.h"
#include "log.h"
#include "utils.h"

/* ------------------------------------------------------------------------- */
/* EXTERNAL DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* EXTERNAL GLOBAL VARIABLES */
extern int bail_out;
extern char *global_failure;

/* ------------------------------------------------------------------------- */
/* EXTERNAL FUNCTION PROTOTYPES */
/* None */

/* ------------------------------------------------------------------------- */
/* GLOBAL VARIABLES */
volatile sig_atomic_t resume_testrun_count = 0;

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL GLOBAL VARIABLES */
LOCAL volatile sig_atomic_t timer_value = 0;
LOCAL struct sigaction default_alarm_action = { .sa_handler = NULL };
LOCAL testrunner_lite_options *options;
LOCAL exec_data *current_data; 
#ifdef ENABLE_LIBSSH2
LOCAL libssh2_conn *lssh2_conn;
#endif
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
/* ------------------------------------------------------------------------- */
LOCAL void parse_command_args(const char* command, char* argv[], int max_args);
/* ------------------------------------------------------------------------- */
LOCAL void free_args(char* argv[]);
#endif
/* ------------------------------------------------------------------------- */
LOCAL void set_child_signal_handlers();
/* ------------------------------------------------------------------------- */
LOCAL pid_t fork_process_redirect(int* stdout_fd, int* stderr_fd, 
				   const char *command, exec_data* data);
/* ------------------------------------------------------------------------- */
LOCAL pid_t fork_process(const char *command, exec_data* data);
/* ------------------------------------------------------------------------- */
LOCAL void* stream_data_realloc(stream_data* data, int size);
/* ------------------------------------------------------------------------- */
LOCAL void stream_data_free(stream_data* data);
/* ------------------------------------------------------------------------- */
LOCAL void stream_data_append(stream_data* data, char* src);
/* ------------------------------------------------------------------------- */
LOCAL int read_and_append(int fd, stream_data* data);
/* ------------------------------------------------------------------------- */
LOCAL void timer_handler(int signum);
/* ------------------------------------------------------------------------- */
LOCAL int set_timer(long secs);
/* ------------------------------------------------------------------------- */
LOCAL void reset_timer();
/* ------------------------------------------------------------------------- */
LOCAL int execution_terminated(exec_data* data);
/* ------------------------------------------------------------------------- */
LOCAL void process_output_streams(int stdout_fd, int stderr_fd, 
				   exec_data* data);
/* ------------------------------------------------------------------------- */
LOCAL void communicate(int stdout_fd, int stderr_fd, exec_data* data);
/* ------------------------------------------------------------------------- */
LOCAL void strip_ctrl_chars (stream_data* data);
/* ------------------------------------------------------------------------- */
LOCAL void utf8_check (stream_data* data, const char *id, pid_t pid);
#ifdef ENABLE_LIBSSH2
/* ------------------------------------------------------------------------- */
LOCAL int executor_init_libssh2 (testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
LOCAL int execute_libssh2 (const char *command, exec_data *data);
#endif
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
#if 0
/* ------------------------------------------------------------------------- */
/** Parse a command
 * @param command Command string to parse
 * @param argv Array of strings to store parsed arguments
 * @param max_args Maximum number of arguments to parse
 */
LOCAL void parse_command_args(const char* command, char* argv[], int max_args)
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
			argv[n] = stdup (str);
			str = strtok(NULL, " ");
		}
	}
	argv[n] = NULL;

	free(buff);
}
/* ------------------------------------------------------------------------- */
/** Free argument strings
 * @param argv Array of argument strings to free
 */
LOCAL void free_args(char* argv[]) {
	while(*argv != NULL) {
		free(*argv);
		argv++;
	}
}
#endif
/* ------------------------------------------------------------------------- */
/** Resets signal handlers set by parent process
 * 
 */
LOCAL void set_child_signal_handlers()
{
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
}
/* ------------------------------------------------------------------------- */
/** Executes a command on local or remote host
 * @param command Command to execute
 * @return Does not return in success, error code from exec in case of error
 */
LOCAL int exec_wrapper(const char *command, exec_data* data)
{
	int ret = 0;

	if (options->remote_executor) {
		ret = remote_execute (options->remote_executor, command);
	} else {
		if (data != NULL && data->disobey_chroot) {
			LOG_MSG(LOG_DEBUG, "Disobeying chroot for command %s",
				command);
		} else if (options->chroot_folder) {
			if (chdir(options->chroot_folder) == -1) {
				LOG_MSG(LOG_ERR, "Failed to chdir into chroot '%s'",
					options->chroot_folder);
				return -1;
			}
			if (chroot(".") == -1) {
				LOG_MSG(LOG_ERR, "Failed to set chroot to '%s'",
					options->chroot_folder);
				return -1;
			}
		}

		/* on success, execvp does not return */
		ret = execl(SHELLCMD, SHELLCMD, SHELLCMD_ARGS,
			    command, (char*)NULL);
	}

	return ret;
}
/* ------------------------------------------------------------------------- */
/** Create new process with new session ID and redirect its output
 * @param stdout_fd Pointer to a file descriptor used to read stdout of
 *        executed command
 * @param stderr_fd Pointer to a file descriptor used to read stderr of 
 *        executed command
 * @param command Command to execute
 * @return PID of process on success, -1 in error
 */
LOCAL pid_t fork_process_redirect(int* stdout_fd, int* stderr_fd, const char *command,
                                 exec_data* data) {
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
		set_child_signal_handlers();

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
			perror("dup(out_pipe[1]");
		}
		
		/* redirect stderr to the pipe */
		close(2);
		if (dup(err_pipe[1]) < 0) {
			perror("dup(err_pipe[1])");
		}
		
		exec_wrapper(command, data);
		/* execution should never reach this point */
		exit(1);
	} else {
		LOG_MSG(LOG_ERR, "Fork failed: %s", strerror(errno));
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
/* ------------------------------------------------------------------------- */
/** Create new process with new session ID
 * @param command Command to execute
 * @return PID of process on success, -1 in error
 */
LOCAL pid_t fork_process(const char *command, exec_data* data) {
	pid_t pid = fork();

	if (pid > 0) {		/* parent */
		LOG_MSG(LOG_DEBUG, "Forked new process %d", pid);
	} else if (pid == 0) {	/* child */
		set_child_signal_handlers();

		/* Create new session id.
		 * Process group ID and session ID
		 * are set to PID (they were PPID) */
		setsid();
		
		exec_wrapper(command, data);
		/* execution should never reach this point */
		exit(1);
	} else {
		LOG_MSG(LOG_ERR, "Fork failed: %s", strerror(errno));
	}

	return pid;
}
/* ------------------------------------------------------------------------- */
/** Allocate memory for stream_data
 * @param data Pointer to stream_data structure
 * @param size Number of bytes to be allocated
 * @return Non NULL on success, NULL in error
 */
LOCAL void* stream_data_realloc(stream_data* data, int size) {
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
		LOG_MSG(LOG_ERR, "Stream data memory allocation failed");
	}

	return (void*)newptr;
}
/* ------------------------------------------------------------------------- */
/** Free memory allocated for stream_data
 * @param data Pointer to stream_data structure
 */
LOCAL void stream_data_free(stream_data* data) {
	free(data->buffer);
	data->buffer = NULL;
	data->size = 0;
	data->length = 0;
}
/* ------------------------------------------------------------------------- */
/** Append data to stream_data and reallocate memory if necessary
 * @param data Pointer to stream_data structure
 * @param src Data to append
 */
LOCAL void stream_data_append(stream_data* data, char* src) {
	int length = strlen(src);

	if (data->size - data->length >= length + 1 || 
	    stream_data_realloc(data, data->length + length + 1) != NULL) {
		strncpy((char *)&data->buffer[data->length], src, length + 1);
		data->length += length;
	}

}
/* ------------------------------------------------------------------------- */
/** Read data from file descriptor and append to stream_data. Reallocates 
 * memory if necessary
 * @param fd File descriptor to read
 * @param data Pointer to stream_data structure
 * @return Value returned by read
 */
LOCAL int read_and_append(int fd, stream_data* data) {
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
			data->buffer[data->length + ret] = '\0';
			if (options->print_step_output)
				printf ("%s", &data->buffer[data->length]);

			data->length += ret;

		}
	}
	while (ret > 0);

	return ret;
}
/* ------------------------------------------------------------------------- */
/** Signal handler for SIGALRM (timer)
 * @param signum Identifier of received signal
 */
LOCAL void timer_handler(int signum) {
	if (signum == SIGALRM) {
		timer_value = 1;
	}
}
/* ------------------------------------------------------------------------- */
/** Initialize timer and set signal action for SIGALRM
 * @param secs Value in seconds after which global variable timer_value
 * will be set
 * @return 0 in success, -1 in error
 */
LOCAL int set_timer(long secs) {
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
/* ------------------------------------------------------------------------- */
/** Reset timer and restore signal action of SIGALRM
 */
LOCAL void reset_timer() {
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
/* ------------------------------------------------------------------------- */
/** Do unblocking wait for state change of process(es)
 * belonging to process group
 * @param data Input and output data of the process
 * @return 1 if process(es) has terminated, 0 otherwise
 */
LOCAL int execution_terminated(exec_data* data) {
	pid_t pid = 0;
	pid_t pgroup = 0;
	int ret = 0;
	int status = 0;
	char fail_str [100];

	pgroup = getpgid(data->pid);
	if (pgroup > 1 && pgroup != data->pgid) {
		/* store process group id for further use */
		data->pgid = pgroup;
		LOG_MSG(LOG_DEBUG, "Process group of process %d is %d",
			data->pid, data->pgid);
	}

	pid = waitpid(-pgroup, &status, WNOHANG);

	switch (pid) {
	case -1:
		if (errno == ECHILD) {
			if (!data->waited) {
				/* data->pid has not been terminated */
				break;
			}
			/* no more children */
			LOG_MSG(LOG_DEBUG, "waitpid reported no more children");
			ret = 1;
		} else {
			LOG_MSG(LOG_ERR, "waitpid: %s", strerror (errno));
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

		/* set flag that process has been terminated */
		data->waited = 1;

		if (WIFEXITED(status)) {
			/* child exited normally */
			data->result = WEXITSTATUS(status);
			if (options->remote_executor &&
			    (data->result == 255 ||
			     (data->result > 64 && data->result < 80))
			    ) {
				if (remote_check_conn
				    (options->remote_executor)) {
					bail_out = TESTRUNNER_LITE_REMOTE_FAIL;
					global_failure = 
						"earlier connection failure";
					LOG_MSG(LOG_ERR, 
						"remote connection failure");
					stream_data_append(&data->failure_info,
							   "connection "
							   "failure");
				}
			}
			else if (options->remote_executor &&
				 data->result > 128 &&
				 data->result < 160) {
				stream_data_append(&data->failure_info,
						   "terminated by signal");
			}
			LOG_MSG(LOG_DEBUG,
				"Process %d exited with status %d",
				pid, WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)) {
			/* child terminated by a signal */
			data->result = WTERMSIG(status);
			snprintf (fail_str, 100,
				  " terminated by signal %d ",
				  WTERMSIG(status));
			stream_data_append(&data->failure_info,
					   fail_str);
			LOG_MSG(LOG_DEBUG,
				"Process %d was terminated by signal %d",
				pid, WTERMSIG(status));
		} else {
			data->result = -1;
			LOG_MSG(LOG_ERR,
				"Unexpected return status %d from process %d",
				status, pid);
		}
		break;
	}

	return ret;
}
/* ------------------------------------------------------------------------- */
/** Read output streams from executed process
 * @param stdout_fd File descriptor to read stdout stream
 * @param stderr_fd File descriptor to read stderr stream
 * @param data Input and output data controlling execution
 */
LOCAL void process_output_streams(int stdout_fd, int stderr_fd, 
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
/* ------------------------------------------------------------------------- */
/** Set timeout timers, read output, and control execution of test step
 * @param stdout_fd File descriptor to read stdout stream
 * @param stderr_fd File descriptor to read stderr stream
 * @param data Input and output data controlling execution
 */
LOCAL void communicate(int stdout_fd, int stderr_fd, exec_data* data) {
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
			LOG_MSG(LOG_DEBUG, "Timeout, terminating process %d", 
				data->pid);

			if (options->remote_executor && !bail_out) {
				remote_kill (options->remote_executor, 
					     data->pid, SIGTERM);
			}

			if(!options->remote_executor) {
				kill_step(data->pid, SIGTERM);
			}

			data->signaled = SIGTERM;

			stream_data_append(&data->failure_info,
					   FAILURE_INFO_TIMEOUT);

			terminated = 1;
			reset_timer();
			set_timer(data->hard_timeout);
		} else if (timer_value && !killed) {
			/* try to kill */
			LOG_MSG(LOG_DEBUG, "Timeout, killing process %d", 
				data->pid);

			if (options->remote_executor && !bail_out) {
				remote_kill (options->remote_executor, 
					     data->pid, SIGKILL);
			}

			kill_step(data->pid, SIGKILL);

			data->signaled = SIGKILL;

			killed = 1;
		}
	}

	/* ensure that test process' children which have not terminated by
	   SIGTERM are terminated now. */
	if (data->signaled && data->pgid > 0) {
		kill_pgroup(data->pgid, SIGKILL);
	}

	reset_timer();

	if (options->remote_executor && !bail_out) {
		remote_clean(options->remote_executor, data->pid);
	}
	if (data->redirect_output == REDIRECT_OUTPUT) {
		close(stdout_fd);
		close(stderr_fd);
	}
}
/* ------------------------------------------------------------------------- */
/** Replace control characters with space 
 * @param data stream data to mangle
 */
LOCAL void strip_ctrl_chars (stream_data* data)
{
	size_t clean_len = 0, tmp;
	char *p = (char *)data->buffer, *endp;
	
	/* \x01-\x09\x0B\x0C\x0E-\x1F\x7F */
	const char rej[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
			    0x0B,
			    0x0C,
			    0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
			    0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F, 
			    0x7F, '\0'};

	
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
/** If the data contains non utf-8 write the data to file instead of result xml
 * @param data stream data to mangle
 * @param id identifier (stdout/stderr)
 * @param pid pid of test step
 */
LOCAL void utf8_check (stream_data* data, const char *id, pid_t pid)
{
	char *fname = NULL;
	FILE *ofile = NULL;
	size_t written = 0, len;
	
	if (utf8_validity_check (data->buffer, options->max_utf8_bytes ?
				 options->max_utf8_bytes : 4)) {
		return;
	}
	len = strlen (options->output_folder) + strlen ("id") + 10 + 1 + 1;
	fname = (char *)malloc (len);
	if (!fname) {
			LOG_MSG(LOG_ERR, "OOM");
			goto error;
	}
	snprintf (fname, len, "%s/%s.%d", options->output_folder, 
		 id, pid);
	
	ofile = fopen (fname, "w+");
	if (!ofile)  {
		LOG_MSG (LOG_ERR, "%s:%s:failed to open file %s %s\n",
			 PROGNAME, __FUNCTION__, fname,
			 strerror(errno));
		goto error;
	}
	written = fwrite (data->buffer, 1, data->length, ofile);
	if (written != data->length) {
		LOG_MSG(LOG_WARNING, "failed to write full stdout to"
			"%s\n", fname,  strerror(errno));
	}
	len = strlen("non utf-8 output detected - see file ") + strlen (fname) 
		+ 1;
	data->buffer = (unsigned char *)realloc (data->buffer, len);
	if (!data->buffer) {
		LOG_MSG(LOG_ERR, "OOM");
		goto error;
	}

	snprintf ((char *)data->buffer, len,
		 "non utf-8 output detected - see file %s.%d", id, pid);
	
	LOG_MSG (LOG_DEBUG, "non utf-8 ouput from test step -  wrote to "
		 "%s instead of results xml", fname);
	
	fclose (ofile);
	free (fname);
	return;
 error:
	if (ofile) fclose (ofile);
	if (fname) free (fname);

	memset (data->buffer, 'a', data->length - 1);
	return;
} 

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** Execute a test step command
 * @param command Command to execute
 * @param data Input and output data controlling execution
 * @return 0 in success, -1 in error
 */
int execute(const char* command, exec_data* data) {
	int stdout_fd = -1;
	int stderr_fd = -1;
	
	if (command != NULL) {
		LOG_MSG(LOG_DEBUG, "Executing command \'%s\'", command);
	}

#ifdef ENABLE_LIBSSH2
	if (options->libssh2 && options->target_address) {
		return execute_libssh2(command, data);
	}
#endif

	data->start_time = time(NULL);
	
	if (data->redirect_output == REDIRECT_OUTPUT) {
		data->pid = fork_process_redirect(&stdout_fd, 
		                                  &stderr_fd, 
		                                  command, data);
	} else {
		data->pid = fork_process(command, data);
	}
	current_data = data; 
	
	if (data->pid > 0) {
		/* start communicating with test process */
		communicate(stdout_fd, stderr_fd, data);
	}
	current_data = NULL;
	data->end_time = time(NULL);
	if (data->stdout_data.length) strip_ctrl_chars (&data->stdout_data);
	if (data->stderr_data.length) strip_ctrl_chars (&data->stderr_data);
	if (data->stdout_data.length) utf8_check (&data->stdout_data, "stdout",
						  data->pid);
	if (data->stderr_data.length) utf8_check (&data->stderr_data, "stderr",
						  data->pid);
	return 0;
}
#ifdef ENABLE_LIBSSH2
/* ------------------------------------------------------------------------- */
/** Execute a test step command with libssh2
 * @param command Command to execute
 * @param data Input and output data controlling execution
 * @return 0 in success, -1 in error
 */
LOCAL int execute_libssh2 (const char* command, exec_data* data) {

	data->start_time = time(NULL);

	if (options->libssh2) {
		/* Session may have died, try reopening */
		if (!lssh2_conn) {
			if (executor_init_libssh2(options) < 0) {
				LOG_MSG(LOG_ERR, 
					"Reopening libssh2 session failed");
				bail_out = TESTRUNNER_LITE_REMOTE_FAIL;
				global_failure = "connection fail";
				return -1;
			}
		}
		if (lssh2_conn) {
			if (lssh2_execute(lssh2_conn, command, data) < 0) {
				bail_out = TESTRUNNER_LITE_REMOTE_FAIL;
				global_failure = "connection fail";
				return -1;
			}
		} else {
			LOG_MSG(LOG_ERR, "Could not open libssh2 session");
			bail_out = TESTRUNNER_LITE_REMOTE_FAIL;
			global_failure = "connection fail";
			return -1;
		}
	}

	data->end_time = time(NULL);
	if (data->stdout_data.length) strip_ctrl_chars (&data->stdout_data);
	if (data->stderr_data.length) strip_ctrl_chars (&data->stderr_data);
	if (data->stdout_data.length) utf8_check (&data->stdout_data, "stdout",
						  data->pid);
	if (data->stderr_data.length) utf8_check (&data->stderr_data, "stderr",
						  data->pid);
	return 0;
}
#endif
/* ------------------------------------------------------------------------- */
/** Initialize exec_data structure
 * @param data Pointer to data
 */
void init_exec_data(exec_data* data) {
	/* Initialize with default values */
	data->redirect_output = REDIRECT_OUTPUT;
	data->soft_timeout = COMMON_SOFT_TIMEOUT;
	data->hard_timeout = COMMON_HARD_TIMEOUT;
	data->pid = 0;
	data->pgid = 0;
	init_stream_data(&data->stdout_data, 1024);
	init_stream_data(&data->stderr_data, 1024);
	init_stream_data(&data->failure_info, 0);
	data->waited = 0;
	data->disobey_chroot = 0;
}
/* ------------------------------------------------------------------------- */
/** Clean the exec_data structure
 * @param data Pointer to exec_data
 */
void clean_exec_data(exec_data* data) {
	clean_stream_data(&data->stdout_data);
	clean_stream_data(&data->stderr_data);
	clean_stream_data(&data->failure_info);
}
/* ------------------------------------------------------------------------- */
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
/* ------------------------------------------------------------------------- */
/** Clean stream_data structure
 * @param data Pointer to data
 */
void clean_stream_data(stream_data* data) {
	stream_data_free(data);
}
/* ------------------------------------------------------------------------- */
/** Send signal to process group of a test process
 * @param pid pgroup Process group ID for signal
 * @param sig Signal number
 * @return 0 in succeess, 1 in failure
 */
int kill_pgroup(pid_t pgroup, int sig) {
	if (pgroup <= 1) {
		LOG_MSG(LOG_ERR, "Invalid pgid %d", pgroup);
		return 1;
	}

	/* pgroup must be different than the pgid of testrunner-lite */
	if (pgroup == getpgid(0)) {
		LOG_MSG(LOG_ERR, "Pgid equals the pgid of testrunner-lite");
		return 1;
	}

	LOG_MSG(LOG_DEBUG, "Sending signal %d to process group %d",
		sig, pgroup);

	if (killpg(pgroup, sig) < 0 && errno != ESRCH) {
		LOG_MSG(LOG_ERR, "killpg failed: %s", strerror(errno));
		return 1;
	}

	return 0;
}
/* ------------------------------------------------------------------------- */
/** Send signal to a process group of test step
 * 
 * @param pid Process ID of test step
 * @param sig Signal to send
 */
void kill_step(pid_t pid, int sig)
{
	pid_t pgroup = getpgid(pid);

	if (kill_pgroup(pgroup, sig)) {
		/* sending signal to the group failed, just send to the pid */
		LOG_MSG(LOG_DEBUG, "Sending signal %d to process %d", sig, pid);
		if (kill(pid, sig) < 0 && errno != ESRCH) {
			LOG_MSG(LOG_ERR, "kill failed: %s", strerror(errno));
		}
	}
}
/* ------------------------------------------------------------------------- */
/** Sets the remote host for executor 
 * @param opts testrunner lite options
 */
int executor_init(testrunner_lite_options *opts)
{
	LOG_MSG(LOG_INFO, "Initializing executor");
	options = opts;
#ifdef ENABLE_LIBSSH2
	if (options->libssh2)
		return executor_init_libssh2(opts);
#endif
	if (options->remote_executor)
		return remote_executor_init (options->remote_executor);
	return 0;
}
/* ------------------------------------------------------------------------- */
#ifdef ENABLE_LIBSSH2
/** Sets the remote host for libssh2 executor 
 * @param opts testrunner lite options
 */
LOCAL int executor_init_libssh2(testrunner_lite_options *opts)
{
	lssh2_conn = NULL;
	options = opts;
	lssh2_conn = lssh2_executor_init(options->username, 
	                                 options->target_address,
	                                 options->target_port,
	                                 options->ssh_key);
	if (!lssh2_conn) {
		LOG_MSG(LOG_ERR, "libssh2 executor init failed");
		bail_out = TESTRUNNER_LITE_REMOTE_FAIL;
		global_failure = "connection fail";
		return -1;
	}
	return 0;
}
#endif
/* ------------------------------------------------------------------------- */
/** Clean up for executor
 */
void executor_close()
{
#ifdef ENABLE_LIBSSH2
	if (options->libssh2) {
		lssh2_executor_close(lssh2_conn);
		return;
	}
#endif
	if ((options->remote_executor || options->hwinfo_target) && !bail_out) {
		remote_executor_close();
	}
}
/* ------------------------------------------------------------------------- */
/** 
 * Restore bail_out value to TESTRUNNER_LITE_REMOTE_FAIL if
 * resume_testrun_count has been incremented and bail_out equals zero.
 */
void restore_bail_out_after_resume_execution()
{
	sigset_t mask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	if (resume_testrun_count && bail_out == 0) {
		bail_out = TESTRUNNER_LITE_REMOTE_FAIL;
	}

	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}
/* ------------------------------------------------------------------------- */
/**  Wait for resume testrun signal. Resets bail_out to zero if resume signal
 * is received. bail_out is reset only once.
 * 
 */
void wait_for_resume_execution()
{
	sigset_t mask;
	sigset_t waitmask;

#ifdef ENABLE_LIBSSH2
	if(lssh2_conn) {
		lssh2_conn->status = SESSION_OK;
		lssh2_conn->signaled = 0;
	}
#endif

	/* block certain signals for this section */
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &mask, &waitmask);

	if (bail_out == TESTRUNNER_LITE_REMOTE_FAIL && !resume_testrun_count) {
		/* block certain signals during sigsuspend */
		sigaddset(&waitmask, SIGCHLD);
		/* set handler */
		signal(SIGUSR1, handle_resume_testrun);

		/* signal parent we are suspending */
		if (kill(getppid(), SIGUSR1) < 0) {
			LOG_MSG (LOG_ERR, "Error sending signal to parent: %s",
				 strerror(errno));
		}

		LOG_MSG(LOG_INFO, "Connection failure detected: "
			"waiting for signal SIGUSR1 to continue");

		/* wait for a signal  */
		sigsuspend(&waitmask);

		/* restore default handler */
		signal(SIGUSR1, SIG_DFL);

		/* recheck bail_out value and then resume flag */
		if (bail_out == TESTRUNNER_LITE_REMOTE_FAIL &&
		    resume_testrun_count) {
			LOG_MSG(LOG_INFO, "Continuing after connection "
				"failure");
			bail_out = 0;
		}
	}

	/* restore original mask */
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void wait_for_reboot()
{
	sigset_t mask;
	sigset_t waitmask;

#ifdef ENABLE_LIBSSH2
	if(lssh2_conn) {
		lssh2_conn->status = SESSION_OK;
		lssh2_conn->signaled = 0;
	}
#endif

	/* block certain signals for this section */
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &mask, &waitmask);

	if (!bail_out) {
		/* block certain signals during sigsuspend */
		sigaddset(&waitmask, SIGCHLD);
		/* set handler */
		signal(SIGUSR1, handle_reboot);

		/* signal parent we are suspending */
		if (kill(getppid(), SIGUSR2) < 0) {
			LOG_MSG (LOG_ERR, "Error sending signal to parent: %s",
				 strerror(errno));
		}

		LOG_MSG(LOG_INFO, "Device reboot requested. Sending SIGUSR2 to conductor. "
						"Waiting for SIGUSR1");

		/* wait for a signal  */
		sigsuspend(&waitmask);

		/* restore default handler */
		signal(SIGUSR1, SIG_DFL);

	}

	/* restore original mask */
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}


/* ------------------------------------------------------------------------- */
/** handler for SIGINT
 *  @param signum not used
 */
void handle_sigint (int signum)
{
	if(bail_out == 0) {
		global_failure = "Testrunner-lite interrupted by signal (2)";
		bail_out = 255+SIGINT;
	}

#ifdef ENABLE_LIBSSH2
	if (options->libssh2) {
		lssh2_signal(signum);
		return;
	}
#endif

	if (current_data) {
		if (options->remote_executor) {
			remote_kill (options->remote_executor, current_data->pid, SIGTERM);
		}
		else {
			kill_step(current_data->pid, SIGKILL);
		}
	}

}
/* ------------------------------------------------------------------------- */
/** handler for SIGTERM
 *  @param signum not used
 */
void handle_sigterm (int signum)
{
	if(bail_out == 0) {
		global_failure = "Testrunner-lite interrupted by signal (15)";
		bail_out = 255+SIGTERM;
	}

#ifdef ENABLE_LIBSSH2
	if (options->libssh2) {
		lssh2_signal(signum);
		return;
	}
#endif

	if (current_data) {
		if (options->remote_executor) {
			remote_kill (options->remote_executor, 
				     current_data->pid, SIGTERM);
		}
		else {
			kill_step(current_data->pid, SIGKILL);
		}
	}
}
/* ------------------------------------------------------------------------- */
/** handler for SIGUSR1
 *  @param signum signal number
 */
void handle_resume_testrun(int signum)
{
	if (signum == SIGUSR1) {
		resume_testrun_count++;
	}
}

/* ------------------------------------------------------------------------- */
/** handler for SIGUSR1
 *  @param signum signal number
 */
void handle_reboot(int signum)
{
	if (signum == SIGUSR1) {
		LOG_MSG(LOG_INFO, "Continuing after device reboot");
#ifdef ENABLE_LIBSSH2
		if (!options->libssh2) {
#endif
		remote_executor_init (options->remote_executor);
#ifdef ENABLE_LIBSSH2
		}
#endif
	}
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

