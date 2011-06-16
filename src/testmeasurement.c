/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010,2011 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Sampo Saaristo <sampo.saaristo@sofica.fi>
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
#include <getopt.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "testrunnerlite.h"
#include "testdefinitiondatatypes.h"
#include "testmeasurement.h"
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
/* None */

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
typedef struct {
	int verdict;
	char *failure_string;
} container;

typedef struct {
	td_measurement_series *series;
	container *cont;
} item_extra_data;

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL int parse_measurements (FILE *f, xmlListPtr list);
/* ------------------------------------------------------------------------- */
LOCAL int parse_series_header(char *header, td_measurement_series **series);
/* ------------------------------------------------------------------------- */
LOCAL int parse_timestamp(char *line, struct timespec *timestamp);
/* ------------------------------------------------------------------------- */
LOCAL int parse_series (FILE *f, xmlListPtr list);
/* ------------------------------------------------------------------------- */
LOCAL int eval_meas (const void *data, const void *user);
/* ------------------------------------------------------------------------- */
LOCAL int eval_measurement_item(const void *data, const void *user);
/* ------------------------------------------------------------------------- */
LOCAL int eval_measurement_series(const void *data, const void *user);
/* ------------------------------------------------------------------------- */
LOCAL void add_measurement_item(td_measurement_series *series,
				struct timespec *timestamp,
				double value);
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/** Parse measurement data from file into list 
 *  @param f file containing measurement data
 *  @param list list to save data into 
 *  @return 0 on success, 1 on failure e.g. parse failure  
 */
LOCAL int parse_measurements (FILE *f, xmlListPtr list)
{
	int present, c;
	char line[4096], *p, *name, *unit;
	char *value = "", *target = "", *failure = "";
	td_measurement *meas;

	while (fgets (line, 4095, f)) {
		c = line[0];
		if (isspace((int)c))
			continue;
		LOG_MSG (LOG_DEBUG, "measurement: %s", line);
		present = 0;
		p = strtok (line, ";");
		if (p) 
			name = p;
		else {
			LOG_MSG (LOG_WARNING, "parsing measurement fail,"
				 "no 'name'");
			goto ERR_OUT;
		}
		p = strtok (NULL, ";");
		if (p)
			value = p;
		else {
			LOG_MSG (LOG_WARNING, "parsing measurement fail,"
				 "no 'value'");
			goto ERR_OUT;
		}
		p = strtok (NULL, ";");
		if (p)
			unit = p;
		else {
			LOG_MSG (LOG_WARNING, "parsing measurement fail,"
				 "no 'unit'");
			goto ERR_OUT;
		}

		p = strtok (NULL, ";");
		if (p && !isspace ((int)*p)) {
			present = 1;
			target = p;
			p = strtok (NULL, ";");
			if (!p || !strlen (p)) {
				LOG_MSG (LOG_WARNING, "parsing measurement "
					 "fail no 'failure' value");
				goto ERR_OUT;
			}
			failure = p;
		}
		/* Succesfully parsed, add to list of measurements */
		meas = (td_measurement *)malloc (sizeof (td_measurement));
		if (meas == NULL) {
			LOG_MSG (LOG_ERR, "%s: FATAL : OOM", __FUNCTION__,
				 PROGNAME);
			goto ERR_OUT;
		}
		memset (meas, 0x0, sizeof (td_measurement));
		meas->name = xmlCharStrdup (name);
		meas->value = strtod (value, NULL);
		meas->unit = xmlCharStrdup (unit);
		meas->target_specified = present;
		if (present) {
			meas->target = strtod (target, NULL);
			meas->failure = strtod (failure, NULL);
		}
		if (xmlListAppend (list, meas)) {
			LOG_MSG (LOG_ERR, "%s adding to list failed", 
				 __FUNCTION__);
			goto ERR_OUT;
		}
	}
	return 0;
 ERR_OUT:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Parse a measurement series header. The format of the header line is
 *  name;unit[;target;failure]
 *  The fields are separated by semicolons. Target and failure are optional.
 * 
 * @param header A string containing the header line
 * @param series pointer to td_measurement_series pointer to store new series
 * 
 * @return 0 on success, 1 on failure
 */
LOCAL int parse_series_header(char *header, td_measurement_series **series)
{
	char *name, *unit, *target, *failure;
	char *endptr;

	name = strtok(header, ";");
	if (name == NULL) {
		LOG_MSG (LOG_ERR, "Cannot parse measurement series name");
		goto ERR_OUT;
	}

	unit = strtok(NULL, ";");
	if (unit == NULL) {
		LOG_MSG (LOG_ERR, "Cannot parse measurement series unit");
		goto ERR_OUT;
	}

	target = strtok(NULL, ";");
	failure = strtok(NULL, ";");

	*series = td_measurement_series_create();
	(*series)->name = xmlCharStrdup(name);
	(*series)->unit = xmlCharStrdup(unit);

	do {
		if (target == NULL || failure == NULL) {
			break;
		}

		(*series)->target = strtod(target, &endptr);
		if (target == endptr) {
			LOG_MSG (LOG_WARNING, "Cannot parse 'target'"
				 " for measurement series");
			break;
		}

		(*series)->failure = strtod(failure, &endptr);
		if (failure == endptr) {
			LOG_MSG (LOG_WARNING, "Cannot parse 'failure'"
				 " for measurement series");
			break;
		}

		(*series)->target_specified = 1;
	} while(0);

	return 0;

 ERR_OUT:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Parse a timestamp in measurement series. The timestamp format is
 *  yyyy-mm-ddThh:mm:ss[.ssssss]
 *  which conforms to ISO 8601. The fractional part of seconds is optional.
 *  Examples:
 *    2011-03-08T09:22:54
 *    2011-03-08T09:22:54.005020
 * 
 * @param line A string having the timestamp in the beginning
 * @param timestamp A pointer to struct where to store the timestamp
 * 
 * @return 0 on success, 1 on failure
 */
LOCAL int parse_timestamp(char *line, struct timespec *timestamp)
{
	struct tm tm;
	char *p;
	double frac;
	char *endptr;

	memset(&tm, 0, sizeof(struct tm));
	p = strptime(line, "%FT%T", &tm);
	if (p == NULL)
		goto ERR_OUT;

	timestamp->tv_sec = mktime(&tm);
	timestamp->tv_nsec = 0L;

	/* check if there is a fractional part of seconds */
	if (*p == '.') {
		frac = strtod(p, &endptr);
		if (p != endptr) {
			timestamp->tv_nsec = (long)(frac * 1000000000.0 + 0.5);
			p = endptr;
		} else {
			LOG_MSG (LOG_WARNING, "Invalid fractional part"
				 " in timestamp of measurement series");
		}
	}

	if(*p != ';') {
		/* only semicolon allowed after timestamp */
		goto ERR_OUT;
	}

	return 0;

 ERR_OUT:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Parse measurement series data from file into list 
 *  @param f file containing measurement series data
 *  @param list list to save data into 
 *  @return 0 on success, 1 on failure e.g. parse failure  
 */
LOCAL int parse_series (FILE *f, xmlListPtr list)
{
	char line[1024];
	struct timespec timestamp = { 0, 0L };
	td_measurement_series *series = NULL;
	double value;
	char *p, *endptr;
	int n;
	int has_timestamp = 0;

	for (n = 0; fgets (line, sizeof(line), f); n++) {
		has_timestamp = 0;

		if (n == 0) {
			/* the first line is header */
			if (parse_series_header(line, &series)) {
				goto ERR_OUT;
			}
			xmlListAppend(list, series);
			continue;
		}

		/* search for a ';' dividing optional timestamp and value */
		p = strchr(line, (int)';');
		if (p) {
			if (parse_timestamp(line, &timestamp)) {
				LOG_MSG (LOG_WARNING, "Cannot parse timestamp"
				 " in measurement series");
				continue;
			} else {
				has_timestamp = 1;
			}
			p++;
		} else {
			/* no timestamp */
			p = line;
		}

		/* convert value string */
		value = strtod(p, &endptr);
		if (endptr == p) {
			LOG_MSG (LOG_WARNING, "Invalid value in"
				 " measurement series");
			continue;
		}

		/* add valid measurement series item */
		add_measurement_item(series,
				     has_timestamp ? &timestamp : NULL,
				     value);
	}

	return 0;

 ERR_OUT:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Evaluate single measurement data
 *  @param data measurement data
 *  @param user container for verdict and failure info
 *  @return 1 on success, 0 if measurement fails
 */
LOCAL int eval_meas (const void *data, const void *user)
{
	int retval = 1;
	size_t len;
	td_measurement *meas = (td_measurement *)data;
	container *cont = (container *)user;

	LOG_MSG (LOG_DEBUG, "MEASUREMENT: %s(%s) %f %d %f %f",
		 meas->name, meas->unit, meas->value, meas->target_specified,
		 meas->target, meas->failure);

	if (!meas->target_specified)
		return retval; /* No need to do further processing */

	if (meas->target > meas->failure) {
		if (meas->value <= meas->failure) {
			len = strlen ((char *)meas->name) + 
				strlen ("measured value too small") + 5;
			cont->failure_string = malloc (len); 
			snprintf (cont->failure_string, len, "%s - "
				  "measured value too small", meas->name); 
			cont->verdict = CASE_FAIL;
			retval = 0;
		}
	} else if (meas->target < meas->failure) {
		if (meas->value >= meas->failure) {
			cont->verdict = CASE_FAIL;
			len = strlen ((char *)meas->name) + 
				strlen ("measured value too big") + 5;
			cont->failure_string = malloc (len);
			snprintf (cont->failure_string, len, "%s - "
				 "measured value too big", meas->name); 
			retval = 0;
		}
	} else {
		LOG_MSG (LOG_WARNING, "Invalid measurement data (%s) "
			 "target and failure are the same (%f)", meas->name,
			 meas->target);
	}
	return retval;
}
/* ------------------------------------------------------------------------- */
/** Evaluate a measurement series item against failure limit of series
 * 
 * @param data measurement item data
 * @param user measurement item extra data
 * 
 * @return 1 on success, 0 if measurement fails
 */
#define SERIES_FAIL_BELOW  "measurement series value below failure limit"
#define SERIES_FAIL_OVER   "measurement series value over failure limit"
LOCAL int eval_measurement_item(const void *data, const void *user)
{
	int retval = 1;
	td_measurement_item *item = (td_measurement_item *)data;
	item_extra_data *extra = (item_extra_data *)user;
	td_measurement_series *series = (td_measurement_series *)extra->series;
	size_t len;

	if (series->target > series->failure) {
		if (item->value <= series->failure) {
			extra->cont->verdict = CASE_FAIL;
			len = strlen ((char *)series->name) + 
			      strlen (SERIES_FAIL_BELOW) + 5;
			extra->cont->failure_string = (char *)malloc (len);
			snprintf (extra->cont->failure_string, len, "%s - "
				 SERIES_FAIL_BELOW, series->name);
			retval = 0;
		}
	} else if (series->target < series->failure) {
		if (item->value >= series->failure) {
			extra->cont->verdict = CASE_FAIL;
			len = strlen ((char *)series->name) +
				strlen (SERIES_FAIL_OVER) + 5;
			extra->cont->failure_string = (char *)malloc (len);
			snprintf (extra->cont->failure_string, len, "%s - "
				 SERIES_FAIL_OVER, series->name);
			retval = 0;
		}
	} else {
		LOG_MSG (LOG_WARNING, "%Invalid measurement series (%s), "
			 "target and failure are the same", series->name);
	}

	return retval;
}
/* ------------------------------------------------------------------------- */
/**  Evaluate a measurement series result
 * 
 * @param data measuremen series data
 * @param user container for verdict and failure info
 * 
 * @return 1 on success, 0 if measurement fails
 */
LOCAL int eval_measurement_series(const void *data, const void *user)
{
	int retval = 1;
	td_measurement_series *series = (td_measurement_series *)data;
	item_extra_data extra;

	extra.series = series;
	extra.cont = (container *)user;

	/* no evaluation if target hasn't been specified */
	if (!series->target_specified)
		return retval;

	xmlListWalk(series->items, eval_measurement_item, &extra);

	if (extra.cont->verdict == CASE_FAIL) {
		retval = 0;
	}

	return retval;
}
/* ------------------------------------------------------------------------- */
/** Append an measurement item to measurement series
 * 
 * @param series Pointer to measurement series
 * @param timestamp Pointer to timestamp or NULL if not specified
 * @param value The measurement value
 */
LOCAL void add_measurement_item(td_measurement_series *series,
				struct timespec *timestamp,
				double value)
{
	td_measurement_item *item = td_measurement_item_create();

	if (item) {
		item->value = value;
		if (timestamp) {
			item->has_timestamp = 1;
			item->timestamp = *timestamp;
		}
		xmlListAppend(series->items, item);
	}
}
/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** Get measurement results from file 
 *  @param file path to file
 *  @param c td_case to store measurements
 *  @param series 1 if a file is measurement series, otherwise 0
 *  @return 0 on success, 1 on error
 */
int get_measurements (const char *file, td_case *c, int series)
{
	FILE *f;

	f = fopen (file, "r");
	if (!f) {
		LOG_MSG (LOG_WARNING, "Failed to open measurement file %s: %s",
			 file, strerror (errno));
		goto ERR_OUT;
	}
	
	if (series) {
		if (parse_series(f, c->series)) {
			LOG_MSG (LOG_WARNING, "Failed to parse measurement"
				 " series file %s", file);
			goto ERR_OUT;
		}
	} else {
		if (parse_measurements (f, c->measurements)) {
			LOG_MSG (LOG_WARNING, "Failed to parse measurement"
				 " file %s", file);
			goto ERR_OUT;
		}
	}
	
	return 0;
 ERR_OUT:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Get measurement results from file 
 *  @param case td_case to store measurements verdict and info
 *  @param verdict [OUT] used to pass PASS/FAIL verdict to caller
 *  @param fail_string [OUT] used to pass failure info to caller
 *  @param series 1 if to use 'series' member of td_case, 0 if 'measurement'
 *  @return 0 on success, 1 on error
 */
int eval_measurements (td_case *c, int *verdict,
		       char **fail_string, int series)
{
	container cont;
	cont.verdict = CASE_PASS;
	cont.failure_string = NULL;

	if (series) {
		xmlListWalk (c->series, eval_measurement_series, &cont);
	} else {
		xmlListWalk (c->measurements, eval_meas, &cont);
	}
	
	*verdict = cont.verdict;
	*fail_string = cont.failure_string;

	return 0;
}
/* ------------------------------------------------------------------------- */
/** Process a current measurement file produced by hat_drv and add results
 *  to the case.
 * 
 * @param filename File name of the current measurement data
 * @param c Pointer to td_case to store results
 * 
 * @return Always 0
 */
int process_current_measurement(const char *filename, td_case *c)
{
	char line[80];
	char *endptr;
	int n = 1;
	double value;
	td_measurement_series *series = NULL;

	FILE *f = fopen(filename, "r");

	if (f != NULL) {
		while (fgets(line, 80, f)) {
			value = strtod(line, &endptr);

			if (endptr == line) {
				/* no conversion was performed */
				LOG_MSG (LOG_WARNING, "Invalid line in "
					 "current measurement data");
				continue;
			}

			if (n++ == 1) {
				/* create current sample series */
				series = td_measurement_series_create();
				series->name = xmlCharStrdup("Current samples");
				series->group = xmlCharStrdup("Current measurement");
				series->unit = xmlCharStrdup("mA");
				series->has_interval = 1;
				series->interval = 200;
				series->interval_unit = xmlCharStrdup("ms");
				xmlListAppend(c->series, series);
			}

			add_measurement_item(series, NULL, value);
		}
		fclose(f);
	} else {
		LOG_MSG (LOG_ERR, "Can't open current measurement results");
	}

	return 0;
}
/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

