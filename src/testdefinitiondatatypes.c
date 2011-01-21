/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include <string.h>

#include <libxml/xmlreader.h>
#include <libxml/xmlschemas.h>

#include "testrunnerlite.h"
#include "testdefinitiondatatypes.h"
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
LOCAL const char *case_res_str[] = {"FAIL", "PASS", "N/A"};
LOCAL const char *event_type_string[] = {"unknown", "send", "wait"};

/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL void list_string_delete (xmlLinkPtr);
/* ------------------------------------------------------------------------- */
LOCAL void gen_attribs_delete (td_gen_attribs *);
/* ------------------------------------------------------------------------- */
LOCAL int list_string_compare (const void *, const void *);
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/** Deallocator for list with xmlchar* items
 *  @param lk list item
 */
LOCAL void list_string_delete (xmlLinkPtr lk)
{
	xmlChar *string = (xmlChar *)xmlLinkGetData (lk);
	free (string);
}
/* ------------------------------------------------------------------------- */
/** Comparison function for list with xmlchar* items
 *  @param data0 string to compare
 *  @param data1 string to compare
 *  @return 0 if strings match 
 */
LOCAL int list_string_compare(const void * data0, 
			      const void * data1)
{
	
	return xmlStrcmp ((xmlChar *)data0, (xmlChar *)data1);
}
/* ------------------------------------------------------------------------- */
/** Comparison function for list without ordering
 *  @param data0 string to compare - not used
 *  @param data1 string to compare - not used
 *  @return 0 always
 */
LOCAL int list_dummy_compare(const void * data0, 
			      const void * data1)
{
	
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Deallocator for list with td_file items
 *  @param lk list item
 */
LOCAL void td_file_delete (xmlLinkPtr lk)
{
	td_file *file = (td_file *)xmlLinkGetData (lk);
	free (file->filename);
	free (file);
}
/* ------------------------------------------------------------------------- */
/** Deallocator for list with td_measurement items
 *  @param lk list item
 */
LOCAL void td_measurement_delete (xmlLinkPtr lk)
{
	td_measurement *meas = (td_measurement *)xmlLinkGetData (lk);
	free (meas->name);
	free (meas->unit);
	free (meas);
}
/* ------------------------------------------------------------------------- */
/** Deallocator for general attributes 
 *  @param gen general attributes 
 */
LOCAL void gen_attribs_delete (td_gen_attribs *gen)
{
	free (gen->name);
	free (gen->description);
	free (gen->requirement);
	free (gen->level);
	free (gen->type);
	free (gen->domain);
	free (gen->feature);
	free (gen->component);
	free (gen->hwid);
}
/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** Returns string matching the case result value
 *  @param  cr case result value
 *  @return const string matching the value
 */
const char *case_result_str (case_result_t cr)
{
	if (cr > CASE_NA || cr < CASE_FAIL)
		return "INVALID";
	return case_res_str[cr];
}
/* ------------------------------------------------------------------------- */
/** Returns string matching the event type value
 *  @param  et event type value
 *  @return const string matching the value
 */
const char *event_type_str (event_type_t et)
{
	if (et < EVENT_TYPE_UNKNOWN || et > EVENT_TYPE_WAIT)
		return "INVALID";
	return event_type_string[et];
}
/* ------------------------------------------------------------------------- */
/** Creates test definition data structure
 *  @return pointer to td_td or NULL in case of OOM
 */
td_td *td_td_create()
{
	td_td *td = (td_td *)malloc (sizeof (td_td));
	if (td == NULL) {
		LOG_MSG (LOG_ERR, "%s: FATAL : OOM", PROGNAME);
		return NULL;
	}

	memset (td, 0x0, sizeof (td_td));
	return td;
}
/* ------------------------------------------------------------------------- */
/** De-allocate test definition data structure
 *  @param *td td_td data
 */
void td_td_delete(td_td *td)
{
	if (td) {
		xmlFree(td->hw_detector);
		xmlFree(td->detected_hw);
		xmlFree(td->version);
		xmlFree(td->description);
		free (td);
	}
}
/** Creates a td_suite data structure, initializes lists for pre_steps etc.
 *  @return pointer to td_set or NULL in case of OOM
 */
td_suite *td_suite_create()
{
	td_suite *s = (td_suite *)malloc (sizeof (td_suite));
	if (s == NULL) {
		LOG_MSG (LOG_ERR, "%s: FATAL : OOM", PROGNAME);
		return NULL;
	}

	memset (s, 0x0, sizeof (td_suite));
	s->gen.timeout = DEFAULT_TIMEOUT;
	return s;
}
/* ------------------------------------------------------------------------- */
/** De-allocate td_suite data structure
 *  @param *s td_suite data 
 */
void td_suite_delete(td_suite *s)
{
	gen_attribs_delete (&s->gen);
	xmlFree (s->description);
	free (s);
}
/* ------------------------------------------------------------------------- */
/** Creates a td_set data structure, initializes lists for pre_steps etc.
 *  @return pointer to td_set or NULL in case of OOM
 */
td_set *td_set_create ()
{
	td_set *set = (td_set *)malloc (sizeof (td_set));
	xmlChar *env;
	if (set == NULL) {
		LOG_MSG (LOG_ERR, "%s: FATAL : OOM", PROGNAME);
		return NULL;
	}
	memset (set, 0x0, sizeof (td_set));
	set->pre_steps = xmlListCreate (td_steps_delete, list_dummy_compare);
	set->post_steps = xmlListCreate (td_steps_delete, list_dummy_compare);
	set->cases = xmlListCreate (td_case_delete, list_dummy_compare);
	set->environments = xmlListCreate (list_string_delete, 
					   list_string_compare);

	/*
	** Initialize environments with default values
	*/
	env =  xmlCharStrdup ("hardware");
	xmlListAppend (set->environments, env);
	env =  xmlCharStrdup ("scratchbox");
	xmlListAppend (set->environments, env);

	set->gets = xmlListCreate (td_file_delete, NULL);

	return set;
}
/* ------------------------------------------------------------------------- */
/** De-allocate td_set data structure
 *  @param *s td_set data 
 */
void td_set_delete(td_set *s)
{
	gen_attribs_delete(&s->gen);

	xmlListDelete (s->pre_steps);
	xmlListDelete (s->post_steps);
	xmlListDelete (s->cases);
	xmlListDelete (s->environments);
	xmlListDelete (s->gets);
	xmlFree (s->description);
	xmlFree (s->environment);
	free (s);
}
/* ------------------------------------------------------------------------- */
/** Creates a td_step data structure
 *  @return pointer to td_step or NULL in case of OOM
 */
td_step *td_step_create()
{
	td_step *step;

	step = (td_step *) malloc (sizeof (td_step));
	if (step == NULL) {
		LOG_MSG (LOG_ERR, "%s: FATAL : OOM", PROGNAME);
		return NULL;
	}
	memset (step, 0x0, sizeof (td_step));
	return step;
}
/* ------------------------------------------------------------------------- */
/** Creates a td_case data structure
 *  @return pointer to td_case or NULL in case of OOM
 */
td_case *td_case_create()
{
	td_case *td_c;

	td_c = (td_case *) malloc (sizeof (td_case));
	if (td_c == NULL) {
		LOG_MSG (LOG_ERR, "%s: FATAL : OOM", PROGNAME);
		return NULL;
	}
	memset (td_c, 0x0, sizeof (td_case));
	td_c->steps = xmlListCreate (td_step_delete, list_dummy_compare);
	td_c->gets = xmlListCreate (td_file_delete, NULL);
	td_c->measurements = xmlListCreate (td_measurement_delete, 
					    list_dummy_compare);

	return td_c;
}
/* ------------------------------------------------------------------------- */
/** Creates a td_steps data structure
 *  @return pointer to td_case or NULL in case of OOM
 */
td_steps *td_steps_create()
{
	td_steps *steps;

	steps = (td_steps *) malloc (sizeof (td_steps));
	if (steps == NULL) {
		LOG_MSG (LOG_ERR, "%s: FATAL : OOM", PROGNAME);
		return NULL;
	}
	memset (steps, 0x0, sizeof (td_steps));
	steps->timeout = DEFAULT_PRE_STEP_TIMEOUT; 
	steps->steps = xmlListCreate (td_step_delete, list_dummy_compare);

	return steps;
}
/* ------------------------------------------------------------------------- */
/** Deallocator for td_step called by xmlListDelete
 */
void td_step_delete(xmlLinkPtr lk)
{
	td_step *step = xmlLinkGetData (lk);
	if (step->step)
		free (step->step);

	if (step->event)
		td_event_delete(step->event);

	free (step->stdout_);
	free (step->stderr_);
	free (step->failure_info);
	
	free (step);
}
/* ------------------------------------------------------------------------- */
/** Deallocator for  td_case data structure
 */
void td_case_delete(xmlLinkPtr lk)
{
	td_case *td_c = xmlLinkGetData (lk);
	xmlListDelete (td_c->steps);
	xmlListDelete (td_c->gets);
	xmlListDelete (td_c->measurements);

	xmlFree (td_c->comment);
	xmlFree (td_c->failure_info);
	xmlFree (td_c->tc_id);
	xmlFree (td_c->state);
	xmlFree (td_c->subfeature);
	xmlFree (td_c->bugzilla_id);
	xmlFree (td_c->description);

	gen_attribs_delete(&td_c->gen);
	free (td_c);
}
/* ------------------------------------------------------------------------- */
/** Deallocator for  td_steps data structure
 */
void td_steps_delete(xmlLinkPtr lk)
{
	td_steps *steps = xmlLinkGetData (lk);
	xmlListDelete (steps->steps);
	free (steps);
}
/* ------------------------------------------------------------------------- */
/** Creates a td_event data structure
 *  @return pointer to td_event or NULL in case of OOM
 */
td_event *td_event_create()
{
	td_event *event;

	event = (td_event *) malloc (sizeof (td_event));
	if (event == NULL) {
		LOG_MSG (LOG_ERR, "%s: FATAL : OOM", PROGNAME);
		return NULL;
	}

	event->type = EVENT_TYPE_UNKNOWN;
	event->resource = NULL;
	event->timeout = DEFAULT_TIMEOUT;
	event->params = xmlListCreate (td_event_param_delete,
				       list_dummy_compare);

	return event;
}
/* ------------------------------------------------------------------------- */
/** Deallocator for td_event
 */
void td_event_delete(td_event *event)
{
	if (event->resource)
		xmlFree (event->resource);
	xmlListDelete (event->params);
	free (event);
}
/* ------------------------------------------------------------------------- */
/** Creates a td_event_param data structure
 *  @return pointer to td_event_param or NULL in case of OOM
 */
td_event_param *td_event_param_create()
{
	td_event_param *param;

	param = (td_event_param *) malloc (sizeof (td_event_param));
	if (param == NULL) {
		LOG_MSG (LOG_ERR, "%s: FATAL : OOM", PROGNAME);
		return NULL;
	}

	param->type = NULL;
	param->name = NULL;
	param->value = NULL;

	return param;
}
/* ------------------------------------------------------------------------- */
/** Deallocator for td_event_param
 */
void td_event_param_delete(xmlLinkPtr lk)
{
	td_event_param *param = xmlLinkGetData (lk);

	if (param->type)
		xmlFree (param->type);
	if (param->name)
		xmlFree (param->name);
	if (param->value)
		xmlFree (param->value);
	free (param);
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

