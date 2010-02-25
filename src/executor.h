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

#ifndef TEMPLATE_H
#define TEMPLATE_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
/* None */

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* DATA TYPES */
/* ------------------------------------------------------------------------- */
typedef void (*output_processor_callback)(int stdout_fd, int stderr_fd);

struct _stream_data {
	unsigned char* buffer;
	int size;
	int length;
};

typedef struct _stream_data stream_data;

struct _exec_data {
	stream_data stdout_data;
	stream_data stderr_data;
	int result;
	unsigned timeout;
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
void init_stream_data(stream_data* data);
void clean_stream_data(stream_data* data);

/* ------------------------------------------------------------------------- */
#endif                          /* TEMPLATE_H */
/* End of file */

