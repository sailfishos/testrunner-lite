/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <time.h>
#include <unistd.h>

/* ------------------------------------------------------------------------- */
/* INCLUDES */
/* None */

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */

#define SHELLCMD       "/bin/sh"
/* shell options are
 * -l : Login shell. .profile is read upon login
 * -c : Execute a command. This must be the last option followed by a command
 */
#define SHELLCMD_ARGS      "-c"
#define SHELLCMD_ARGS_STR  "-c"
#define FAILURE_INFO_TIMEOUT "timeout "
#define POLL_TIMEOUT_MS   100
#define POLL_TIMEOUT_US   (1000*POLL_TIMEOUT_MS)
#define COMMON_SOFT_TIMEOUT    90
#define COMMON_HARD_TIMEOUT    5

/* ------------------------------------------------------------------------- */
/* DATA TYPES */
/* ------------------------------------------------------------------------- */
struct _stream_data {
	unsigned char* buffer;
	int size;
	int length;
};

typedef struct _stream_data stream_data;

enum _stream_output_redirection {
	DONT_REDIRECT_OUTPUT = 0,
	REDIRECT_OUTPUT
};

struct _exec_data {
	/* input parameters */
	int redirect_output;
	unsigned soft_timeout;	/* in seconds, 0 = no timeout */
	unsigned hard_timeout;	/* after soft_timeout, 0 = no timeout */
	/* output parameters */
	pid_t pid;
	pid_t pgid;
	stream_data stdout_data;
	stream_data stderr_data;
	stream_data failure_info;
	time_t start_time;
	time_t end_time;
	int result;
	int signaled; /* In case step is terminated by signal */
	int waited;   /* flag for that pid has been returned by waitpid */
};

typedef struct _exec_data exec_data;

/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
int executor_init (testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
int execute(const char* command, exec_data* data);
/* ------------------------------------------------------------------------- */
void init_exec_data(exec_data* data);
/* ------------------------------------------------------------------------- */
void clean_exec_data(exec_data* data);
/* ------------------------------------------------------------------------- */
void init_stream_data(stream_data* data, int allocate);
/* ------------------------------------------------------------------------- */
void clean_stream_data(stream_data* data);
/* ------------------------------------------------------------------------- */
int kill_pgroup(pid_t pgroup, int sig);
/* ------------------------------------------------------------------------- */
void kill_step(pid_t pid, int sig);
/* ------------------------------------------------------------------------- */
void executor_close ();
/* ------------------------------------------------------------------------- */
void restore_bail_out_after_resume_execution();
/* ------------------------------------------------------------------------- */
void wait_for_resume_execution();
/* ------------------------------------------------------------------------- */
void handle_sigint (int signum);
/* ------------------------------------------------------------------------- */
void handle_sigterm (int signum);
/* ------------------------------------------------------------------------- */
void handle_resume_testrun(int signum);
/* ------------------------------------------------------------------------- */
#endif                          /* EXECUTOR_H */
/* End of file */

