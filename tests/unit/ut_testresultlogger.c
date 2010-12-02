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
#include "hwinfo.h"

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
    hw_info hwinfo;
    
    memset (&hwinfo, 0x0, sizeof (hw_info));
    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));

    fail_unless (init_result_logger(&test_opts, &hwinfo));
    test_opts.output_type = OUTPUT_TYPE_XML;
    fail_unless (init_result_logger(&test_opts, &hwinfo));
    test_opts.output_type = OUTPUT_TYPE_TXT;
    fail_unless (init_result_logger(&test_opts, &hwinfo));
    
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_logger_write_xml)

    td_parser_callbacks cbs;
    testrunner_lite_options test_opts;
    hw_info hwinfo;
    

    suite = NULL;
    set = NULL;
    
    td_reader_close();
    
    memset (&cbs, 0x0, sizeof (cbs));
    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));
    memset (&hwinfo, 0x0, sizeof (hw_info));

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
    fail_if (init_result_logger (&test_opts, &hwinfo));
    fail_if (write_pre_suite (suite));
    fail_if (write_post_suite ());
    fail_if (write_pre_set (set));
    fail_if (write_post_set (set));

    td_suite_delete (suite);
    suite = NULL;
    td_set_delete (set);
    set = NULL;

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_logger_write_txt)

    td_parser_callbacks cbs;
    testrunner_lite_options test_opts;
    hw_info hwinfo;
    

    suite = NULL;
    set = NULL;
    
    td_reader_close();
    
    memset (&cbs, 0x0, sizeof (cbs));
    memset (&test_opts, 0x0, sizeof (testrunner_lite_options));
    memset (&hwinfo, 0x0, sizeof (hw_info));

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
    fail_if (init_result_logger (&test_opts, &hwinfo));
    fail_if (write_pre_suite (suite));
    fail_if (write_post_suite ());
    fail_if (write_pre_set (set));
    fail_if (write_post_set (set));

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

    tc = tcase_create ("Test logger init with invalid arguments.");
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
