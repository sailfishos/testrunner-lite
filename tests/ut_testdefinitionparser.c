/* * This file is part of testrunner-lite *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 *
 * Contact: Test-tools-dev <Test-tools-dev@projects.maemo.org>
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
#include <check.h>
#include <string.h>

#include "testdefinitionparser.h"
#include "testrunnerlite.h"
#include "testrunnerlitetestscommon.h"

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
/* None */

/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
START_TEST (test_parse_test_definition)

    /* Test parsing valid test definition xml. */
    testrunner_lite_options test_opts;

    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));
    test_opts.input_filename = malloc (strlen(TESTDATA_VALID_XML_1)+1);
    strcpy (test_opts.input_filename, TESTDATA_VALID_XML_1); 

    fail_if (parse_test_definition(&test_opts) != 0, 
        "Parsing test definition failed!");

END_TEST

START_TEST (test_parse_test_definition_no_schema)

    /* Test parsing valid test definition xml (without schema). */
    testrunner_lite_options test_opts;

    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));
    test_opts.input_filename = malloc (strlen(TESTDATA_VALID_XML_1)+1);
    strcpy (test_opts.input_filename, TESTDATA_VALID_XML_1);

    test_opts.disable_schema = 1;

    fail_if (parse_test_definition(&test_opts) != 0, 
        "Parsing test definition failed!");

END_TEST

START_TEST (test_parse_not_existing_test_definition)

    /* Test parsing not existing test definition xml. */
    char *file_not_existing = "not_existing_xml.xml";
    testrunner_lite_options test_opts;

    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));
    test_opts.input_filename = malloc (strlen(file_not_existing)+1);
    strcpy (test_opts.input_filename, file_not_existing);

    fail_if (parse_test_definition(&test_opts) == 0, NULL);

END_TEST

START_TEST (test_parse_invalid_test_definition)

    /* Test parsing invalid test definition xml. */
    testrunner_lite_options test_opts;

    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));
    test_opts.input_filename = malloc (strlen(TESTDATA_INVALID_XML_1)+1);
    strcpy (test_opts.input_filename, TESTDATA_INVALID_XML_1);

    fail_if (parse_test_definition(&test_opts) == 0, NULL);

END_TEST

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
Suite *make_testdefinitionparser_suite (void)
{
    /* Create suite. */
    Suite *s = suite_create ("testdefinitionparser");

    /* Create test cases and add to suite. */
    TCase *tc;

    tc = tcase_create ("Test parsing valid test definition.");
    tcase_add_test (tc, test_parse_test_definition);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test parsing valid test definition - no schema validation.");
    tcase_add_test (tc, test_parse_test_definition_no_schema);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test parsing not existing test definition.");
    tcase_add_test (tc, test_parse_not_existing_test_definition);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test parsing invalid test definition.");
    tcase_add_test (tc, test_parse_invalid_test_definition);
    suite_add_tcase (s, tc);

    return s;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
