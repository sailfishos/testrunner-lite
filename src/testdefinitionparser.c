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
td_parser_callbacks *cbs;
xmlTextReaderPtr reader;

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
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL int td_parse_suite (void);
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/** Read test suite in to td_suite data structure and call pass it to callback
 *  @return 0 on success
 */
LOCAL int td_parse_suite ()
{
	td_suite *s;
	xmlChar *name;

	if (!cbs->test_suite)
		return 1;
	
	s = (td_suite *)malloc (sizeof (td_suite));
	if (s == NULL) {
		fprintf (stderr, "%s: FATAL : OOM", PROGNAME);
		return 1;
	}

	memset (s, 0x0, sizeof (td_suite));

	while (xmlTextReaderMoveToNextAttribute(reader)) {
		name = xmlTextReaderName(reader);
		if (!xmlStrcmp (name, (xmlChar *)"name")) {
			s->name= xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, (xmlChar *)"domain")) {
			s->domain = xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, (xmlChar *)"type")) {
			s->suite_type = xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, (xmlChar *)"level")) {
			s->level = xmlTextReaderValue(reader);
			continue;
		}
		if (!xmlStrcmp (name, (xmlChar *)"timeout")) {
			s->timeout = xmlTextReaderValue(reader);
			continue;
		}
		fprintf (stderr, "%s :suite contains unhandled attribute %s\n",
			 PROGNAME, name);
	}
	
	cbs->test_suite(s);

	return 0;
}
/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** parse testdefinition xml file and validate it agains testdefinition schema
 *  @param opts testrunner-lite options given by user
 *  @return 0 if validation is succesfull
 */
int parse_test_definition (testrunner_lite_options *opts){
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
	schema_ctxt = xmlSchemaNewParserCtxt("testdefinition.xsd");
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
		return 1;
		
	}
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Process next node from XML reader instance.
 *  @return 0 on success
 */
int td_next_node(void) {
	int ret;
	xmlChar *name;
	
        ret = xmlTextReaderRead(reader);
	
	if (!ret)
		return !ret;

	name = xmlTextReaderName(reader);
	if (!name)
		return 1;
	
	if (!xmlStrcmp (name, (xmlChar *)"suite"))
		return td_parse_suite();

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

