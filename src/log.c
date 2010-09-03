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

/* ------------------------------------------------------------------------- */
/* INCLUDE FILES */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <unistd.h>
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
LOCAL int  do_syslog = 0;

/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */

/* Match these to priorities in syslog.h */
const char *stream_names[] = {"EMERG", "ALERT", "CRITICAL", "ERROR",
			      "WARNING", "NOTICE", "INFO", "DEBUG", "UNKNOWN" };

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
 * @param file Source file name the log entry corresponds to
 * @param function Function emitting the log
 * @param lineno Line number
 * @param format Message format
 */
void log_msg(int type, const char *file, const char *function,
	     int lineno, char *format, ...) 
{
	
	const char *stream_name;
	char timestamp[10];
	char *msg, *post_msg, *url_enc_msg,*module, *p;
	CURLcode res;
	struct tm *tm;
	time_t current_time;
	struct timeval now, diff;
	va_list args;
	/* Syslog:
	   LOG_EMERG	0
	   LOG_ALERT	1
	   LOG_CRIT	2	
	   LOG_ERR	3	
	   LOG_WARNING	4	
	   LOG_NOTICE	5
	   LOG_INFO	6	
	   LOG_DEBUG	7	
	   Python:
	   CRITICAL 	50
	   ERROR 	40
	   WARNING 	30
	   INFO         20
	   DEBUG 	10
	   NOTSET 	0 
	*/
	unsigned int to_python_level[] = {50, 50, 50, 40,
					  30, 20, 20, 10, 0};

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
	if (type == LOG_DEBUG)
		fprintf (stdout, "%s %s() %d ", file, function, lineno);

	/* Print given arguments */
	va_start(args, format);
	msg = vcreate_msg (format, args);
	va_end(args);

	if (!msg)
		return;

	if (do_syslog)
		syslog (type, "%s", msg);
	
	fprintf (stdout, "%s\n", msg);
	fflush (stdout);
	
	if (!curl) {
		free (msg);
		return;
	}
	/* 
	 * Calculate the elapsed time since this program started
	 */
	gettimeofday (&now, NULL);
	timersub (&now, &created, &diff);
	/*
	** module is the source file name w/o the .c
	*/
	module = (char *)malloc (strlen (file) + 1);
	strcpy (module, file);
	if ((p = strchr (module, '.')))
	    *p = '\0';

	/*
	** Compose http POST
	*/
	url_enc_msg = curl_easy_escape (curl, msg, strlen(msg));
	post_msg = create_msg ("levelno=%d&"
			       "name=testrunner-lite&"
			       "levelname=%s&" 
			       "module=%s&"
			       "filename=%s&"
			       "pathname=testrunner-lite/src&"
			       "funcName=%s&"
			       "lineno=%d&"
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
			       ,to_python_level[type],
			       stream_name,
			       module,
			       file,
			       function,
			       lineno,
			       url_enc_msg,
			       created.tv_sec,
			       created.tv_usec,
			       getpid(),
			       diff.tv_sec,
			       diff.tv_usec,
			       diff.tv_usec ? diff.tv_usec/1000 : 0,
			       diff.tv_usec % 1000);
	if (!post_msg) {
		curl_free (url_enc_msg);
		free (msg);
		return;
	}

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_msg);
	
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf (stdout, "[ERROR] %s http logging failed: %s\n ", 
			 timestamp, curl_easy_strerror(res));

		log_close();
	}
	curl_free (url_enc_msg);
	free (msg);
	free (post_msg);
	free (module);
	
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
		LOG_MSG (LOG_INFO, "Verbosity level set to: %d\n", 
			 opts->log_level);
	} else {
		LOG_MSG (LOG_ERR, 
			 "Incorrect verbosity level %d, values [0..%d]\n", 
			 opts->log_level, LOG_LEVELS_COUNT - 1);
	}
	if (opts->remote_logger) {
		curl =	curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, 
				 CURLOPT_URL, 
				 opts->remote_logger);
		if (opts->remote_logger_port)
			curl_easy_setopt(curl, 
					 CURLOPT_PORT, 
					 opts->remote_logger_port);
			
	}
	if (opts->syslog_output) {
		openlog ("testrunner-lite", 0, LOG_LOCAL1);
		do_syslog = 1;
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
	if (do_syslog)
		closelog();
}


/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

