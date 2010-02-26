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
#include <stdlib.h>
#include <check.h>
#include <string.h>

#include "testdefinitionparser.h"
#include "testdefinitiondatatypes.h"
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
td_suite *suite;
td_set   *set;
char  *suite_description;
/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL void ut_test_suite (td_suite *);
/* ------------------------------------------------------------------------- */
LOCAL void ut_test_suite_description (char *); 
/* ------------------------------------------------------------------------- */
LOCAL void ut_test_set (td_set *);     
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
LOCAL void ut_test_suite (td_suite *s)
{
    if (suite)
	td_suite_delete (suite);
    suite = s;
}
/* ------------------------------------------------------------------------- */
LOCAL void ut_test_suite_description (char *desc)
{
    if (suite_description)
	free (suite_description);
    suite_description = desc;
	    
}
/* ------------------------------------------------------------------------- */
LOCAL void ut_test_set (td_set *s)
{
    if (set)
	td_set_delete (set);
    set = s;
}
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
/* ------------------------------------------------------------------------- */
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
/* ------------------------------------------------------------------------- */
START_TEST (test_parse_not_existing_test_definition)

    /* Test parsing not existing test definition xml. */
    char *file_not_existing = "not_existing_xml.xml";
    testrunner_lite_options test_opts;

    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));
    test_opts.input_filename = malloc (strlen(file_not_existing)+1);
    strcpy (test_opts.input_filename, file_not_existing);

    fail_if (parse_test_definition(&test_opts) == 0, NULL);

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_parse_invalid_test_definition)

    /* Test parsing invalid test definition xml. */
    testrunner_lite_options test_opts;

    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));
    test_opts.input_filename = malloc (strlen(TESTDATA_INVALID_XML_1)+1);
    strcpy (test_opts.input_filename, TESTDATA_INVALID_XML_1);

    fail_if (parse_test_definition(&test_opts) == 0, NULL);

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_reader_null_callbacks)

    td_parser_callbacks cbs;
    testrunner_lite_options test_opts;

    memset (&cbs, 0x0, sizeof (cbs));
    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));
    test_opts.input_filename = malloc (strlen(TESTDATA_VALID_XML_1)+1);
    strcpy (test_opts.input_filename, TESTDATA_VALID_XML_1);
    fail_if (td_register_callbacks (&cbs));
    fail_if (td_reader_init(&test_opts));
    while (td_next_node() == 0);

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_reader_init_null)

    testrunner_lite_options test_opts;
    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));
    fail_unless (td_reader_init(&test_opts));

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_reader_suite)

    td_parser_callbacks cbs;
    testrunner_lite_options test_opts;
    
    suite = NULL;
    
    memset (&cbs, 0x0, sizeof (cbs));
    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));
    test_opts.input_filename = malloc (strlen(TESTDATA_VALID_XML_1)+1);
    strcpy (test_opts.input_filename, TESTDATA_VALID_XML_1);
    cbs.test_suite = ut_test_suite;
    cbs.test_suite_description = ut_test_suite_description;

    fail_if (td_register_callbacks (&cbs));
    fail_if (td_reader_init(&test_opts));
    
    while (td_next_node() == 0);
    
    fail_unless (suite != NULL);
    fail_if (strcmp ((const char *)suite->gen.name, "examplebinary-tests"));
    fail_if (strcmp ((const char *)suite->domain, "sample_suite_domain"));
    td_suite_delete (suite);
    suite = NULL;
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_reader_set)

    td_parser_callbacks cbs;
    testrunner_lite_options test_opts;
    
    suite = NULL;
    set = NULL;
    
    td_reader_close();
    
    memset (&cbs, 0x0, sizeof (cbs));
    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));
    test_opts.input_filename = malloc (strlen(TESTDATA_VALID_XML_1)+1);
    strcpy (test_opts.input_filename, TESTDATA_VALID_XML_1);
    cbs.test_suite = ut_test_suite;
    cbs.test_suite_description = ut_test_suite_description;
    cbs.test_set = ut_test_set;

    fail_if (td_register_callbacks (&cbs));
    fail_if (td_reader_init(&test_opts));
    
    while (td_next_node() == 0);
    
    fail_unless (suite != NULL);
    fail_if (strcmp ((const char *)suite->gen.name, "examplebinary-tests2"));
    fail_if (strcmp ((const char *)suite->domain, "domain2"));
    td_suite_delete (suite);
    suite = NULL;

    fail_unless (set != NULL);
    fail_if (strcmp ((const char *)set->gen.name, "testset3"));
    fail_if (strcmp ((const char *)set->gen.name, "testset3"));
    fail_if (strcmp ((const char *)set->gen.description, "set description 1"));
    fail_if (strcmp ((const char *)set->feature, "feature2"));
    fail_unless (xmlListSize(set->environments) == 1);
    fail_unless (xmlListSize(set->pre_steps) == 1);
    fail_unless (xmlListSize(set->cases) == 3);
    
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

    tc = tcase_create ("Test reader with NULL callbacks.");
    tcase_add_test (tc, test_reader_null_callbacks);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test reader_init with NULL opts.");
    tcase_add_test (tc, test_reader_init_null);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Validate suite reading.");
    tcase_add_test (tc, test_reader_suite);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Validate set reading.");
    tcase_add_test (tc, test_reader_set);
    suite_add_tcase (s, tc);

    return s;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
