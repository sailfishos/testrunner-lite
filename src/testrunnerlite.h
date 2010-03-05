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

#ifndef TESTRUNNERLITE_H
#define TESTRUNNERLITE_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
/* None */

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */
#define LOCAL static
#define PROGNAME "testrunner-lite"
/* ------------------------------------------------------------------------- */
/* DATA TYPES */
/* ------------------------------------------------------------------------- */
/** Result output type */
typedef enum {
	OUTPUT_TYPE_XML = 1,
	OUTPUT_TYPE_TXT 
} result_output;

/** Used for storing and passing user (command line) options.*/
typedef struct {
	char *input_filename;  /**< the input xml file */
	char *output_filename; /**< the result file */
	char *output_folder;   /**< folder for storing results */
        char *environment;     /**< execution environment */
	int   disable_schema;  /**< flag for disabling DTD validation */
	int   semantic_schema; /**< flag for enabling sricter DTD */
        result_output   output_type;   /**< result output type selector */
	int   run_automatic;   /**< flag for automatic tests */  
	int   run_manual;      /**< flag for manual tests */
	int   verbose;         /**< print debug is set */
} testrunner_lite_options;    
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES */
/* None */

/* ------------------------------------------------------------------------- */
#endif                          /* TESTRUNNERLITE_H */
/* End of file */
