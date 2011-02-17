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
#include "testdefinitionparser.h"
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
LOCAL td_parser_callbacks *cbs;
LOCAL xmlTextReaderPtr reader;
LOCAL xmlSchemaParserCtxtPtr schema_context = NULL;
LOCAL xmlSchemaPtr schema = NULL;
LOCAL td_td *current_td;
LOCAL td_suite *current_suite;
LOCAL td_set *current_set;
LOCAL int parsing_level = 0;
/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_gen_attribs (td_gen_attribs *,td_gen_attribs *);
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_td (void);
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_suite (void);
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_steps (xmlListPtr, const char *);
/* ------------------------------------------------------------------------- */
LOCAL td_step *td_parse_step (int manual_default);
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_case (td_set *s);
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_environments(xmlListPtr);
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_gets(xmlListPtr);
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_set ();
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_hwiddetect ();
/* ------------------------------------------------------------------------- */
#ifdef ENABLE_EVENTS
LOCAL td_step *td_parse_event();
/* ------------------------------------------------------------------------- */
LOCAL td_event_param *td_parse_event_param();
/* ------------------------------------------------------------------------- */
#endif
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_gen_attribs (td_gen_attribs *attr,
				td_gen_attribs *defaults)
{
	const xmlChar *name;

	if (defaults) {
		attr->timeout = defaults->timeout;
		attr->manual  = defaults->manual;
		attr->insignificant = defaults->insignificant;
		if (defaults->requirement)
			attr->requirement = xmlStrdup(defaults->requirement);
		if (defaults->level)
			attr->level = xmlStrdup(defaults->level);
		if (defaults->type)
			attr->type = xmlStrdup(defaults->type);
		if (defaults->hwid)
			attr->hwid = xmlStrdup(defaults->hwid);
		if (defaults->component)
			attr->component = xmlStrdup(defaults->component);
		if (defaults->feature)
		        attr->feature = xmlStrdup(defaults->feature);
		if (defaults->domain)
		        attr->domain = xmlStrdup(defaults->domain);
	}

	while (xmlTextReaderMoveToNextAttribute(reader)) {
		name = xmlTextReaderConstName(reader);
		if (!xmlStrcmp (name, BAD_CAST "name")) {
			attr->name = xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "timeout")) {
			attr->timeout = strtoul((char *)
						xmlTextReaderConstValue(reader),
						NULL, 10);
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "description")) {
			attr->description =  xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "requirement")) {
			if (attr->requirement) free (attr->requirement);
			attr->requirement =  xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "type")) {
			if (attr->type) free (attr->type);
			attr->type =  xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "level")) {
			if (attr->level) free (attr->level);
			attr->level =  xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "domain")) {
			if (attr->domain) free (attr->domain);
			attr->domain =  xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "feature")) {
			if (attr->feature) free (attr->feature);
			attr->feature =  xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "component")) {
			if (attr->component) free (attr->component);
			attr->component =  xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "manual")) {
			attr->manual = !xmlStrcmp (xmlTextReaderConstValue
						   (reader), BAD_CAST "true");
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "insignificant")) {
			attr->insignificant = 
				!xmlStrcmp (xmlTextReaderConstValue (reader), 
					    BAD_CAST "true");
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "hwid")) {
			if (attr->hwid)
				free (attr->hwid);
			attr->hwid =  xmlTextReaderValue(reader);
			continue;
		}
	}
	xmlTextReaderMoveToElement (reader);

	return 0;
}
/* ------------------------------------------------------------------------- */
/** Parse one step  
 *  @return *td_step on success, NULL on error
 */
LOCAL td_step *td_parse_step(int manual_default)
{
	const xmlChar *name;
	td_step *step = NULL;
	xmlNodePtr node;
	int ret;

	step = td_step_create();
	step->manual = manual_default;

	while (xmlTextReaderMoveToNextAttribute(reader)) {
		name = xmlTextReaderConstName(reader);
		if (!xmlStrcmp (name, BAD_CAST "expected_result")) {
			step->expected_result = 
				strtoul((char *)xmlTextReaderConstValue(reader),
					NULL, 10);
			step->has_expected_result = 1;
			
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "manual")) {
			step->manual = !xmlStrcmp (xmlTextReaderConstValue
						   (reader), BAD_CAST "true");
			continue;
		}
	}
	
	do {
		ret = xmlTextReaderRead(reader);
		if (!ret) {
			LOG_MSG (LOG_ERR, "%s:%s: ReaderRead() fail\n",
				 PROGNAME, __FUNCTION__);
			
			goto ERROUT;
		}
		name = xmlTextReaderConstName(reader);
		if (!name) {
			LOG_MSG (LOG_ERR, "%s: ReaderName() fail\n",
				 PROGNAME);
			goto ERROUT;
		}

		if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT) {
			if (step->step)
				step->step = xmlStrcat 
					(step->step, 
					 xmlTextReaderReadString (reader));
			else
				step->step = xmlTextReaderReadString (reader);
		}

		if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_CDATA) {
			node = xmlTextReaderCurrentNode (reader);
			if (step->step)
				step->step = xmlStrcat 
					(step->step, 
					 node->content);
			else
				step->step = xmlStrdup(node->content);
		
		}

	} while  (!(xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_END_ELEMENT &&
		    !xmlStrcmp (name, BAD_CAST "step")));

	return step;
 ERROUT:
	LOG_MSG (LOG_ERR, "%s:%s: Exiting with error\n", 
		 PROGNAME, __FUNCTION__);
	free (step);
	
	return NULL;
}
/* ------------------------------------------------------------------------- */
/** Parse step elements ([pre|post]_step and save them in list 
 *  @param list list into which the steps are to be inserted
 *  @param tag element name
 *  @return 0 on success, 1 on error
 */
LOCAL int td_parse_steps(xmlListPtr list, const char *tag)
{
	const xmlChar *name;
	td_steps *steps = NULL;
	td_step *step = NULL;
	int ret;

	steps = td_steps_create();

	if (!steps) {
		goto ERROUT;
	}

	if (xmlTextReaderMoveToAttribute (reader, BAD_CAST "timeout") == 1) {
		steps->timeout = strtoul((char *)
					 xmlTextReaderConstValue(reader),
					 NULL, 10);
	}

	do {
		ret = xmlTextReaderRead(reader);
		if (!ret) {
			LOG_MSG (LOG_ERR, "%s: ReaderRead() fail\n",
				 PROGNAME);
			goto ERROUT;
		}

		name = xmlTextReaderConstName(reader);
		if (!name) {
			LOG_MSG (LOG_ERR, "%s: ReaderName() fail\n",
				 PROGNAME);
			goto ERROUT;
		}

		if (xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_ELEMENT && 
		    !xmlStrcmp (name, BAD_CAST "step")) {
			step = td_parse_step (current_set->gen.manual);
			if (!step)
				goto ERROUT;
			if (xmlListAppend (steps->steps, step)) {
				LOG_MSG (LOG_ERR, "%s: list insert failed\n",
					 PROGNAME);
				goto ERROUT;
			}
		}
	} while  (!(xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_END_ELEMENT &&
		    !xmlStrcmp (name, BAD_CAST tag)));
	
	xmlListAppend (list, steps);

	return 0;
 ERROUT:
	LOG_MSG (LOG_ERR, "%s:%s: Exiting with error\n", 
		 PROGNAME, __FUNCTION__);
	xmlListDelete (steps->steps);
	free(steps);
	return 1;
}
/* ------------------------------------------------------------------------- */
#ifdef ENABLE_EVENTS
/** Parse event
 *  @return td_step* on success, NULL on error
 */
LOCAL td_step *td_parse_event()
{
	const xmlChar *name;
	const xmlChar *value;
	td_step *step = NULL;
	td_event *event = NULL;
	td_event_param *param = NULL;
	int ret;

	step = td_step_create();
	event = td_event_create();
	step->event = event;

	/* parse attributes */
	while (xmlTextReaderMoveToNextAttribute(reader)) {
		name = xmlTextReaderConstName(reader);

		if (!xmlStrcmp (name, BAD_CAST "type")) {
			value = xmlTextReaderConstValue(reader);
			if (!xmlStrcmp(value, BAD_CAST "send")) {
				event->type = EVENT_TYPE_SEND;
			}
			else if (!xmlStrcmp(value, BAD_CAST "wait")) {
				event->type = EVENT_TYPE_WAIT;
			} else {
				/* TODO: error */
				event->type = EVENT_TYPE_UNKNOWN;
			}
			continue;
		}

		if (!xmlStrcmp (name, BAD_CAST "resource")) {
			event->resource = xmlTextReaderValue(reader);
			continue;
		}

		if (!xmlStrcmp (name, BAD_CAST "timeout")) {
			event->timeout = 
				strtoul((char *)xmlTextReaderConstValue(reader),
					NULL, 10);
			continue;
		}
	}

	xmlTextReaderMoveToElement (reader);
	if (xmlTextReaderIsEmptyElement (reader))
		goto OK_OUT;

	/* possible param elements */
	do {
		ret = xmlTextReaderRead(reader);
		if (!ret) {
			LOG_MSG (LOG_ERR, "%s:%s: ReaderRead() fail\n",
				 PROGNAME, __FUNCTION__);
			
			goto ERROUT;
		}
		name = xmlTextReaderConstName(reader);
		if (!name) {
			LOG_MSG (LOG_ERR, "%s: ReaderName() fail\n",
				 PROGNAME);
			goto ERROUT;
		}

		if (xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_ELEMENT && 
		    !xmlStrcmp (name, BAD_CAST "param")) {
			param = td_parse_event_param ();
			if (!param)
				goto ERROUT;
			if (xmlListAppend (event->params, param)) {
				LOG_MSG (LOG_ERR, "%s: list insert failed\n",
					 PROGNAME);
				goto ERROUT;
			}
		}

	} while  (!(xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_END_ELEMENT &&
		    !xmlStrcmp (name, BAD_CAST "event")));
	
 OK_OUT:
	return step;

 ERROUT:
	td_event_delete (event);
	free (step);
	return NULL;
}
/* ------------------------------------------------------------------------- */
/** Parse event_param
 *  @return td_event_param* on success, NULL on error
 */
LOCAL td_event_param *td_parse_event_param()
{
	const xmlChar *name;
	td_event_param *param = NULL;
	int ret;

	param = td_event_param_create();

	/* parse attributes */
	while (xmlTextReaderMoveToNextAttribute(reader)) {
		name = xmlTextReaderConstName(reader);

		if (!xmlStrcmp (name, BAD_CAST "type")) {
			param->type = xmlTextReaderValue(reader);
			continue;
		}

		if (!xmlStrcmp (name, BAD_CAST "name")) {
			param->name = xmlTextReaderValue(reader);
			continue;
		}
	}

	xmlTextReaderMoveToElement (reader);
	if (xmlTextReaderIsEmptyElement (reader))
		goto OK_OUT;

	do {
		ret = xmlTextReaderRead(reader);
		if (!ret) {
			LOG_MSG (LOG_ERR, "%s:%s: ReaderRead() fail\n",
				 PROGNAME, __FUNCTION__);
			
			goto ERROUT;
		}
		name = xmlTextReaderConstName(reader);
		if (!name) {
			LOG_MSG (LOG_ERR, "%s: ReaderName() fail\n",
				 PROGNAME);
			goto ERROUT;
		}

		if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT) {
			param->value = xmlTextReaderReadString (reader);
		}

	} while  (!(xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_END_ELEMENT &&
		    !xmlStrcmp (name, BAD_CAST "param")));

 OK_OUT:
	return param;

 ERROUT:
	if (param->type) xmlFree(param->type);
	if (param->name) xmlFree(param->name);
	if (param->value) xmlFree(param->value);
	return NULL;
}
/* ------------------------------------------------------------------------- */
#endif	/* ENABLE_EVENTS */
/** Parse test case and insert to set list of cases.
 *  @param *s td_set structure
 *  @return 0 on success, 1 on error 
 */
LOCAL int td_parse_case(td_set *s)
{
	const xmlChar *name;
	td_step *step = NULL;
	td_case *c = NULL;
	int ret;

	c = td_case_create();
	if (!c)
		goto ERROUT;

	if (td_parse_gen_attribs (&c->gen, &s->gen))
		goto ERROUT;

	if (xmlTextReaderMoveToAttribute (reader, 
					  BAD_CAST "subfeature") == 1) {
		c->subfeature = xmlTextReaderValue(reader);
	}

	if (xmlTextReaderMoveToAttribute (reader, 
					  BAD_CAST "bugzilla_id") == 1) {
		c->bugzilla_id = xmlTextReaderValue(reader);
	}
	if (xmlTextReaderMoveToAttribute (reader, 
					  BAD_CAST "TC_ID") == 1) {
		c->tc_id = xmlTextReaderValue(reader);
	}
	if (xmlTextReaderMoveToAttribute (reader, 
					  BAD_CAST "state") == 1) {
		c->state = xmlTextReaderValue(reader);
	}

	xmlTextReaderMoveToElement (reader);
	if (xmlTextReaderIsEmptyElement (reader))
		goto OK_OUT;

	do {
		ret = xmlTextReaderRead(reader);
		if (!ret) { 
			goto ERROUT;
		}
		name = xmlTextReaderConstName(reader);
		if (!name) {
			LOG_MSG (LOG_ERR, "%s: ReaderName() fail\n",
				 PROGNAME);
			goto ERROUT;
		}
		if (xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_ELEMENT && 
		    !xmlStrcmp (name, BAD_CAST "description")) {
		    c->description = xmlTextReaderReadString(reader);
		    
		}

		if (xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_ELEMENT && 
		    !xmlStrcmp (name, BAD_CAST "step")) {
		    step = td_parse_step (c->gen.manual);
		    if (!step)
			    goto ERROUT;
		    if (xmlListAppend (c->steps, step)) {
			    LOG_MSG (LOG_ERR, "%s: list insert failed\n",
				     PROGNAME);
			    goto ERROUT;
		    }
		}

#ifdef ENABLE_EVENTS
		if (xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_ELEMENT && 
		    !xmlStrcmp (name, BAD_CAST "event")) {
		    /* event is special case of step */
		    step = td_parse_event();
		    if (!step)
			    goto ERROUT;
		    if (xmlListAppend (c->steps, step)) {
			    LOG_MSG (LOG_ERR, "%s: list insert failed\n",
				     PROGNAME);
			    goto ERROUT;
		    }
		}
#endif

		if (!xmlStrcmp (name, BAD_CAST "get"))
			if (td_parse_gets(c->gets))
				goto ERROUT;
	    
	} while  (!(xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_END_ELEMENT &&
		    !xmlStrcmp (name, BAD_CAST "case")));
 OK_OUT:	
	xmlListAppend (s->cases, c);
	
	return 0;
 ERROUT:
	LOG_MSG (LOG_ERR, "%s:%s: Exiting with error\n", 
		 PROGNAME, __FUNCTION__);
	xmlListDelete (c->steps);
	if (c->gen.name) free (c->gen.name);
	if (c->gen.description) free (c->gen.description);
	free (c);
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Parse set environments and save them in list
 *  @param list used for saving the enabled environments
 *  @return 0 on success, 1 on error
 */
LOCAL int td_parse_environments(xmlListPtr list)
{
	int ret;
	const xmlChar *name;
	xmlChar *value;

	do {
		ret = xmlTextReaderRead(reader);
		if (!ret) {
			LOG_MSG (LOG_ERR, "%s:%s: ReaderRead() fail\n",
				 PROGNAME, __FUNCTION__);
			
			goto ERROUT;
		}
		/* Environment name (scratchbox, hardware) */
		if (xmlTextReaderNodeType(reader) ==  XML_READER_TYPE_ELEMENT) {
			name = xmlTextReaderConstName(reader);
			if (!name) {
				LOG_MSG (LOG_ERR, "%s:%s: ReaderName() "
					 "fail\n",
					 PROGNAME, __FUNCTION__);
				goto ERROUT;
			}
		}
		/* remove from list of environments if "false" */
		if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT) {
			value = xmlTextReaderReadString (reader);
			if (!xmlStrcmp (value, BAD_CAST "false")) {
				xmlListRemoveAll (list, (void *)name);
			}
			free (value);
		}
	} while (!(xmlTextReaderNodeType(reader) == 
		   XML_READER_TYPE_END_ELEMENT &&
		   !xmlStrcmp (xmlTextReaderConstName(reader), 
			       BAD_CAST "environments")));
	
	return 0;
 ERROUT:
	LOG_MSG (LOG_ERR, "%s:%s: Exiting with error\n", 
		 PROGNAME, __FUNCTION__);
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Parse get-element, add filenames to list
 *  @param list used for saving "get" filenames
 *  @return 0 on success, 1 on error
 */
LOCAL int td_parse_gets(xmlListPtr list)
{
	int ret;
	int delete_after = 0, measurement = 0;
	td_file *file;

	do {

		ret = xmlTextReaderRead(reader);
		if (!ret) {
			LOG_MSG (LOG_ERR, "%s:%s: ReaderRead() fail\n",
				 PROGNAME, __FUNCTION__);
			
			goto ERROUT;
		}

		if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_ELEMENT) {
			if (xmlTextReaderMoveToAttribute 
			    (reader, 
			     BAD_CAST "delete_after") == 1) {
				delete_after = !xmlStrcmp (
					xmlTextReaderConstValue
					(reader), 
					BAD_CAST "true");
			}
			
			if (xmlTextReaderMoveToAttribute 
			    (reader, 
			     BAD_CAST "measurement") == 1) {
				 measurement = !xmlStrcmp (
					 xmlTextReaderConstValue
					 (reader), 
					 BAD_CAST "true");
			}
		} 

		/* add to list get files */
		if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT) {
			file = (td_file *)malloc (sizeof (td_file));
			file->filename = xmlTextReaderReadString (reader);
			file->delete_after = delete_after;
			file->measurement = measurement;
			delete_after = 0;
			if (xmlListAppend (list, file)) {
				LOG_MSG (LOG_ERR, 
					 "%s:%s list insert failed\n",
					 PROGNAME, __FUNCTION__);
				goto ERROUT;
			}
			
		}
	} while (!(xmlTextReaderNodeType(reader) == 
		   XML_READER_TYPE_END_ELEMENT &&
		   !xmlStrcmp (xmlTextReaderConstName(reader), 
			       BAD_CAST "get")));
	
	return 0;
 ERROUT:
	LOG_MSG (LOG_ERR, "%s:%s: Exiting with error\n", 
		 PROGNAME, __FUNCTION__);
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Read test suite in to td_suite data structure and call pass it to callback
 *  @return 0 on success
 */
LOCAL int td_parse_suite ()
{
	td_suite *s;


	if (!cbs->test_suite)
		return 1;
	
	s = td_suite_create();
	if (!s) return 1;
	
	current_suite = s;

	td_parse_gen_attribs (&s->gen, NULL);

	cbs->test_suite(s);

	if (xmlTextReaderIsEmptyElement (reader)) {
	        parsing_level--;
	        cbs->test_suite_end(s);
	}
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Read test set in to td_set data structure and call pass it to callback
 *  @return 0 on success
 */
LOCAL int td_parse_set ()
{
	int ret = 0;
	td_set *s;
	const xmlChar *name;
	xmlReaderTypes type;


	if (!cbs->test_set)
		return 1;
	s = td_set_create ();
	current_set = s;

	if (td_parse_gen_attribs(&s->gen, &current_suite->gen))
		goto ERROUT;

	if (xmlTextReaderIsEmptyElement (reader))
		goto OKOUT;

	do {
		ret = xmlTextReaderRead(reader);
		if (!ret) 
			goto OKOUT;
		name = xmlTextReaderConstName(reader);
		type = xmlTextReaderNodeType(reader);
		if (!name) {
			LOG_MSG (LOG_ERR, "%s: ReaderName() fail\n",
				 PROGNAME);
			goto ERROUT;
		}

		if (!xmlStrcmp (name, BAD_CAST "pre_steps"))
			ret = !td_parse_steps(s->pre_steps, "pre_steps");
		if (!xmlStrcmp (name, BAD_CAST "post_steps"))
			ret = !td_parse_steps(s->post_steps, "post_steps");
		if (!xmlStrcmp (name, BAD_CAST "case"))
			ret = !td_parse_case(s);
		if (!xmlStrcmp (name, BAD_CAST "environments"))
		    ret = !td_parse_environments(s->environments);
		if (!xmlStrcmp (name, BAD_CAST "get"))
		    ret = !td_parse_gets(s->gets);
		if (!xmlStrcmp (name, BAD_CAST "description") &&
		    type == XML_READER_TYPE_ELEMENT) {
			s->description = 
				xmlTextReaderReadString (reader);
		}
		if (!ret)
			goto ERROUT;
	} while (!(xmlTextReaderNodeType(reader) == 
		   XML_READER_TYPE_END_ELEMENT &&
		   !xmlStrcmp (name, BAD_CAST "set")));
 OKOUT:
	cbs->test_set(s);

	return 0;
 ERROUT:
	LOG_MSG (LOG_ERR, "%s:%s: Exiting with error\n", 
		 PROGNAME, __FUNCTION__);
	
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Create test definition data type and call callback function
 *  @return 0 on success
 */
LOCAL int td_parse_td ()
{
	td_td *td;
	const xmlChar *name;

	td = td_td_create();
	if (!td)
		return 1;

	parsing_level++;
	while (xmlTextReaderMoveToNextAttribute(reader)) {
		name = xmlTextReaderConstName(reader);
 		if (!xmlStrcmp (name, BAD_CAST "version")) {
			td->version = xmlTextReaderValue(reader);
			continue;
		}
	}
	current_td = td;

	if (cbs->test_td)
		cbs->test_td(td);

	return 0;
}
/* ------------------------------------------------------------------------- */
/** Read hwiddetect tag, store detector command, and use callback to run it
 *  @return 0 on success
 */
LOCAL int td_parse_hwiddetect ()
{
	int ret;
	const xmlChar *name;

	do {
		if (xmlTextReaderHasValue(reader) > 0) {
			current_td->hw_detector = xmlTextReaderValue(reader);
			LOG_MSG (LOG_INFO, "HW ID dectector command: %s",
				 current_td->hw_detector);
		}
		ret = xmlTextReaderRead(reader);
		name = xmlTextReaderConstName(reader);
	}
	while(xmlTextReaderNodeType(reader) != XML_READER_TYPE_END_ELEMENT &&
	      xmlStrcmp (name, BAD_CAST "hwiddetect"));

	/* callback to perfrom hw detection */
	if (cbs->test_hwiddetect) {
		cbs->test_hwiddetect();
	}

	return 0;
}

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** parse testdefinition xml file and validate it agains testdefinition schema
 *  @param opts testrunner-lite options given by user
 *  @return 0 if validation is succesfull
 */
int parse_test_definition (testrunner_lite_options *opts)
{
	int ret = TESTRUNNER_LITE_XML_PARSE_FAIL;
	xmlDocPtr doc = NULL; 
	xmlParserCtxtPtr ctxt = NULL; 
	xmlSchemaPtr sch = NULL;
	xmlSchemaValidCtxtPtr valid_ctxt = NULL;
	xmlSchemaParserCtxtPtr schema_ctxt = NULL;
	    
        xmlSubstituteEntitiesDefault(1);

	/*
	 * 1) Create basic parser context and validate it.
	 */
	ctxt = xmlNewParserCtxt();
	if (ctxt == NULL) {
		LOG_MSG (LOG_ERR, "%s: Failed to allocate parser context\n",
			 PROGNAME);
		goto out;
	}
	
	doc = xmlCtxtReadFile(ctxt, opts->input_filename, NULL, XML_PARSE_NOENT);
	if (doc == NULL) {
		LOG_MSG (LOG_ERR, "%s: Failed to parse %s\n", PROGNAME,
			opts->input_filename);
		goto out;
	} else if (!ctxt->valid) {
		LOG_MSG (LOG_ERR, "%s: Failed to validate %s\n", PROGNAME, 
			opts->input_filename);
		xmlFreeDoc(doc);
		doc = NULL;
		goto out;
	}
	
	if (opts->disable_schema) {
		ret = 0;
		goto out;
	}
	/*
	 * 2) Create schema context from test defintion and validate against i
	 */
	if (opts->semantic_schema)
	    schema_ctxt = xmlSchemaNewParserCtxt("/usr/share/test-definition/"
						 "testdefinition-tm_terms.xsd");
	else
	    schema_ctxt = xmlSchemaNewParserCtxt("/usr/share/test-definition/"
						 "testdefinition-syntax.xsd");
	    
	if (schema_ctxt == NULL) {
		LOG_MSG (LOG_ERR, "%s: Failed to allocate schema context\n",
			 PROGNAME);
		goto out;
	}

	sch = xmlSchemaParse(schema_ctxt);
	if (sch == NULL) {
		LOG_MSG (LOG_ERR, "%s: Failed to parse schema\n",
			 PROGNAME);
		goto out;
	}

	valid_ctxt = xmlSchemaNewValidCtxt(sch);
	if (valid_ctxt == NULL) {
		LOG_MSG (LOG_ERR, "%s: Failed to create schema validation "
			 "context\n", PROGNAME);
		goto out;
		
	}
	
	ret = xmlSchemaValidateDoc(valid_ctxt, doc);
	if (ret)
		ret = TESTRUNNER_LITE_XML_VALIDATION_FAIL;
out:
	/* 
	 * 3) Clean up
	 */
	if (doc) xmlFreeDoc(doc);
	if (ctxt) xmlFreeParserCtxt(ctxt);
	if (sch) xmlSchemaFree(sch);
	if (valid_ctxt) xmlSchemaFreeValidCtxt(valid_ctxt);
	if (schema_ctxt) xmlSchemaFreeParserCtxt(schema_ctxt);
	
	return ret;
}
/* ------------------------------------------------------------------------- */
/** Initialize the xml reader instance
 *  @param opts testrunner-lite options given by user
 *  @return 0 on success
 */
int td_reader_init (testrunner_lite_options *opts)
{

	reader =  xmlNewTextReaderFilename(opts->input_filename);
	if (!reader) {
		LOG_MSG (LOG_ERR, "%s: failed to create xml reader for %s\n", 
			 PROGNAME, opts->input_filename);
		
	}

	if (opts->disable_schema)
		return 0;
	
	if (opts->semantic_schema)
		schema_context = xmlSchemaNewParserCtxt
			("/usr/share/test-definition/"
			 "testdefinition-tm_terms.xsd");
	else
		schema_context = xmlSchemaNewParserCtxt
			("/usr/share/test-definition/"
			 "testdefinition-syntax.xsd");
	if (schema_context == NULL) {
		LOG_MSG (LOG_ERR, "%s: Failed to allocate schema context\n",
			 PROGNAME);
		goto err_out;
	}

	schema = xmlSchemaParse(schema_context);
	if (schema == NULL) {
		LOG_MSG (LOG_ERR, "%s: Failed to parse schema\n",
			 PROGNAME);
		goto err_out;
	}

	if (xmlTextReaderSetSchema (reader, schema)) {
		LOG_MSG (LOG_ERR, "%s: Failed to set schema for xml reader\n",
			 PROGNAME);
		goto err_out;
	}
	
	
	return 0;
 err_out:
	LOG_MSG (LOG_ERR, "%s:%s: Exiting with error\n", 
             PROGNAME, __FUNCTION__);
	td_reader_close ();
	return 1;
		
}
/* ------------------------------------------------------------------------- */
/** De-init the reader instance
 */
void td_reader_close ()
{
	if (reader) xmlFreeTextReader (reader); 
	if (schema) xmlSchemaFree(schema);
	if (schema_context) xmlSchemaFreeParserCtxt(schema_context);
}
/* ------------------------------------------------------------------------- */
/** Process next node from XML reader instance.
 *  @return 0 on success
 */
int td_next_node (void) {
	int ret;
	const xmlChar *name = NULL;
	xmlReaderTypes type;
	
        ret = xmlTextReaderRead(reader);
	
	if (!ret)
		return !ret;

	name = xmlTextReaderConstName(reader);
	type = xmlTextReaderNodeType(reader);

	if (!name)
		return 1;

	if (!xmlStrcmp (name, BAD_CAST "testdefinition")) {
		if (type == XML_READER_TYPE_ELEMENT) {
			return td_parse_td();
		}
		else if (type == XML_READER_TYPE_END_ELEMENT) {
			parsing_level--;
			if (cbs->test_td_end)
				cbs->test_td_end();
			return 0;
		}
	}

	if (!xmlStrcmp (name, BAD_CAST "hwiddetect")) {
		if (type == XML_READER_TYPE_ELEMENT) {
			return td_parse_hwiddetect();
		}
	}

	if (!xmlStrcmp (name, BAD_CAST "description") &&
	    type == XML_READER_TYPE_ELEMENT) {
		switch (parsing_level) {
		case 1:
			current_td->description = 
				xmlTextReaderReadString (reader);
			break;
		case 2:
			current_suite->description = 
				xmlTextReaderReadString (reader); 
			break;
		default:
			LOG_MSG (LOG_ERR, "run time error, "
				 "invalid parsing level %d", parsing_level);
			return 1;
		}
		return 0;
	}

	if (!xmlStrcmp (name, BAD_CAST "suite")) {
		if (type == XML_READER_TYPE_ELEMENT) {
			parsing_level ++;
			return td_parse_suite();
		}
		else if (type == XML_READER_TYPE_END_ELEMENT) {
			parsing_level --;
			if (cbs->test_suite_end) cbs->test_suite_end();
			return 0;
		}
	}

	if (!xmlStrcmp (name, BAD_CAST "set") && 
	    type == XML_READER_TYPE_ELEMENT)
		return td_parse_set();

	return !ret;
} 
/* ------------------------------------------------------------------------- */
/** Set the callbacks for parser
 *  @return 0 (always so far)
 */
int td_register_callbacks(td_parser_callbacks *pcbs)
{
	cbs = pcbs;

	return 0;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

