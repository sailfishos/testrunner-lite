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
LOCAL int validate_and_add_filter (char *key, char *values);
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
/** Check that filter type seems correct and adds to correc list
 *  @param filter filter
 *  @return 0 on success, 1 on failure
 */
LOCAL int filter_add (test_filter *filter)
{
	/* TODO add all filter types to correct list */
	if (!strcasecmp ((char *)filter->key, "environment")) {
		;
	}
	else if (!strcasecmp ((char *)filter->key, "manual")) {
		return xmlListAppend (case_filter_list, filter);
	}	
	else if (!strcasecmp ((char *)filter->key, "domain")) {
		;
	}
	else if (!strcasecmp ((char *)filter->key, "feature")) {
		;
	}
	else if (!strcasecmp ((char *)filter->key, "requirement")) {
		return xmlListAppend (case_filter_list, filter);
	}
	else if (!strcasecmp ((char *)filter->key, "type")) {
		;
	}
	else if (!strcasecmp ((char *)filter->key, "insignificant")) {
		;
	}
	else if (!strcasecmp ((char *)filter->key, "level")) {
		;
	}
	else if (!strcasecmp ((char *)filter->key, "subfeature")) {
		;
	}
	else if (!strcasecmp ((char *)filter->key, "testsuite")) {
		return xmlListAppend (suite_filter_list, filter);
	}
	else if (!strcasecmp ((char *)filter->key,  "testset")) {
		return xmlListAppend (set_filter_list, filter);
	}
	else if (!strcasecmp ((char *)filter->key, "testcase")) {
		return xmlListAppend (case_filter_list, filter);
	} else {
		LOG_MSG (LOG_ERROR, "Unknow filter type %s",
			 filter->key);
		free (filter->key);
		free (filter->value);
		free (filter);
		return 1;
	}

	return 0;
}
/* ------------------------------------------------------------------------- */
/** Validate filter semantics, parse value list and add filter to correct list
 *  @param key filter key
 *  @param values list of values
 *  @return 0 on success, 1 on failure
 */
LOCAL int validate_and_add_filter (char *key, char *values)
{
	int exclude = 0, retval = 0;
	xmlChar *k = NULL, *val;
	char *p;
	test_filter *filter = NULL;

	/* handle possible +/- in beginning of key */
	if (*key == '-') {
		exclude = 1;
		k = xmlCharStrdup (key + 1);
	} else if (*key == '+')
		k = xmlCharStrdup (key + 1);
	else
		k = xmlCharStrdup (key);

	/* Go through value list */
	p = strtok (values, ",");
	do {
		/* Clean possible "-signs */
		if (p[0] == '"') {
			if (strlen (p) < 3) {
				LOG_MSG (LOG_ERROR, "empty value");
				retval = 1;
				goto out;
			}
			if (p[strlen(p)-1] != '"') {
				LOG_MSG (LOG_ERROR, "Mismatched \" %s", p);
				retval = 1;
				goto out;
			}
			val = xmlCharStrndup(&p[1], strlen(p)-2);
		} else
			val = xmlCharStrdup (p);
		filter = (test_filter *)malloc (sizeof (test_filter));
		filter->exclude = exclude;
		filter->key = xmlStrdup(k);
		filter->value = val;
		LOG_MSG (LOG_INFO, "new filter: key=%s, value=%s, exculde=%d",
			 filter->key, filter->value, filter->exclude);
		/* Finally try to add filter to correct list */
		if ((retval = filter_add (filter)))
			goto out;
	} while ((p = strtok (NULL, ",")));
 out:
	free (k);
	return retval;
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
	int in_quotes = 0, done = 0, retval = 0;
	char *p, *f, *key = NULL, *value = NULL, c;

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
	retval = validate_and_add_filter (key, value);
	free (f);
	return retval;
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
