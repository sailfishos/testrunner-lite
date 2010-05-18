/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Sampo Saaristo <ext-sampo.2.saaristo@nokia.com>
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
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <libxml/list.h>

#include "testrunnerlite.h"
#include "testfilters.h"
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
LOCAL 	xmlListPtr suite_filter_list;
LOCAL 	xmlListPtr set_filter_list;
LOCAL 	xmlListPtr case_filter_list;

/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
/* None */
/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL void filter_delete (xmlLinkPtr lk);
/* ------------------------------------------------------------------------- */
LOCAL int filter_list_compare (const void * data0, const void * data1);
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/** Deallocator for test_filters called by xmlListDelete
 *  @param lk list iterator
 */
LOCAL void filter_delete (xmlLinkPtr lk)
{
	test_filter *filter = xmlLinkGetData (lk);
	if (filter->key)
		free (filter->key);
	if (filter->value)
		free (filter->value);

	free (filter);
}
/* ------------------------------------------------------------------------- */
/** Comparison function for list without ordering
 *  @param data0 string to compare - not used
 *  @param data1 string to compare - not used
 *  @return 0 always
 */
LOCAL int filter_list_compare (const void * data0, 
			       const void * data1)
{
	
	return 0;
}

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** Initialize filter list */
void init_filters()
{
	suite_filter_list = xmlListCreate (filter_delete, filter_list_compare);
	set_filter_list = xmlListCreate (filter_delete, filter_list_compare);
	case_filter_list = xmlListCreate (filter_delete, filter_list_compare);
}
/* ------------------------------------------------------------------------- */
/** Parses the filter string given from commandline and saves the data in 
 *  to list of filters
 *  @param filters filter string
 *  @return 0 on success, 1 on parsing failure
 */
int parse_filter_string (char *filters)
{
	int in_quotes = 0, done = 0;
	char *p, *f, *key, *value, c;

	/* Make a local copy of filter string */
	LOG_MSG (LOG_INFO, "string to parse %s", filters);
	f = (char *)malloc (strlen(filters) + 1);
	strcpy (f, filters);
	p = f;
	while (p && !done) {
		/* Find filter key */
		key = p;
		p = strchr (p, '=');
		if (p) {
			*p = '\0';
			p++;
		} else {
			LOG_MSG (LOG_INFO, "failed to parse filter string - "
				 "missing '='\n");
			goto err_out;
		}
		/* Find filter value */
		value = p;
		do {
			c = *p;
			switch (c) {
			case '"':
				in_quotes = !in_quotes;
				p ++;
				break;
			case ' ':
				if (!in_quotes)
					c = *p = '\0';
				p ++;
				break;
			case '\0':
				done = 1;
				break;
			default:
				p++;
			}
		 } while (c != '\0');
		if (in_quotes) {
			LOG_MSG (LOG_ERROR, 
				 "failed to parse filter string - "
				 "mismatched \"");
			goto err_out;
		}
		LOG_MSG (LOG_INFO, "FILTER key=%s,value=%s\n",
			 key, value);
	}
	
	free (f);
	return 0;
 err_out:
	if (f) free (f);
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Do filtering on suite level
 *  @param s suite to be checked against filters
 *  @return 1 if the suite is filtered, 0 if not.
 */
int filter_suite (td_suite *s)
{
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Do filtering on set
 *  @param s set to be checked against filters
 *  @return 1 if the set is filtered, 0 if not.
 */
int filter_set (td_set *s)
{
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Do filtering on case
 *  @param c case to be checked against filters
 *  @return 1 if the case is filtered, 0 if not.
 */
int filter_case (td_case *c)
{
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Free the memory allocated for filters
 */
void cleanup_filters ()
{
	xmlListDelete (suite_filter_list);
	xmlListDelete (set_filter_list);
	xmlListDelete (case_filter_list);
}
/* ------------------------------------------------------------------------- */

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
