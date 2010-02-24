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
#include <unistd.h>  		/* dup, pipe, fork, close, execvp */
#include <string.h>		/* strtok, strncpy */
#include <poll.h>
#include <sys/wait.h>
#include <stdio.h>

#include <errno.h>

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
static void parse_command_args(const char* command, char* argv[], int max_args);
static void free_args(char* argv[]);
static int my_popen(int* stdout_fd, int* stderr_fd, const char *command, char * argv[]);
static int my_pclose(int pid, int stdout_fd, int stderr_fd);

/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */

static void parse_command_args(const char* command, char* argv[], int max_args) {
	int n = 0;
	int size = strlen(command) + 1;
	char* buff = NULL;
	char* str = NULL;

	buff = (char*)malloc(size);

	if (buff != NULL) {
		/* command must be copied because strtok modifies its first argument */
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

static void free_args(char* argv[]) {
	while(*argv != NULL) {
		free(*argv);
		argv++;
	}
}

static int my_popen(int* stdout_fd, int* stderr_fd, const char *command, char * argv[]) {
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
		dup(out_pipe[1]);

		/* redirect stderr to the pipe */
		close(2);
		dup(err_pipe[1]);

		execvp(command, (char**)argv);
		/* TODO: Handle error */
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

static int my_pclose(int pid, int stdout_fd, int stderr_fd) {
	int rc, status;
	close(stdout_fd);
	close(stderr_fd);

	rc = waitpid(pid, &status, 0);

	if (WIFEXITED(status)) {
		/* child exited noamally */
		return WEXITSTATUS(status);
	} else if (WIFSIGNALED(status)) {
		/* child terminated by a signal */
		return -2;
	} else {
		return -1;
	}
}

static char* reallocate(stream_data* data, int size) {
	char* newptr = (char*)malloc(size);
	char* oldptr = data->buffer;

	if (newptr) {
		if (data->buffer != NULL) {
			memcpy(newptr, data->buffer, data->size);
		}
		data->buffer = newptr;
		data->size = size;
		if (oldptr != NULL) {
			free(oldptr);
		}
	}

	return newptr;
}

static int read_and_append(int fd, stream_data* data) {
	int ret = 0;

	if (data->buffer == NULL || (data->size - data->length < 4)) {
		if (reallocate(data, data->size+1024) == NULL) {
			return -1;
		}
	}

	while ((ret = read(fd, &data->buffer[data->length], 4)) > 0) {
		data->length += ret;
		data->buffer[data->length] = '\0';
	}

	return ret;
}

static void process_output_streams(int stdout_fd, int stderr_fd, stream_data* stdout_data, stream_data* stderr_data) {
	char out[1024];
	char err[1024];
	ssize_t bytes = 0;
	char* p = out;
	int poll_timeout = -1;
	struct pollfd fds[2];
	int ret = 0;
	int i = 0;
	int outeof = 0;
	int erreof = 0;

	fds[0].fd = stdout_fd;
	fds[0].events = POLLIN;
	fds[1].fd = stderr_fd;
	fds[1].events = POLLIN;

	while (!outeof && !erreof) {
		ret = poll(fds, 2, poll_timeout);

		switch(ret) {
		case 0:
			/* timeout */
			return;
			break;
		case -1:
			/* error */
			printf("poll error\n");
			return;
			break;
		default:
			for(i = 0; i < 2; i++) {
				if (fds[i].revents & POLLIN) {
					if (fds[i].fd == stdout_fd) {
						bytes = read_and_append(stdout_fd, stdout_data);
						if (bytes <= 0) {
							outeof = 1;
						}
					} else if (fds[i].fd == stderr_fd) {
						bytes = read_and_append(stderr_fd, stderr_data);
						if (bytes <= 0) {
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
	}
}

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */

int execute(const char* command, exec_data* data) {
	int pid = 0;
	int stdout_fd = 0;
	int stderr_fd = 0;
	int ret = 0;
	char* argv[256];

	parse_command_args(command, argv, sizeof(argv)-1);

	pid = my_popen(&stdout_fd, &stderr_fd, argv[0], argv);

	if (pid > 0) {
		process_output_streams(stdout_fd, stderr_fd, &data->stdout_data, &data->stderr_data);
		ret = my_pclose(pid, stdout_fd, stderr_fd);
		data->result = ret;
	}

	free_args(argv);

	return 0;
}

void init_exec_data(exec_data* data) {
	init_stream_data(&data->stdout_data);
	init_stream_data(&data->stderr_data);
}

void init_stream_data(stream_data* data) {
	data->buffer = NULL;
	data->size = 0;
	data->length = 0;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

