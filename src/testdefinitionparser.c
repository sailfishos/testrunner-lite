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
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** parse testdefinition xml file and validate it agains testdefinition schema
 *  @param opts testrunner-lite options given by user
 *  @return 0 if validation is succesfull
 */
int parse_test_definition (testrunner_lite_options *opts){
	int ret = 1;
	xmlParserCtxtPtr ctxt = NULL; 
	xmlDocPtr doc = NULL; 
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

	valid_ctxt = xmlSchemaNewValidCtxt (sch);
	if (valid_ctxt == NULL) {
		fprintf (stderr, "%s: Failed to create schema validation "
			 "context\n", PROGNAME);
		goto out;
		
	}
	
	ret = xmlSchemaValidateDoc(valid_ctxt, doc);
    
	/* 
	 * 3) Clean up
	 */
out:
	if (doc) xmlFreeDoc (doc);
	if (ctxt) xmlFreeParserCtxt (ctxt);
	if (sch) xmlSchemaFree (sch);
	if (valid_ctxt) xmlSchemaFreeValidCtxt (valid_ctxt);
	if (schema_ctxt) xmlSchemaFreeParserCtxt (schema_ctxt);
	
	return ret;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

