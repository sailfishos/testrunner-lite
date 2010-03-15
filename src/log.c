/* * This file is part of testrunner-lite *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 *
 * Contact: 
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
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>

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
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */

/* Match these to log.h log_message_types */
const char *stream_names[] = {"ERROR", "INFO", "DEBUG", "WARNING", "UNKNOWN" };

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
static int verbosity_level = 0;

/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
/* None */

/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* None */

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */ 

/** Prints a log message to stdout in format "[LOG_TYPE] HH:MM:SS message"
 *  Usage is the same as in regular printf(), except the first parameter
 *  Example: log_msg (LOG_ERR, "Error message %s\n", "Failure");
 * @param type Log type defined in log_message_types enum
 * @param format Message format
 */
void log_msg (int type, char *format, ...) {
	
    const char *stream_name;
    char timestamp[10];
    struct tm *tm;
    time_t current_time;
    va_list args;
    
	/* Check if message should be printed */
	if (verbosity_level == LOG_LEVEL_SILENT || 
        (type == LOG_DEBUG && verbosity_level != LOG_LEVEL_DEBUG)) {
		/* Do nothing */
		return;
	} 

	/* All messages go to stdout, also errors */
    
    /* Log name. */
	if (type >= 0 && type < LOG_TYPES_COUNT) {
		stream_name = stream_names[type];
	} else {
		/* Should not happen */
		stream_name = stream_names[LOG_TYPES_COUNT];
	}

	/* Current timestamp */	
	time (&current_time);
	tm = localtime (&current_time);
	strftime (timestamp, sizeof (timestamp), "%H:%M:%S", tm);
	
	fprintf (stdout, "[%s] %s ", stream_name, timestamp);

	/* Print given arguments */
	va_start (args, format);
	vfprintf (stdout, format, args);
	va_end (args);

	fprintf (stdout, "\n");

}

/* ------------------------------------------------------------------------- */
/** Sets the verbosity level
 * @param level Verbosity level
 */
void log_set_verbosity_level (int level) {
    
    verbosity_level = 0;
    
	if (level > 0 && level < LOG_LEVELS_COUNT) {
		verbosity_level = level;
        log_msg (LOG_INFO, "Verbosity level set to: %d\n", level);
	} else {
		log_msg (LOG_ERROR, 
			"Incorrect verbosity level %d, values [0..%d]\n", 
			level, LOG_LEVELS_COUNT - 1);
	}
}


/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

