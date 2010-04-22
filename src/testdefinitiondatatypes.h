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
/** General attributes */
typedef struct {
	xmlChar *name;          /**< Name (for suite, set, case ...) */
        xmlChar *description;   /**< Description */
        xmlChar *requirement;   /**< Requirement attribute */
	unsigned long timeout;  /**< Timeout (defaults to 90) */
	xmlChar *level;         /**< Level (Component, Feature, System) */
	int      manual;        /**< Manual flag (default false) */
	int      insignificant; /**< Insignificant flag (default false) */
} td_gen_attribs;

/** Test suite */
typedef struct {
	td_gen_attribs gen;   /**< General attributes */
	xmlChar    *domain;   /**< Domain */

} td_suite;
/* ------------------------------------------------------------------------- */
/** Test set. */
typedef struct {
	td_gen_attribs gen;      /**< General attributes */
	xmlChar   *feature;      /**< Feature attribute */
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
	int      has_expected_result; /**< is expected result specified */
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
	td_gen_attribs gen;     /**< General attributes */
	xmlChar   *subfeature;  /**< Sub feature attribute */
	xmlListPtr steps;       /**< Steps in this test case */
	xmlChar   *comment;     /**< Manual test case comment */
	/* Executor fills */
	int        passed;      /**< Flag stating whether this case is passed */
	int        dummy;       /**< Case is dummy - used with pre post steps */
	int        filtered;    /**< Case is filtered */
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
