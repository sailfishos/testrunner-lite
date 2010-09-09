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
#include "utils.h"
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
LOCAL 	xmlListPtr suite_filter_list = NULL;
LOCAL 	xmlListPtr set_filter_list = NULL;
LOCAL 	xmlListPtr case_filter_list = NULL;

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
LOCAL void filter_value_delete (xmlLinkPtr lk);
/* ------------------------------------------------------------------------- */
LOCAL int filter_list_compare (const void * data0, const void * data1);
/* ------------------------------------------------------------------------- */
LOCAL int validate_and_add_filter (char *key, char *values);
/* ------------------------------------------------------------------------- */
LOCAL int filter_exec (const void *data, const void *user) ;
/* ------------------------------------------------------------------------- */
LOCAL int filter_value_print (const void *data, const void *user) ;
/* ------------------------------------------------------------------------- */
LOCAL int manual_filter (test_filter *filter, const void *data);
/* ------------------------------------------------------------------------- */
LOCAL int test_case_filter (test_filter *filter, const void *data);
/* ------------------------------------------------------------------------- */
LOCAL int test_set_filter (test_filter *filter, const void *data);
/* ------------------------------------------------------------------------- */
LOCAL int feature_filter (test_filter *filter, const void *data);
/* ------------------------------------------------------------------------- */
LOCAL int type_filter (test_filter *filter, const void *data);
/* ------------------------------------------------------------------------- */
LOCAL int requirement_filter (test_filter *filter, const void *data);
/* ------------------------------------------------------------------------- */
LOCAL xmlListPtr string2valuelist (char *str);
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
	xmlListDelete (filter->value_list);

	free (filter);
}
/** Deallocator for test_filters value list called by xmlListDelete
 *  @param lk list iterator
 */
LOCAL void filter_value_delete (xmlLinkPtr lk)
{
	xmlChar *val = xmlLinkGetData (lk);
	free (val);
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
/** Comparison function for value list
 *  @param data0 string to compare 
 *  @param data1 string to compare 
 *  @return 0 always
 */
LOCAL int filter_value_list_compare (const void * data0, 
				     const void * data1)
{
	
	return xmlStrcmp (BAD_CAST data0, BAD_CAST data1);
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
		filter->filter = manual_filter;
		return xmlListAppend (case_filter_list, filter);
	}	
	else if (!strcasecmp ((char *)filter->key, "domain")) {
		;
	}
	else if (!strcasecmp ((char *)filter->key, "feature")) {
		filter->filter = feature_filter;
		return xmlListAppend (set_filter_list, filter);
	}
	else if (!strcasecmp ((char *)filter->key, "requirement")) {
		filter->filter = requirement_filter;
		return xmlListAppend (case_filter_list, filter);
	}
	else if (!strcasecmp ((char *)filter->key, "type")) {
		filter->filter = type_filter;
		return xmlListAppend (case_filter_list, filter);
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
		filter->filter = test_set_filter;
		return xmlListAppend (set_filter_list, filter);
	}
	else if (!strcasecmp ((char *)filter->key, "testcase")) {
		filter->filter = test_case_filter;
		return xmlListAppend (case_filter_list, filter);
	} else {
		LOG_MSG (LOG_ERR, "Unknow filter type %s",
			 filter->key);
		free (filter->key);
		xmlListDelete (filter->value_list);
		free (filter);
		return 1;
	}

	return 0;
}
/* ------------------------------------------------------------------------- */
/** Create list of values in a string of form 'value,"val ue",value'
 *  @param str string containing the values separeted by ','
 *  @return list on success, NULL on failure
 */
LOCAL xmlListPtr string2valuelist (char *str)
{
	char *p;
	xmlChar *val, *clean_val;
	xmlListPtr list = xmlListCreate (filter_value_delete, 
					 filter_value_list_compare);
	if (!list) {
		LOG_MSG (LOG_ERR, "OOM");
		return NULL;
	}
	/* Go through value list */
	p = strtok (str, ",");
	if (!p) {
		LOG_MSG (LOG_ERR, "empty value");
		return NULL;
	}
	do {
		/* Clean possible "-signs */
		if (p[0] == '"') {
			if (strlen (p) < 3) {
				LOG_MSG (LOG_ERR, "empty value");
				goto err_out;
			}
			if (p[strlen(p)-1] != '"') {
				LOG_MSG (LOG_ERR, "Mismatched \" %s", p);
				goto err_out;
			}
			val = xmlCharStrndup(&p[1], strlen(p)-2);
		} else
			val = xmlCharStrdup (p);
		/* trim leading and trailing white spaces */
		clean_val = xmlStrdup (val);
		trim_string ((char *)val, (char *)clean_val);
		free (val);
		xmlListAppend (list, clean_val);
	} while ((p = strtok (NULL, ",")));
	
	return list;
 err_out:
	if (list) xmlListDelete (list);
	return NULL;
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
	xmlChar *k = NULL;
	test_filter *filter = NULL;

	/* handle possible +/- in beginning of key */
	if (*key == '-') {
		exclude = 1;
		k = xmlCharStrdup (key + 1);
	} else if (*key == '+')
		k = xmlCharStrdup (key + 1);
	else
		k = xmlCharStrdup (key);

	filter = (test_filter *)malloc (sizeof (test_filter));
	filter->exclude = exclude;
	filter->key = k;
	filter->value_list = string2valuelist (values);
	if (!filter->value_list) {
		retval = 1;
		goto out;
	}
	LOG_MSG (LOG_DEBUG, "FILTER: key=%s exclude=%d",
		 filter->key, filter->exclude);
	LOG_MSG (LOG_DEBUG, "values:");
	xmlListWalk (filter->value_list, filter_value_print, NULL);
	
	retval = filter_add (filter);
 out:
	return retval;
}
/* ------------------------------------------------------------------------- */
/** Execute filters in the list
 *  @param data test filter
 *  @param user test suite, test set or test case
 *  @return 0 if data is to be filtered
 */
LOCAL int filter_exec (const void *data, const void *user) 
{
	test_filter *filter = (test_filter *)data;

	return !(filter->filter(filter, user));
}
/* ------------------------------------------------------------------------- */
/** Execute filters in the list
 *  @param data test filter value
 *  @param user not used
 *  @return 1 always
 */
LOCAL int filter_value_print (const void *data, const void *user) 
{
	xmlChar *val = BAD_CAST data;

	LOG_MSG (LOG_DEBUG, "%s", val);

	return 1;
}
/* ------------------------------------------------------------------------- */
/** Filter based on manual flag
 *  @param data test case data
 *  @param filter filter used 
 *  @return 0 on when data passes the filter, 1 if the data is to be filtered
 */
LOCAL int manual_filter (test_filter *filter, const void *data)
{
	//td_case *c = (td_case *)data;
	/* FIXME: implement */
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Filter based on test case name
 *  @param data test case data
 *  @param filter filter used 
 *  @return 0 on when data passes the filter, != 0 if the data is to be filtered
 */
LOCAL int test_case_filter (test_filter *filter, const void *data)
{
	int found = 0;
	td_case *c = (td_case *)data;
	
	if (xmlListSearch (filter->value_list, c->gen.name))
		found = 1;

	c->filtered = filter->exclude ? found : !found;

	return c->filtered;
}
/* ------------------------------------------------------------------------- */
/** Filter based on test set name
 *  @param data test case data
 *  @param filter filter used 
 *  @return 0 on when data passes the filter, 1 if the data is to be filtered
 */
LOCAL int test_set_filter (test_filter *filter, const void *data)
{
	int found = 0;
	td_set *s = (td_set *)data;
	
	if (xmlListSearch (filter->value_list, s->gen.name))
		found = 1;

	s->filtered = filter->exclude ? found : !found;

	return s->filtered;
}
/* ------------------------------------------------------------------------- */
/** Filter based on feature
 *  @param data test set data
 *  @param filter filter used 
 *  @return 0 on when data passes the filter, 1 if the data is to be filtered
 */
LOCAL int feature_filter (test_filter *filter, const void *data)
{
	int found = 0;
	td_set *s = (td_set *)data;
	xmlListPtr fea_list;
	xmlLinkPtr lk;
	xmlChar *fea, *feas;
	
	if (!s->gen.feature)
		goto skip;
	feas = xmlStrdup (s->gen.feature);
	fea_list = string2valuelist ((char *)feas);
	while (xmlListSize (fea_list) > 0) {
		lk = xmlListFront (fea_list);
		fea = xmlLinkGetData (lk);
		if (xmlListSearch (filter->value_list, fea))
			found = 1;
		xmlListPopFront (fea_list);
	}
	free (feas);
	xmlListDelete (fea_list);
 skip:
	s->filtered = filter->exclude ? found : !found;

	return s->filtered;
}
/* ------------------------------------------------------------------------- */
/** Filter based on type
 *  @param data test case data
 *  @param filter filter used 
 *  @return 0 on when data passes the filter, 1 if the data is to be filtered
 */
LOCAL int type_filter (test_filter *filter, const void *data)
{
	int found = 0;
	td_case *c = (td_case *)data;
	
	if (c->gen.type && xmlListSearch (filter->value_list, c->gen.type))
		found = 1;

	c->filtered = filter->exclude ? found : !found;

	return c->filtered;
}
/* ------------------------------------------------------------------------- */
/** Filter based on requirements
 *  @param data test case data
 *  @param filter filter used 
 *  @return 0 on when data passes the filter, 1 if the data is to be filtered
 */
LOCAL int requirement_filter (test_filter *filter, const void *data)
{
	int found = 0;
	td_case *c = (td_case *)data;
	xmlListPtr req_list;
	xmlLinkPtr lk;
	xmlChar *req, *reqs;
	
	if (!c->gen.requirement)
		goto skip;
	reqs = xmlStrdup (c->gen.requirement);
	req_list = string2valuelist ((char *)reqs);
	while (xmlListSize (req_list) > 0) {
		lk = xmlListFront (req_list);
		req = xmlLinkGetData (lk);
		if (xmlListSearch (filter->value_list, req))
			found = 1;
		xmlListPopFront (req_list);
	}
	free (reqs);
	xmlListDelete (req_list);
 skip:
	c->filtered = filter->exclude ? found : !found;

	return c->filtered;
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
	LOG_MSG (LOG_DEBUG, "string to parse %s", filters);
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
			LOG_MSG (LOG_ERR, "failed to parse filter string - "
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
			LOG_MSG (LOG_ERR, 
				 "failed to parse filter string - "
				 "mismatched \"");
			goto err_out;
		}
		retval = validate_and_add_filter (key, value);
		if (retval) {
		        goto err_out;
		}
	}
	free (f);
	return retval;
 err_out:
	if (f) free (f);
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Do filtering on suite level
 *  @param s suite to be checked against filters
 *  @return != 0 if the suite is filtered, 0 if not.
 */
int filter_suite (td_suite *s)
{
	xmlListWalk (suite_filter_list, filter_exec, s);
	return s->filtered;
}
/* ------------------------------------------------------------------------- */
/** Do filtering on set
 *  @param s set to be checked against filters
 *  @return != 0 if the set is filtered, 0 if not.
 */
int filter_set (td_set *s)
{
	xmlListWalk (set_filter_list, filter_exec, s);
	return s->filtered;
}
/* ------------------------------------------------------------------------- */
/** Do filtering on case
 *  @param c case to be checked against filters
 *  @return != 0 if the case is filtered, 0 if not.
 */
int filter_case (td_case *c)
{
	xmlListWalk (case_filter_list, filter_exec, c);
	return c->filtered;
}
/* ------------------------------------------------------------------------- */
/** Free the memory allocated for filters
 */
void cleanup_filters ()
{
	if (suite_filter_list) xmlListDelete (suite_filter_list);
	if (set_filter_list) xmlListDelete (set_filter_list);
	if (case_filter_list) xmlListDelete (case_filter_list);
}
/* ------------------------------------------------------------------------- */

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
