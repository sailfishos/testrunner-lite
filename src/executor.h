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
#define SHELLCMD_ARG1  "-c"
#define FAILURE_INFO_TIMEOUT "timeout"
#define POLL_TIMEOUT_MS   100
#define POLL_TIMEOUT_US   (1000*POLL_TIMEOUT_MS)

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
	unsigned soft_timeout;
	unsigned hard_timeout;
	/* output parameters */
	pid_t pid;
	stream_data stdout_data;
	stream_data stderr_data;
	stream_data failure_info;
	time_t start_time;
	time_t end_time;
	int result;
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

int execute(const char* command, exec_data* data);
void init_exec_data(exec_data* data);
void clean_exec_data(exec_data* data);
void init_stream_data(stream_data* data, int allocate);
void clean_stream_data(stream_data* data);

/* ------------------------------------------------------------------------- */
#endif                          /* EXECUTOR_H */
/* End of file */

