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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>  		/* dup, pipe, fork, close, execvp */
#include <string.h>		/* strtok, strncpy */
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <string.h>

#include "executor.h"

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
/* None */

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
static int my_popen(int* stdout_fd, int* stderr_fd, const char *command);
static int my_pclose(int pid, int stdout_fd, int stderr_fd);
static void* stream_data_realloc(stream_data* data, int size);
static void stream_data_free(stream_data* data);
static void stream_data_append(stream_data* data, char* src);
static int read_and_append(int fd, stream_data* data);
static void process_output_streams(int stdout_fd, int stderr_fd, 
				   exec_data* data);
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
	char* argv[4];

	argv[0] = (char*)malloc(strlen(SHELLCMD) + 1);
	strcpy(argv[0], SHELLCMD);
	argv[1] = (char*)malloc(strlen(SHELLCMD_ARG1) + 1);
	strcpy(argv[1], SHELLCMD_ARG1);
	argv[2] = (char*)malloc(strlen(command) + 1);
	strcpy(argv[2], command);
	argv[3] = NULL;

	/* on success, execvp does not return */
	ret = execvp(argv[0], argv);

	free(argv[0]);
	free(argv[1]);
	free(argv[2]);
	
	return ret;
}

/** Execute a command and get file descriptors to its output streams
 * @param stdout_fd Pointer to a file descriptor used to read stdout of
 *        executed command
 * @param stderr_fd Pointer to a file descriptor used to read stderr of 
 *        executed command
 * @param command Command to execute
 * @return PID of process on success, -1 in error
 */
static int my_popen(int* stdout_fd, int* stderr_fd, const char *command) {
	int out_pipe[2];
	int err_pipe[2];
	int pid;

	if (pipe(out_pipe) < 0)
		goto error_out;

	if (pipe(err_pipe) < 0)
		goto error_err;

	pid = fork();
	if (pid > 0) { /* parent */
		/* close the write end of the pipes */
		close(out_pipe[1]);
		close(err_pipe[1]);
		*stdout_fd = out_pipe[0];
		*stderr_fd = err_pipe[0];
		return pid;
	} else if (pid == 0) { /* child */
		/* close the read end of the pipes */
		close(out_pipe[0]);
		close(err_pipe[0]);

		/* redirect stdout to the pipe */
		close(1);
		if (dup(out_pipe[1]) < 0) {
			fprintf (stderr, "%s dup() failed %s\n",
				 __FUNCTION__, strerror(errno));
			goto error_err;
		}
		

		/* redirect stderr to the pipe */
		close(2);

		if (dup(err_pipe[1]) < 0) {
			fprintf (stderr, "%s dup() failed %s\n",
				 __FUNCTION__, strerror(errno));
			goto error_err;
		}
		

		exec_wrapper(command);
		/* execution should never reach this point */
		exit(1);
	} else {
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

/** Close file descriptors returned by my_popen and wait for process
 * @param pid PID returned by my_popen
 * @param stdout_fd File descriptor to close
 * @param stderr_fd File descriptor to close
 * @return Status given by waitpid command
 */
static int my_pclose(int pid, int stdout_fd, int stderr_fd) {
	int status = 0;

	close(stdout_fd);
	close(stderr_fd);
	waitpid(pid, &status, 0);

	return status;
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
 * @param stream_data Pointer to stream_data structure
 * @return Value returned by read
 */
static int read_and_append(int fd, stream_data* data) {
	const int read_size = 255;	/* number of bytes to read */
	int ret = 0;

	if (data->buffer == NULL) {
		return -2;
	}

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

/** Read output streams from executed process and handle timeouts
 * @param stdout_fd File descriptor to read stdout stream
 * @param stderr_fd File descriptor to read stderr stream
 * @param data Input and output data controlling execution
 */
static void process_output_streams(int stdout_fd, int stderr_fd, 
				   exec_data* data) 
{
	int bytes = 0;
	int poll_timeout = 500;	/* ms */
	struct pollfd fds[2];
	int ret = 0;
	int i = 0;
	int outeof = 0;
	int erreof = 0;
	struct timeval start_time;
	struct timeval current_time;
	time_t elapsed_time = 0;
	int terminated = 0;

	/* Use non blocking mode such that read will not block */
	fcntl(stdout_fd, F_SETFL, O_NONBLOCK);
	fcntl(stderr_fd, F_SETFL, O_NONBLOCK);

	fds[0].fd = stdout_fd;
	fds[0].events = POLLIN;
	fds[1].fd = stderr_fd;
	fds[1].events = POLLIN;

	gettimeofday(&start_time, NULL);

	while (!outeof && !erreof) {
		ret = poll(fds, 2, poll_timeout);

		switch(ret) {
		case 0:
			/* poll timeout */
			break;
		case -1:
			/* error */
			return;
			break;
		default:
			for(i = 0; i < 2; i++) {
				if (fds[i].revents & POLLIN) {
					if (fds[i].fd == stdout_fd) {
						bytes = 
							read_and_append(
								stdout_fd, 
								&data->\
								stdout_data);
						if (bytes == 0) {
							outeof = 1;
						}
					} else if (fds[i].fd == stderr_fd) {
						bytes = read_and_append(
							stderr_fd, &data->\
							stderr_data);
						if (bytes == 0) {
							erreof = 1;
						}
					}
				}
				if (fds[i].revents & POLLHUP) {
					/* EOF for a pipe */
					if (fds[i].fd == stdout_fd) {
						outeof = 1;
					} else if (fds[i].fd == stderr_fd) {
						erreof = 1;
					}
				}
			}
			break;
		}

		gettimeofday(&current_time, NULL);
		elapsed_time = current_time.tv_sec - start_time.tv_sec;

		if (terminated && elapsed_time > data->hard_timeout) {
			/* kill test process and stop reading its output */
			kill(data->pid, SIGKILL);
			break;
		} else if (!terminated && elapsed_time > data->soft_timeout) {
			/* try to terminate */
			kill(data->pid, SIGTERM);
			terminated = 1;
		}
	}
}
/* ------------------------------------------------------------------------- */
/** Replace control characters with <space> 
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
			    0x7F};

	
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
	int status = 0;

	data->start_time = time(NULL);
	data->pid = my_popen(&stdout_fd, &stderr_fd, command);

	if (data->pid > 0) {
		process_output_streams(stdout_fd, stderr_fd, data);
		status = my_pclose(data->pid, stdout_fd, stderr_fd);

		if (WIFEXITED(status)) {
			/* child exited normally */
			data->result = WEXITSTATUS(status);
		} else if (WIFSIGNALED(status)) {
			/* child terminated by a signal */
			data->result = WTERMSIG(status);
			stream_data_append(&data->failure_info, 
					   FAILURE_INFO_TIMEOUT);
		} else {
			data->result = -1;
		}
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
	init_stream_data(&data->stdout_data, 1024);
	init_stream_data(&data->stderr_data, 1024);
	init_stream_data(&data->failure_info, 0);
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

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

