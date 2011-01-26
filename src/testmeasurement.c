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

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL int parse_measurements (FILE *f, xmlListPtr list);
/* ------------------------------------------------------------------------- */
LOCAL int eval_meas (const void *data, const void *user);
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
/** Evaluate single measurement data
 *  @param data measurement data
 *  @param user container for verdict and failure info
 *  @return 1 on success, 0 if measurement fails
 */
LOCAL int eval_meas (const void *data, const void *user)
{
	int retval = 1;
	td_measurement *meas = (td_measurement *)data;
	container *cont = (container *)user;

	LOG_MSG (LOG_DEBUG, "MEASUREMENT: %s(%s) %f %d %f %f",
		 meas->name, meas->unit, meas->value, meas->target_specified,
		 meas->target, meas->failure);

	if (!meas->target_specified)
		return retval; /* No need to do further processing */

	if (meas->target > meas->failure) {
		if (meas->value <= meas->failure) {
			cont->failure_string = malloc (strlen ((char *)
							       meas->name) + 
						       strlen ("measured "
							       "value too" 
							       "small") + 5); 
			sprintf (cont->failure_string, "%s - "
				 "measured value too small", meas->name); 
			cont->verdict = CASE_FAIL;
			retval = 0;
		}
	} else if (meas->target < meas->failure) {
		if (meas->value >= meas->failure) {
			cont->verdict = CASE_FAIL;
			cont->failure_string = malloc (strlen ((char *)
							       meas->name) + 
						       strlen ("measured "
							       "value too" 
							       "small") + 5); 
			sprintf (cont->failure_string, "%s - "
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
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** Get measurement results from file 
 *  @param file path to file
 *  @param measurements list to store measurement data in
 *  @return 0 on success, 1 on error
 */
int get_measurements (const char *file, xmlListPtr measurements)
{
	FILE *f;

	f = fopen (file, "r");
	if (!f) {
		LOG_MSG (LOG_WARNING, "Failed to open measurement file %s: %s",
			 file, strerror (errno));
		goto ERR_OUT;
	}
	
	if (parse_measurements (f, measurements)) {
		LOG_MSG (LOG_WARNING, "Failed to parse measurement file %s",
			 file);
		goto ERR_OUT;
	}
	
	return 0;
 ERR_OUT:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Get measurement results from file 
 *  @param measurements list of measurement data
 *  @param verdict [OUT] used to pass PASS/FAIL verdict to caller
 *  @param fail_string [OUT] used to pass failure info to caller
 *  @return 0 on success, 1 on error
 */
int eval_measurements (xmlListPtr measurements, int *verdict, 
		       char **fail_string)
{
	container cont;
	cont.verdict = CASE_PASS;
	cont.failure_string = NULL;

	xmlListWalk (measurements, eval_meas, &cont);
	
	*verdict = cont.verdict;
	*fail_string = cont.failure_string;

	return 0;
}
/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

