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

#include <libxml/parser.h>
#include <libxml/tree.h>

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

int parse_test_definition (testrunner_lite_options *opts){
    xmlParserCtxtPtr ctxt; 
    xmlDocPtr doc; 
    xmlParserInputPtr inp;
    
    ctxt = xmlNewParserCtxt();
    if (ctxt == NULL) {
	    fprintf (stderr, "%s: Failed to allocate parser context\n",
		     PROGNAME);
	    return 1;
    }
    inp =  xmlLoadExternalEntity("testdefinition.xsd",
				 "testdefinition",
				 ctxt);
    if (inp == NULL) {
	    fprintf (stderr, "%s: Failed to load DTD\n",
		     PROGNAME);
	    return 1;

    }
    doc = xmlCtxtReadFile(ctxt, opts->input_filename, NULL, 0);
    
    /* XML_PARSE_DTDLOAD | XML_PARSE_DTDVALID);  */
    if (doc == NULL) {
	    fprintf(stderr, "%s: Failed to parse %s\n", PROGNAME,
		    opts->input_filename);
	    return 1;
    } else if (!ctxt->valid) {
	    fprintf(stderr, "%s: Failed to validate %s\n", PROGNAME, 
		    opts->input_filename);
	    xmlFreeDoc(doc);
	    return 1;
    }
    
    xmlFreeParserCtxt(ctxt);
    xmlFreeDoc(doc);
    
    xmlCleanupParser();
    xmlMemoryDump();
  
    return 0;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

