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
#include <stdio.h>
#include <check.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "testrunnerlitetestscommon.h"
#include "testrunnerlite.h"
#include "../src/testfilters.c"
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
void
setup (void)
{
	init_filters();
}
/* ------------------------------------------------------------------------- */
void
teardown (void)
{
	cleanup_filters();
}
/* ------------------------------------------------------------------------- */
START_TEST (test_filter_parsing_simple)
	fail_if (parse_filter_string ("testcase=testi"));
	fail_if (parse_filter_string ("-testcase=testi"));
	fail_if (parse_filter_string ("testcase=\"testi 1\""));
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_filter_parsing_complex)
	fail_if (parse_filter_string ("-requirement=1001,\"Some req\","
				      "2001,other_req +testcase=testi"));

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_filter_parsing_inv_white_space)
     fail_unless (parse_filter_string (" "));
     fail_unless (parse_filter_string ("level= "));
     fail_unless (parse_filter_string ("-requirement=1001,\"Some req\","
				      "2001,other_req +testcase=testi "));
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_filter_invalid_type)
	fail_unless (parse_filter_string ("foo=testi"));
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_filter_missing_quote)
	fail_unless (parse_filter_string ("testcase=\"testi"));
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_test_case_filter)
     td_case c;
     test_filter filt;
     filt.value_list = xmlListCreate (filter_value_delete, 
				      filter_value_list_compare);
     xmlListAppend (filt.value_list, BAD_CAST "test");

     filt.exclude = 0;
     filt.key = BAD_CAST "testcase";
     c.gen.name = BAD_CAST "test";

     fail_if (test_case_filter (&filt, (void *)&c));
     filt.exclude = 1;
     fail_unless (test_case_filter (&filt, (void *)&c));
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_requirement_filter)
     td_case c;
     test_filter filt;
     filt.value_list = xmlListCreate (filter_value_delete, 
				      filter_value_list_compare);
     xmlListAppend (filt.value_list, BAD_CAST "1000");

     filt.exclude = 0;
     filt.key = BAD_CAST "requirement";
     c.gen.requirement = BAD_CAST "1001,\"some req\",2000";

     fail_unless (requirement_filter (&filt, (void *)&c));

     filt.exclude = 1;
     fail_if (requirement_filter (&filt, (void *)&c));


     xmlListAppend (filt.value_list, BAD_CAST "2000");
     filt.exclude = 0;
     fail_if (requirement_filter (&filt, (void *)&c));

     filt.exclude = 1;
     fail_unless (requirement_filter (&filt, (void *)&c));
     
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_test_set_filter)
     td_set s;
     test_filter filt;
     filt.value_list = xmlListCreate (filter_value_delete, 
				      filter_value_list_compare);
     xmlListAppend (filt.value_list, BAD_CAST "setname");

     filt.exclude = 0;
     filt.key = BAD_CAST "testset";
     s.gen.name = BAD_CAST "setname";

     fail_if (test_set_filter (&filt, (void *)&s));
     filt.exclude = 1;
     fail_unless (test_set_filter (&filt, (void *)&s));
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_type_filter)
     td_case c;
     test_filter filt;
     filt.value_list = xmlListCreate (filter_value_delete, 
				      filter_value_list_compare);
     xmlListAppend (filt.value_list, BAD_CAST "unit");

     filt.exclude = 0;
     filt.key = BAD_CAST "type";
     c.gen.type = BAD_CAST "unit";

     fail_if (type_filter (&filt, (void *)&c));
     filt.exclude = 1;
     fail_unless (type_filter (&filt, (void *)&c));
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_feature_filter)
     td_set s;
     test_filter filt;
     filt.value_list = xmlListCreate (filter_value_delete, 
				      filter_value_list_compare);
     xmlListAppend (filt.value_list, BAD_CAST "ui");

     filt.exclude = 0;
     filt.key = BAD_CAST "feature";
     s.feature = BAD_CAST "some_fea,voice call,3g data";

     fail_unless (feature_filter (&filt, (void *)&s));

     filt.exclude = 1;
     fail_if (feature_filter (&filt, (void *)&s));


     xmlListAppend (filt.value_list, BAD_CAST "voice call");
     filt.exclude = 0;
     fail_if (feature_filter (&filt, (void *)&s));

     filt.exclude = 1;
     fail_unless (feature_filter (&filt, (void *)&s));
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (acceptance_test_case_filter)
     int ret;
     char cmd[1024];

     /* Execute testrunner with filter_tests.xml and testcase filter  */
     sprintf (cmd, "%s -a -v -f %s -e scratchbox "
	      "-o /tmp/testrunnerlitetestdir/res.xml "
	      "-l'-testcase=serm003'", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_FILTER_TESTS_XML);
     ret = system (cmd);
     fail_if (ret, cmd);

     /* Check that case serm003 is excluded */
     ret = system ("grep serm003 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep serm002 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);

     /* Try another filter  */
     sprintf (cmd, "%s -a -v -f %s "
	      "-o /tmp/testrunnerlitetestdir/res.xml "
	      "-l'testcase=abc111,abc211'", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_FILTER_TESTS_XML);
     ret = system (cmd);
     fail_if (ret, cmd);

     /* Check that cases abc111 and abc211 are included */
     ret = system ("grep abc111 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
     ret = system ("grep abc211 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
     /* and e.g. abc112 is not */
     ret = system ("grep abc112 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (acceptance_test_set_filter)
     int ret;
     char cmd[1024];

     /* Execute testrunner with filter_tests.xml and set filter  */
     sprintf (cmd, "%s -a -v -f %s "
	      "-o /tmp/testrunnerlitetestdir/res.xml "
	      "-l'-testset=testset21'", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_FILTER_TESTS_XML);
     ret = system (cmd);
     fail_if (ret, cmd);

     /* Check that only testset21 is excluded */
     ret = system ("grep testset21 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep testset11 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
     ret = system ("grep testset12 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
     ret = system ("grep testset31 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
     ret = system ("grep Yet-another-test-set "
		   "/tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);

     /* Try another filter  */
     sprintf (cmd, "%s -a -v -f %s "
	      "-o /tmp/testrunnerlitetestdir/res.xml "
	      "-l'testset=testset12,Yet-another-test-set'", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_FILTER_TESTS_XML);
     ret = system (cmd);
     fail_if (ret, cmd);
     /* Check that only testset12 are Yet-another-testset are included */
     ret = system ("grep testset21 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep testset11 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep testset12 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
     ret = system ("grep testset31 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep Yet-another-test-set "
		   "/tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);


END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (acceptance_test_requirement_filter)
     int ret;
     char cmd[1024];

     /* Execute testrunner with filter_tests.xml and requirement filter  */
     sprintf (cmd, "%s -a -e scratchbox -v -f %s "
	      "-o /tmp/testrunnerlitetestdir/res.xml "
	      "-l'requirement=66666,50002'", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_FILTER_TESTS_XML);
     ret = system (cmd);
     fail_if (ret, cmd);

     /* Check that cases are filtered correctly by grepping result file */
     /* set testset11 */
     ret = system ("grep serm002 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep serm003 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep serm004 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     /* set testset12 */
     ret = system ("grep serm005 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep manual_test_1 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep manual_test_2 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep serm006 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     /* set testset21 */
     ret = system ("grep abc111 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
     ret = system ("grep abc112 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep abc113 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     /* set testset31 */
     ret = system ("grep abc211 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
     ret = system ("grep abc212 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
     ret = system ("grep abc213 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
END_TEST

START_TEST (acceptance_test_feature_filter)
     int ret;
     char cmd[1024];

     /* Execute testrunner with filter_tests.xml and feature filter  */
     sprintf (cmd, "%s -a -v -f %s "
	      "-o /tmp/testrunnerlitetestdir/res.xml "
	      "-l'feature=feature2'", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_FILTER_TESTS_XML);
     ret = system (cmd);
     fail_if (ret, cmd);

     /* Check that sets are filtered correctly by grepping result file */
     ret = system ("grep testset11 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep testset12 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
     ret = system ("grep testset21 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (acceptance_test_type_filter)
     int ret;
     char cmd[1024];

     /* Execute testrunner with filter_tests.xml and type filter  */
     sprintf (cmd, "%s -a -v -f %s -e scratchbox "
	      "-o /tmp/testrunnerlitetestdir/res.xml "
	      "-l'type=Integration'", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_FILTER_TESTS_XML);
     ret = system (cmd);
     fail_if (ret, cmd);

     /* assert all cases in suites 0,1,2 have been filtered out */
     ret = system ("grep serm002 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep serm003 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep serm004 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep serm005 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep manual_test_1 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep manual_test_2 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep serm006 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep abc111 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep abc112 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep abc113 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep abc211 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep abc212 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep abc213 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     /* assert all cases in suite 3 were left active */
     ret = system ("grep xxx311 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
     ret = system ("grep xxx312 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
     ret = system ("grep xxx313 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (acceptance_test_filter_combo)
     int ret;
     char cmd[1024];

     /* Execute testrunner with filter_tests.xml and type filter  */
     sprintf (cmd, "%s -a -v -f %s -e scratchbox "
	      "-o /tmp/testrunnerlitetestdir/res.xml "
	      "-l'type=Integration -testcase=xxx312'", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_FILTER_TESTS_XML);
     ret = system (cmd);
     fail_if (ret, cmd);

     /* assert all cases in suites 0,1,2 have been filtered out */
     ret = system ("grep serm002 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep serm003 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep serm004 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep serm005 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep manual_test_1 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep manual_test_2 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep serm006 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep abc111 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep abc112 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep abc113 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep abc211 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep abc212 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep abc213 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     /* assert all cases in expect xxx312 were left active */
     ret = system ("grep xxx311 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
     ret = system ("grep xxx312 /tmp/testrunnerlitetestdir/res.xml");
     fail_unless (ret);
     ret = system ("grep xxx313 /tmp/testrunnerlitetestdir/res.xml");
     fail_if (ret);
END_TEST
/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
Suite *make_testfilter_suite (void)
{
    /* Create suite. */
    Suite *s = suite_create ("testfilters");

    /* Create test cases and add to suite. */
    TCase *tc;

    tc = tcase_create ("Test filter parsing simple.");
    tcase_add_test (tc, test_filter_parsing_simple);
    tcase_add_unchecked_fixture (tc, setup, teardown);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test filter parsing complex.");
    tcase_add_unchecked_fixture (tc, setup, teardown);
    tcase_add_test (tc, test_filter_parsing_complex);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test filter parsing invalid white space.");
    tcase_add_unchecked_fixture (tc, setup, teardown);
    tcase_add_test (tc, test_filter_parsing_inv_white_space);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test filter invalid type.");
    tcase_add_unchecked_fixture (tc, setup, teardown);
    tcase_add_test (tc, test_filter_invalid_type);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test filter missing \".");
    tcase_add_unchecked_fixture (tc, setup, teardown);
    tcase_add_test (tc, test_filter_missing_quote);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test case filter");
    tcase_add_test (tc, test_test_case_filter);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Requirement filter");
    tcase_add_test (tc, test_requirement_filter);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test set filter");
    tcase_add_test (tc, test_test_set_filter);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test feature filter");
    tcase_add_test (tc, test_feature_filter);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test type filter");
    tcase_add_test (tc, test_type_filter);
    suite_add_tcase (s, tc);

    tc = tcase_create ("ACCEPTANCE: Test case filter");
    tcase_add_test (tc, acceptance_test_case_filter);
    suite_add_tcase (s, tc);

    tc = tcase_create ("ACCEPTANCE: Test set filter");
    tcase_add_test (tc, acceptance_test_set_filter);
    suite_add_tcase (s, tc);

    tc = tcase_create ("ACCEPTANCE: Test requirement filter");
    tcase_add_test (tc, acceptance_test_requirement_filter);
    suite_add_tcase (s, tc);

    tc = tcase_create ("ACCEPTANCE: Test feature filter");
    tcase_add_test (tc, acceptance_test_feature_filter);
    suite_add_tcase (s, tc);

    tc = tcase_create ("ACCEPTANCE: Test type filter");
    tcase_add_test (tc, acceptance_test_type_filter);
    suite_add_tcase (s, tc);

    tc = tcase_create ("ACCEPTANCE: Test filter combo");
    tcase_add_test (tc, acceptance_test_filter_combo);
    suite_add_tcase (s, tc);
    
    return s;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
