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
#include <string.h>		/* strtok */
#include <sys/wait.h>
#include <stdio.h>

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
static int my_pclose(int pid, int* stdout_fd, int* stderr_fd);

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

static int my_popen(int* stdout_fd, int* stderr_fd, const char *command, char * argv[])
{
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

static int my_pclose(int pid, int* stdout_fd, int* stderr_fd)
{
	int rc, status;
	close(*stdout_fd);
	close(*stderr_fd);

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

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */

int execute(const char* command, output_processor_callback process_output) {
	int pid = 0;
	int stdout_fd = 0;
	int stderr_fd = 0;
	int ret = 0;
	char* argv[256];

	parse_command_args(command, argv, 256);

	pid = my_popen(&stdout_fd, &stderr_fd, argv[0], argv);

	if (pid > 0) {
		if (process_output != NULL) {
			process_output(stdout_fd, stderr_fd);
		}

		ret = my_pclose(pid, &stdout_fd, &stderr_fd);
		printf("retval=%d\n", ret);
	}

	free_args(argv);

	return 0;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

