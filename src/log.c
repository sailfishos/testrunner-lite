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
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <curl/curl.h>
#include "testrunnerlite.h"
#include "log.h"

/* ------------------------------------------------------------------------- */
/* EXTERNAL DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* EXTERNAL GLOBAL VARIABLES */
extern struct timeval created;

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
LOCAL CURL *curl;

/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */

/* Match these to log.h log_message_types */
const char *stream_names[] = {"ERROR", "INFO", "DEBUG", "WARNING", "UNKNOWN" };

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
LOCAL int verbosity_level = 0;


/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL char *create_msg(const char *fmt, ...);
/* ------------------------------------------------------------------------- */
LOCAL char *vcreate_msg (const char *fmt, va_list ap);
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/** Allocate buffer and print to message to it 
 *  @param fmt format as in printf
 *  @return buffer with message or NULL in case of too big message
 */
LOCAL char *create_msg (const char *fmt, ...)
{
           int written, size = 128;
           char *buff, *new_buff;
           va_list ap;

           if ((buff = malloc(size)) == NULL) {
		   fprintf (stderr, "%s: %s: malloc() failed can not log %s\n",
			    PROGNAME, __FUNCTION__, strerror (errno));
		   return NULL;
	   }
           while (1) {
               va_start(ap, fmt);
               written = vsnprintf(buff, size, fmt, ap);
               va_end(ap);
               if (written > -1 && written < size)
		       return buff;
	       size = size * 2;
	       if (size > LOG_MSG_MAX_SIZE) {
		       fprintf (stderr, 
				"%s: %s: log msg max size exceeded, "
				"msg omitted\n",
				PROGNAME, __FUNCTION__);
		       free(buff);
		       return NULL;
	       }
               if ((new_buff = realloc (buff, size)) == NULL) {
		       fprintf (stderr, 
				"%s: %s: realloc() failed can not log %s\n",
				PROGNAME, __FUNCTION__, strerror (errno));
		       free(buff);
		       return NULL;
               } else {
		       buff = new_buff;
               }
           }

	   return NULL;
}
/* ------------------------------------------------------------------------- */
/** Allocate buffer and print to message to it 
 *  @param fmt format as in printf
 *  @param ap argument list
 *  @return buffer with message or NULL in case of too big message
 */
LOCAL char *vcreate_msg (const char *fmt, va_list ap)
{
           int written, size = 128;
           char *buff, *new_buff;

           if ((buff = malloc(size)) == NULL) {
		   fprintf (stderr, "%s: %s: malloc() failed can not log %s\n",
			    PROGNAME, __FUNCTION__, strerror (errno));
		   return NULL;
	   }
           while (1) {
               written = vsnprintf(buff, size, fmt, ap);
               if (written > -1 && written < size)
		       return buff;
	       size = size * 2;
	       if (size > LOG_MSG_MAX_SIZE) {
		       fprintf (stderr, 
				"%s: %s: log msg max size exceeded, "
				"msg omitted\n",
				PROGNAME, __FUNCTION__);
		       free(buff);
		       return NULL;
	       }
               if ((new_buff = realloc (buff, size)) == NULL) {
		       fprintf (stderr, 
				"%s: %s: realloc() failed can not log %s\n",
				PROGNAME, __FUNCTION__, strerror (errno));
		       free(buff);
		       return NULL;
               } else {
		       buff = new_buff;
               }
           }

	   return NULL;
}

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
	char *msg, *post_msg;
	CURLcode res;
	struct tm *tm;
	time_t current_time;
	struct timeval now, diff;
	va_list args;
	unsigned int to_python_level[] = {40,20,10,30,0};
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
	va_start(args, format);
	msg = vcreate_msg (format, args);
	va_end(args);


	if (!msg)
		return;
	
	fprintf (stdout, "%s\n", msg);
	
	if (!curl)
		return;

	gettimeofday (&now, NULL);
	timersub (&now, &created, &diff);
	post_msg = create_msg ("levelno=%d&"
			       "name=testrunner-lite&"
			       "levelname=%s&" 
			       "module=testrunner-lite&"
			       "filename=None&"
			       "pathname=None&"
			       "lineno=None&"
			       "msg=%s&"
			       "exc_info=None&"
			       "exc_text=None&"
			       "args=()&"
			       "threadName=None&"
			       "thread=0.0&"
			       "created=%d.%d&"
			       "process=%d&"
			       "relativeCreated=%d.%d&"
			       "msecs=%d.%d&"
			       ,to_python_level[verbosity_level],
			       stream_name,
			       msg,
			       created.tv_sec,
			       created.tv_usec,
			       getpid(),
			       diff.tv_sec,
			       diff.tv_usec,
			       diff.tv_usec ? diff.tv_usec/1000 : 0,
			       diff.tv_usec % 1000);
	if (!post_msg) {
		free (msg);
		return;
	}

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_msg);
	
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf (stdout, "[%s] [ERROR] http logging failed: %s\n ", 
			 timestamp, curl_easy_strerror(res));

		log_close();
	}
	free (msg);
	free (post_msg);
	return;
}

/* ------------------------------------------------------------------------- */
/** Sets the verbosity level
 * @param opts testrunner lite options
 */
void log_init (testrunner_lite_options *opts) {
    
	verbosity_level = 0;
    
	if (opts->log_level > 0 && opts->log_level < LOG_LEVELS_COUNT) {
		verbosity_level = opts->log_level;
		log_msg (LOG_INFO, "Verbosity level set to: %d\n", 
			 opts->log_level);
	} else {
		log_msg (LOG_ERROR, 
			 "Incorrect verbosity level %d, values [0..%d]\n", 
			 opts->log_level, LOG_LEVELS_COUNT - 1);
	}
	if (opts->remote_logger) {
		curl =	curl_easy_init();
		curl_easy_setopt(curl, 
				 CURLOPT_URL, 
				 opts->remote_logger);
		if (opts->remote_logger_port)
			curl_easy_setopt(curl, 
					 CURLOPT_PORT, 
					 opts->remote_logger_port);
			
	}
}

/* ------------------------------------------------------------------------- */
/** Closes the log, flush stdout and cleanup curl if it's in use
 */
void log_close () {
	fflush (stdout);
	if (curl) {
		curl_easy_cleanup(curl);
		curl = NULL;
	}
}


/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

