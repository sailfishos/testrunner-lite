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

#ifndef TESTDEFINITIONDATATYPES_H
#define TESTDEFINITIONDATATYPES_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
#include <sys/time.h>
#include <sys/types.h>
#include <libxml/hash.h>
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
#define DEFAULT_TIMEOUT 90
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
	/* parser fills */
	xmlChar *hw_detector;         /**< Command used to detect HW  */
	xmlChar *description;
	xmlChar *version;
	/* executor fills */
	xmlChar *detected_hw;         /**< Detected HW identifier */
} td_td;
/* ------------------------------------------------------------------------- */
/** Test suite */
typedef struct {
	td_gen_attribs gen;   /**< General attributes */
	xmlChar *description; /**< Description */
	int        filtered;  /**< Suite is filtered */
} td_suite;
/* ------------------------------------------------------------------------- */
/** File element (for get tag) */
typedef struct {
	int        delete_after;  /**< Delete_after attribute */
	int        measurement;   /**< Measurement attribute */
	int        series;        /**< Is measurement series */
        xmlChar    *filename;     /**< File name */
} td_file;
/* ------------------------------------------------------------------------- */
/** Test set. */
typedef struct {
	td_gen_attribs gen;      /**< General attributes */
	xmlListPtr pre_steps;    /**< Steps executed before each test case */
	xmlListPtr post_steps;   /**< Steps executed after each test case */
	xmlListPtr post_reboot_steps; /**< Steps executed after reboot */
	xmlListPtr cases;        /**< Test cases in this set */
	xmlListPtr environments; /**< Environments (hardware, scratchbox) */
	xmlListPtr gets;         /**< Get commands */
	int        filtered;     /**< Set is filtered */
	xmlChar   *description;  /**< Set description */
	/* Executor fills */
	xmlChar    *environment; /**< Current environment */
} td_set;
/* ------------------------------------------------------------------------- */
#ifdef ENABLE_EVENTS
/** Test event parameter. */
typedef struct {
	xmlChar      *type;        /**< Type of param */
	xmlChar      *name;        /**< Name of param */
	xmlChar      *value;       /**< Value of param as a string */
} td_event_param;
/* ------------------------------------------------------------------------- */
/** Test event type. */
typedef enum {
	EVENT_TYPE_UNKNOWN = 0,
	EVENT_TYPE_SEND,
	EVENT_TYPE_WAIT
} event_type_t;
/* ------------------------------------------------------------------------- */
/** Test event. */
typedef struct {
	event_type_t  type;        /**< Type of event */
	xmlChar      *resource;        /**< Resource (address[/subject]) */
	unsigned long timeout;     /**< Timeout of event */
	xmlListPtr    params;      /**< Parameters of event */
} td_event;
/* ------------------------------------------------------------------------- */
#endif	/* ENABLE_EVENTS */
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
	int      control;         /**< control the device (reboot etc) */
#ifdef ENABLE_EVENTS
	td_event*      event;     /**< event step */
#endif

	/* Executor fills */
	xmlChar *failure_info;    /**< optional failure info */
	time_t   start;           /**< step execution start time */
	time_t   end;             /**< step execution end time */
	xmlChar *stdout_;         /**< step stdout printouts */
	xmlChar *stderr_;         /**< step stderr printouts */
	pid_t    pgid;            /**< step process group id */
	pid_t    pid;             /**< step process id */
	int      fail;            /**< step is failed, regardless of result */
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
	xmlChar   *tc_id;       /**< TC_ID */
	xmlChar   *state;       /**< State attribute */
	xmlChar   *bugzilla_id;  /**< Id mapping the case to bug or 
				    feature number in bugs.meego.com */
        xmlChar   *description;  /**< Description element */
	xmlListPtr gets;         /**< Get commands */

	/* Executor fills */
	xmlListPtr measurements;         /**< measurements */
	xmlListPtr series;         /**< measurement series */
	xmlChar   *comment;     /**< Manual test case comment */
	case_result_t  case_res; /**< Case result */
	xmlChar   *failure_info;   /**< Optional failure info */
	xmlChar   *rich_core_uuid; /**< Optional UUID for rich core dumps */
	xmlHashTablePtr crashes; /**< Maps a crash log file to telemetry URL */
	xmlListPtr post_reboot_steps; /**< Steps executed after reboot */
	int        dummy;       /**< Case is dummy - used with pre post steps */
	int        filtered;    /**< Case is filtered */
} td_case;
/* ------------------------------------------------------------------------- */
/** Pre/post steps */
typedef struct {
	xmlListPtr steps;       /**< Steps of pre/post steps */
#define DEFAULT_PRE_STEP_TIMEOUT 180
	unsigned long timeout;  /**< Timeout */
} td_steps;
/* ------------------------------------------------------------------------- */
/** Test measurement */
typedef struct {
	xmlChar *name;              /**< E.g. bt.upload */
	xmlChar *group;             /**< E.g. bt_measurements */
	double   value;             /**< Value of measurement */
	xmlChar *unit;              /**< E.g. Mb/s */
	int      target_specified;  /**< Is target and failure specified ? */
	double   target;            /**< Target value */
	double   failure;           /**< Failure value */
} td_measurement;
/* ------------------------------------------------------------------------- */
/** Test measurement item */
typedef struct {
	double   value;             /**< Value of measurement */
	int      has_timestamp;     /**< 1 if timestamp set, otherwise 0 */
	struct timespec timestamp;  /**< Timestamp of the measurement */
} td_measurement_item;
/* ------------------------------------------------------------------------- */
/** Test measurement series */
typedef struct {
	xmlChar *name;              /**< Name of series */
	xmlChar *group;             /**< Measurement group */
	xmlChar *unit;              /**< Unit of series */
	xmlListPtr items;           /**< measurement item series */
	int      target_specified;  /**< Is target and failure specified ? */
	double   target;            /**< Target value */
	double   failure;           /**< Failure value */
	int      has_interval;	    /**< 1 if interval set, otherwise 0 */
	int      interval;	    /**< Interval value */
	xmlChar *interval_unit;	    /**< Interval unit */
} td_measurement_series;
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
#ifdef ENABLE_EVENTS
const char *event_type_str (event_type_t);
/* ------------------------------------------------------------------------- */
#endif
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
td_measurement_series *td_measurement_series_create();
/* ------------------------------------------------------------------------- */
void td_measurement_series_delete(xmlLinkPtr);
/* ------------------------------------------------------------------------- */
td_measurement_item *td_measurement_item_create();
/* ------------------------------------------------------------------------- */
void td_measurement_item_delete(xmlLinkPtr);
/* ------------------------------------------------------------------------- */
void td_file_delete (td_file *);
/* ------------------------------------------------------------------------- */
#ifdef ENABLE_EVENTS
td_event *td_event_create();
/* ------------------------------------------------------------------------- */
void td_event_delete(td_event *);
/* ------------------------------------------------------------------------- */
td_event_param *td_event_param_create();
/* ------------------------------------------------------------------------- */
void td_event_param_delete(xmlLinkPtr lk);
/* ------------------------------------------------------------------------- */
#endif	/* ENABLE_EVENTS */
#endif                          /* TESTDEFINITIONDATATYPES_H */
/* End of file */
