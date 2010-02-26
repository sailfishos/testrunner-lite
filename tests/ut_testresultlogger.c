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

#include "testresultlogger.h"
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
START_TEST (test_logger_init_inv_args)

    testrunner_lite_options test_opts;

    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));
    fail_unless (init_result_logger(&test_opts));
    test_opts.output_type = OUTPUT_TYPE_XML;
    fail_unless (init_result_logger(&test_opts));
    test_opts.output_type = OUTPUT_TYPE_TXT;
    fail_unless (init_result_logger(&test_opts));
    
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_logger_write_xml)

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
    fail_unless (set != NULL);

    test_opts.output_type = OUTPUT_TYPE_XML;
    test_opts.output_filename = "/dev/null";
    fail_if (init_result_logger (&test_opts));
    fail_if (write_pre_suite_tag (suite));
    fail_if (write_post_suite_tag ());
    fail_if (write_pre_set_tag (set));
    fail_if (write_post_set_tag (set));

    td_suite_delete (suite);
    suite = NULL;
    td_set_delete (set);
    set = NULL;

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_logger_write_txt)

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
    fail_unless (set != NULL);

    test_opts.output_type = OUTPUT_TYPE_XML;
    test_opts.output_filename = "/dev/null";
    fail_if (init_result_logger (&test_opts));
    fail_if (write_pre_suite_tag (suite));
    fail_if (write_post_suite_tag ());
    fail_if (write_pre_set_tag (set));
    fail_if (write_post_set_tag (set));

    td_suite_delete (suite);
    suite = NULL;
    td_set_delete (set);
    set = NULL;

END_TEST
/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
Suite *make_testresultlogger_suite (void)
{
    /* Create suite. */
    Suite *s = suite_create ("testresultlogger");

    /* Create test cases and add to suite. */
    TCase *tc;

    tc = tcase_create ("Test logge init with invalid arguments.");
    tcase_add_test (tc, test_logger_init_inv_args);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test logger write methods to xml.");
    tcase_add_test (tc, test_logger_write_xml);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test logger write methods to text file.");
    tcase_add_test (tc, test_logger_write_txt);
    suite_add_tcase (s, tc);


    return s;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
