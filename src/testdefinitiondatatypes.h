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

#ifndef TESTDEFINITIONDATATYPES_H
#define TESTDEFINITIONDATATYPES_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
#include <sys/time.h>
#include <libxml/xmlstring.h>
#include <libxml/list.h>
#include "testrunnerlite.h"

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */

/* ------------------------------------------------------------------------- */
/* DATA TYPES */
/* ------------------------------------------------------------------------- */
/** Test suite. */
typedef struct {
	xmlChar *name;        /**< Suite name */
        xmlChar *domain;
        xmlChar *suite_type;
        xmlChar *level;
        xmlChar *timeout;
        xmlChar *requirement;
        xmlChar *description; /**< Suite description */
} td_suite;
/* ------------------------------------------------------------------------- */
/** Test set. */
typedef struct {
	xmlChar   *name;         /**< Set name */
	xmlChar   *description;  /**< Set description */
	xmlListPtr pre_steps;    /**< Steps executed before each test case */
	xmlListPtr post_steps;   /**< Steps executed after each test case */
	xmlListPtr cases;        /**< Test cases in this set */
	xmlListPtr environments; /**< Environments (hardware, scratchbox) */
	xmlListPtr gets;         /**< Get commands */
	/* Executor fills */
	xmlChar    *environment; /**< Current environment */
} td_set;
/* ------------------------------------------------------------------------- */
/** Test step. */
typedef struct {
	/* Parser fills */
	xmlChar *step;            /**< Test step as a string 
				     (with possible CDATA)    */
	int      expected_result; /**< expected result of step */
	int      return_code;     /**< actual result of step */
	/* Executor fills */
	xmlChar *failure_info;    /**< optional failure info */
	time_t   start;           /**< step execution start time */
	time_t   end;             /**< step execution end time */
	xmlChar *stdout_;         /**< step stdout printouts */
	xmlChar *stderr_;         /**< step stderr printouts */
} td_step;
/* ------------------------------------------------------------------------- */
/** Test case */
typedef struct {
	/* Parser fills */
	xmlChar   *name;        /**< Test case name */
	xmlChar   *description; /**< Test case description */
	unsigned long timeout;  /**< Test case timeout */
	xmlListPtr steps;       /**< Steps in this test case */
	/* Executor fills */
	int        passed;      /**< Flag stating whether this case is passed */
} td_case;
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
td_suite *td_suite_create();
/* ------------------------------------------------------------------------- */
void td_suite_delete(td_suite *);
/* ------------------------------------------------------------------------- */
td_set *td_set_create();
/* ------------------------------------------------------------------------- */
void td_set_delete(td_set *);
/* ------------------------------------------------------------------------- */
td_step *td_step_create();
/* ------------------------------------------------------------------------- */
void td_step_delete(xmlLinkPtr);
/* ------------------------------------------------------------------------- */
td_case *td_case_create();
/* ------------------------------------------------------------------------- */
void td_case_delete(xmlLinkPtr);
/* ------------------------------------------------------------------------- */
#endif                          /* TESTDEFINITIONDATATYPES_H */
/* End of file */
