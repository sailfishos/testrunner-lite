/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Riku Halonen <riku.halonen@nokia.com>
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

#ifndef LOG_H
#define LOG_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <syslog.h>
#include "testrunnerlite.h"
/* ------------------------------------------------------------------------- */
/* CONSTANTS */
#define LOG_MSG_MAX_SIZE 2048

/* ------------------------------------------------------------------------- */
/* MACROS */
#define LOG_MSG(__level__,__format__, __args__ ...)\
	do{								\
		log_msg (__level__, __FILE__, __FUNCTION__, __LINE__,	\
			 __format__, ##__args__);			\
	} while (0)
#define LOG_TYPES_COUNT LOG_DEBUG + 1

/* ------------------------------------------------------------------------- */
/* DATA TYPES */
/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* STRUCTURES */
enum log_levels {
    LOG_LEVEL_SILENT = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVELS_COUNT, /* number of entries in enum */
};

/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES */

/* ------------------------------------------------------------------------- */
void log_msg(int type, const char *file, const char *function,
	     int lineno, char *format, ...);
/* ------------------------------------------------------------------------- */
void log_init(testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
void log_close (void);
/* ------------------------------------------------------------------------- */

#endif                          /* LOG_H */
/* End of file */

