/* * This file is part of testrunnerlite *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 *
 * Contact: Sampo Saaristo <ext-sampo.1.saaristo@nokia.com>
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
#include <getopt.h>
#include <errno.h>
#include <string.h>

#include <libxml/xmlreader.h>
#include <libxml/xmlschemas.h>

#include "testrunnerlite.h"
#include "testdefinitiondatatypes.h"
#include "testdefinitionparser.h"

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

/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_suite (void);
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_steps (xmlListPtr, const char *);
/* ------------------------------------------------------------------------- */
LOCAL td_step *td_parse_step (void);
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_case (td_set *s);
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_environments(xmlListPtr);
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_set ();
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_gen_attribs (td_gen_attribs *attr)
{
	const xmlChar *name;

	while (xmlTextReaderMoveToNextAttribute(reader)) {
		name = xmlTextReaderConstName(reader);
		if (!xmlStrcmp (name, BAD_CAST "name")) {
			attr->name= xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "timeout")) {
			attr->timeout = strtoul(
						(char *)
						xmlTextReaderConstValue(reader),
						NULL, 10);
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "description")) {
			attr->description =  xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, BAD_CAST "requirement")) {
			attr->requirement =  xmlTextReaderValue(reader);
			continue;
		}
		
		if (!xmlStrcmp (name, BAD_CAST "level")) {
			attr->level =  xmlTextReaderValue(reader);
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

		fprintf (stderr, "%s :Unknown general attribute %s\n",
			 PROGNAME, name);

	}
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Parse one step  
 *  @return *td_step on success, NULL on error
 */
LOCAL td_step *td_parse_step()
{
	const xmlChar *name;
	td_step *step = NULL;
	xmlNodePtr node;
	int ret;

	step = td_step_create();
	do {
		ret = xmlTextReaderRead(reader);
		if (!ret) {
			fprintf (stderr, "%s:%s: ReaderRead() fail\n",
				 PROGNAME, __FUNCTION__);
			
			goto ERROUT;
		}
		name = xmlTextReaderConstName(reader);
		if (!name) {
			fprintf (stderr, "%s: ReaderName() fail\n",
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
	td_step *step = NULL;
	int ret;
	
	do {
		ret = xmlTextReaderRead(reader);
		if (!ret) {
			fprintf (stderr, "%s: ReaderRead() fail\n",
				 PROGNAME);
			goto ERROUT;
		}
		name = xmlTextReaderConstName(reader);
		if (!name) {
			fprintf (stderr, "%s: ReaderName() fail\n",
				 PROGNAME);
			goto ERROUT;
		}
		if (xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_ELEMENT && 
		    !xmlStrcmp (name, BAD_CAST "step")) {
			step = td_parse_step();
			if (!step)
				goto ERROUT;
			if (xmlListInsert (list, step)) {
				fprintf (stderr, "%s: list insert failed\n",
					 PROGNAME);
				goto ERROUT;
			}
		}
	} while  (!(xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_END_ELEMENT &&
		    !xmlStrcmp (name, BAD_CAST tag)));
	
	return 0;
 ERROUT:
	return 1;
}
/* ------------------------------------------------------------------------- */
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

	if (td_parse_gen_attribs (&c->gen))
		goto ERROUT;

	
	do {
		ret = xmlTextReaderRead(reader);
		if (!ret) {
			fprintf (stderr, "%s: ReaderRead() fail\n",
				 PROGNAME);
			
			goto ERROUT;
		}
		name = xmlTextReaderConstName(reader);
		if (!name) {
			fprintf (stderr, "%s: ReaderName() fail\n",
				 PROGNAME);
			goto ERROUT;
		}
		if (xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_ELEMENT && 
		    !xmlStrcmp (name, BAD_CAST "step")) {
		    step = td_parse_step();
		    if (!step)
			    goto ERROUT;
		    if (xmlListInsert (c->steps, step)) {
			    fprintf (stderr, "%s: list insert failed\n",
				     PROGNAME);
			    goto ERROUT;
		    }
		}
	    
	} while  (!(xmlTextReaderNodeType(reader) == 
		    XML_READER_TYPE_END_ELEMENT &&
		    !xmlStrcmp (name, BAD_CAST "case")));
	
	xmlListInsert (s->cases, c);
	
	return 0;
ERROUT:
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
	xmlChar *env;
	printf ("%s\n", __FUNCTION__);

	do {
		ret = xmlTextReaderRead(reader);
		if (!ret) {
			fprintf (stderr, "%s:%s: ReaderRead() fail\n",
				 PROGNAME, __FUNCTION__);
			
			goto ERROUT;
		}
		/* Environment name (scratchbox, hardware) */
		if (xmlTextReaderNodeType(reader) ==  XML_READER_TYPE_ELEMENT) {
			name = xmlTextReaderConstName(reader);
			if (!name) {
				fprintf (stderr, "%s: ReaderName() fail\n",
					 PROGNAME);
				goto ERROUT;
			}
		}
		/* add to list of environments if "true" */
		if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT) {
			value = xmlTextReaderReadString (reader);
			if (!xmlStrcmp (value, BAD_CAST "true")) {
				env = xmlStrdup(name);
				if (xmlListInsert (list, env)) {
					fprintf (stderr, 
						 "%s:%s list insert failed\n",
						 PROGNAME, __FUNCTION__);
					goto ERROUT;
				}
			}
			free (value);
		}
	} while (!(xmlTextReaderNodeType(reader) == 
		   XML_READER_TYPE_END_ELEMENT &&
		   !xmlStrcmp (xmlTextReaderConstName(reader), 
			       BAD_CAST "environments")));
	
	return 0;
 ERROUT:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Read test suite in to td_suite data structure and call pass it to callback
 *  @return 0 on success
 */
LOCAL int td_parse_suite ()
{
	td_suite *s;
	const xmlChar *name;

	if (!cbs->test_suite)
		return 1;
	
	s = td_suite_create();
	if (!s) return 1;
	
	//	if (!xmlStrcmp (name, BAD_CAST "domain")) {
	//s->domain = xmlTextReaderValue(reader);
	//continue;
	//}
	
	td_parse_gen_attribs (&s->gen);
	
	cbs->test_suite(s);

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

	if (!cbs->test_set)
		return 1;
	s = td_set_create ();

	if (td_parse_gen_attribs(&s->gen))
		goto ERROUT;
	do {
		ret = xmlTextReaderRead(reader);
		if (!ret) {
			fprintf (stderr, "%s:%s: ReaderRead() fail\n",
				 PROGNAME, __FUNCTION__);

			goto ERROUT;
		}
		name = xmlTextReaderConstName(reader);
		if (!name) {
			fprintf (stderr, "%s: ReaderName() fail\n",
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
		
		if (!ret)
			goto ERROUT;
	} while (!(xmlTextReaderNodeType(reader) == 
		   XML_READER_TYPE_END_ELEMENT &&
		   !xmlStrcmp (name, BAD_CAST "set")));
	
	cbs->test_set(s);

	return 0;
 ERROUT:
	fprintf (stderr, "%s: exiting with error\n",
		 __FUNCTION__);
	
	return 1;
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
	int ret = 1;
	xmlDocPtr doc = NULL; 
	xmlParserCtxtPtr ctxt = NULL; 
	xmlSchemaPtr sch = NULL;
	xmlSchemaValidCtxtPtr valid_ctxt = NULL;
	xmlSchemaParserCtxtPtr schema_ctxt = NULL;
	
	/*
	 * 1) Create basic parser context and validate it.
	 */
	ctxt = xmlNewParserCtxt();
	if (ctxt == NULL) {
		fprintf (stderr, "%s: Failed to allocate parser context\n",
			 PROGNAME);
		goto out;
	}
	
	doc = xmlCtxtReadFile(ctxt, opts->input_filename, NULL, 0);
	if (doc == NULL) {
		fprintf(stderr, "%s: Failed to parse %s\n", PROGNAME,
			opts->input_filename);
		goto out;
	} else if (!ctxt->valid) {
		fprintf(stderr, "%s: Failed to validate %s\n", PROGNAME, 
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
	schema_ctxt = xmlSchemaNewParserCtxt("/usr/share/test-definition/"
					     "testdefinition-syntax.xsd");
	if (schema_ctxt == NULL) {
		fprintf (stderr, "%s: Failed to allocate schema context\n",
			 PROGNAME);
		goto out;
	}

	sch = xmlSchemaParse(schema_ctxt);
	if (sch == NULL) {
		fprintf (stderr, "%s: Failed to parse schema\n",
			 PROGNAME);
		goto out;
	}

	valid_ctxt = xmlSchemaNewValidCtxt(sch);
	if (valid_ctxt == NULL) {
		fprintf (stderr, "%s: Failed to create schema validation "
			 "context\n", PROGNAME);
		goto out;
		
	}
	
	ret = xmlSchemaValidateDoc(valid_ctxt, doc);
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
		fprintf(stderr, "%s: failed to create xml reader for %s\n", 
			PROGNAME, opts->input_filename);
		
	}

	schema_context = xmlSchemaNewParserCtxt("/usr/share/test-definition/"
						"testdefinition-syntax.xsd");
	if (schema_context == NULL) {
		fprintf (stderr, "%s: Failed to allocate schema context\n",
			 PROGNAME);
		goto err_out;
	}

	schema = xmlSchemaParse(schema_context);
	if (schema == NULL) {
		fprintf (stderr, "%s: Failed to parse schema\n",
			 PROGNAME);
		goto err_out;
	}

	if (xmlTextReaderSetSchema (reader, schema)) {
		fprintf (stderr, "%s: Failed to set schema for xml reader\n",
			 PROGNAME);
		goto err_out;
	}
	
	
	return 0;
 err_out:
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
	const xmlChar *name;
	xmlReaderTypes type;
	
        ret = xmlTextReaderRead(reader);
	
	if (!ret)
		return !ret;

	name = xmlTextReaderConstName(reader);
	type = xmlTextReaderNodeType(reader);

	if (!name)
		return 1;
	
	if (!xmlStrcmp (name, BAD_CAST "suite")) {
		if (type == XML_READER_TYPE_ELEMENT)
			return td_parse_suite();
		else if (type == XML_READER_TYPE_END_ELEMENT) {
			cbs->test_suite_end();
			return 0;
		}
	}

	if (!xmlStrcmp (name, BAD_CAST "set") && 
	    type == XML_READER_TYPE_ELEMENT)
		return td_parse_set();
	
	fprintf (stderr, "Unhandled tag %s\n", name);
	
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

