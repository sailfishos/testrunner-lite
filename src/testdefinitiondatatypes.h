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
	xmlChar *type;          /**< Type attribute */
	unsigned long timeout;  /**< Timeout (defaults to 90) */
	xmlChar *level;         /**< Level (Component, Feature, System) */
	xmlChar *domain;        /**< Domain */
	xmlChar *feature;       /**< Feature attribute */
	xmlChar *component;     /**< Component attribute */

	int      manual;        /**< Manual flag (default false) */
	int      insignificant; /**< Insignificant flag (default false) */
	xmlChar *hwid;          /**< Comma separated list of HW identifiers */
} td_gen_attribs;
/* ------------------------------------------------------------------------- */
/** Test definition */
typedef struct {
	xmlChar *hw_detector;         /**< Command used to detect HW  */
	xmlChar *detected_hw;         /**< Detected HW identifier */
} td_td;
/* ------------------------------------------------------------------------- */
/** Test suite */
typedef struct {
	td_gen_attribs gen;   /**< General attributes */
	int        filtered;  /**< Suite is filtered */
} td_suite;
/* ------------------------------------------------------------------------- */
/** File element (for get tag) */
typedef struct {
	int        delete_after;  /**< Delete_after attribute */
	xmlChar    *filename;     /**< File name */
} td_file;
/* ------------------------------------------------------------------------- */
/** Test set. */
typedef struct {
	td_gen_attribs gen;      /**< General attributes */
	xmlListPtr pre_steps;    /**< Steps executed before each test case */
	xmlListPtr post_steps;   /**< Steps executed after each test case */
	xmlListPtr cases;        /**< Test cases in this set */
	xmlListPtr environments; /**< Environments (hardware, scratchbox) */
	xmlListPtr gets;         /**< Get commands */
	int        filtered;     /**< Set is filtered */
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
	int      has_result;      /**< should we trust the return_code */
	int      return_code;     /**< actual result of step */
	int      manual;          /**< Manual flag (default from case) */

	/* Executor fills */
	xmlChar *failure_info;    /**< optional failure info */
	time_t   start;           /**< step execution start time */
	time_t   end;             /**< step execution end time */
	xmlChar *stdout_;         /**< step stdout printouts */
	xmlChar *stderr_;         /**< step stderr printouts */
	pid_t    pgid;            /**< step process group id */
	pid_t    pid;             /**< step process id */
} td_step;
/* ------------------------------------------------------------------------- */
/** Test case result */
typedef enum {
	CASE_FAIL = 0,
	CASE_PASS,
	CASE_NA
} case_result_t;
/* ------------------------------------------------------------------------- */
/** Test case */
typedef struct {
	/* Parser fills */
	td_gen_attribs gen;     /**< General attributes */
	xmlChar   *subfeature;  /**< Sub feature attribute */
	xmlListPtr steps;       /**< Steps in this test case */
	xmlChar   *tc_title;    /**< TC_Title */
	xmlChar   *state;       /**< State attribute */
	/* Executor fills */
	xmlChar   *comment;     /**< Manual test case comment */
	case_result_t  case_res; /**< Case result */
	xmlChar   *failure_info;   /**< optional failure info */
	int        dummy;       /**< Case is dummy - used with pre post steps */
	int        filtered;    /**< Case is filtered */
} td_case;
/* ------------------------------------------------------------------------- */
/** Pre/post steps */
typedef struct {
	xmlListPtr steps;       /**< Steps of pre/post steps */
	unsigned long timeout;  /**< Timeout */
} td_steps;
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
const char *case_result_str (case_result_t);
/* ------------------------------------------------------------------------- */
td_td *td_td_create();
/* ------------------------------------------------------------------------- */
void td_td_delete(td_td *);
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
td_steps *td_steps_create();
/* ------------------------------------------------------------------------- */
void td_steps_delete(xmlLinkPtr);
/* ------------------------------------------------------------------------- */
#endif                          /* TESTDEFINITIONDATATYPES_H */
/* End of file */
