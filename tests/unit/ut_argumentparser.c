/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Contains changes by Wind River Systems, 2011-03-09
 *
 * Contact: Riku Halonen <riku.halonen@nokia.com>
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
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#define TEST_CMD_LEN 1024

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
START_TEST (test_parse_cmd_line_arguments) {

    /* Test parsing command line arguments. */
    int ret;
    char cmd[TEST_CMD_LEN];
    char *out_file = "/tmp/testrunner-lite-tests/testrunner-lite.out.xml";
    
    /* Test -f and -o flag. */
    snprintf (cmd, TEST_CMD_LEN, "%s -a -f %s -o %s", TESTRUNNERLITE_BIN, 
	     TESTDATA_VALID_XML_1,  out_file);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    snprintf (cmd, TEST_CMD_LEN, "stat %s", out_file);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    /* Test -A  */
    snprintf (cmd, TEST_CMD_LEN,
	      "%s -a -f %s -A", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    /* Test -S  */
    snprintf (cmd, TEST_CMD_LEN, "%s -a -f %s -A -S", TESTRUNNERLITE_BIN, 
	     TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    /* Test -A -s flag. */
    snprintf (cmd, TEST_CMD_LEN, "%s -a -f %s -A --semantic", 
	      TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    /* Test -E and -G */
    snprintf (cmd, TEST_CMD_LEN, "%s -a -f %s -o %s -E true -G true",
             TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1, out_file);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    /* Test -r text */
    snprintf (cmd, TEST_CMD_LEN, "%s -a -f %s -o %s -r text", 
	      TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1, out_file);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    /* Test -V */
    snprintf (cmd, TEST_CMD_LEN, "%s -V", TESTRUNNERLITE_BIN);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

} END_TEST

START_TEST (test_parse_cmd_line_invalid_arguments) {

    /* Test parsing command line arguments. */
    int ret;
    char cmd[TEST_CMD_LEN];
    char *out_file = "/tmp/out.xml";

    /* Test -f flag without argument. */
    snprintf (cmd, TEST_CMD_LEN, "%s -f", TESTRUNNERLITE_BIN);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    memset (cmd, 0, sizeof(cmd));
    
    /* Test invalid flag. */
    snprintf (cmd, TEST_CMD_LEN,
	      "%s -x %s", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    memset (cmd, 0, sizeof(cmd));

    /* Test -o flag only. */
    snprintf (cmd, TEST_CMD_LEN, "%s -o %s", TESTRUNNERLITE_BIN, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    /* Test -a and -m both. */
    snprintf (cmd, TEST_CMD_LEN, "%s -f  %s -a -m", TESTRUNNERLITE_BIN, 
	      TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    /* Test invalid -r. */
    snprintf (cmd, TEST_CMD_LEN, "%s -f %s -o %s -r foo", TESTRUNNERLITE_BIN, 
	     TESTDATA_VALID_XML_1, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    /* Test invalid -e without argument. */
    snprintf (cmd, TEST_CMD_LEN, "%s -f %s -o %s -e", TESTRUNNERLITE_BIN, 
	     TESTDATA_VALID_XML_1, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    /* Test -C with mutually exclusive -t. */
    snprintf (cmd, TEST_CMD_LEN, "%s -f %s -o %s -C /tmp -t localhost",
             TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    /* Test -C with mutually exclusive -E/-G. */
    snprintf (cmd, TEST_CMD_LEN, "%s -f %s -o %s -C /tmp -E true -G true",
	      TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    /* Test -t with mutually exclusive -E/-G. */
    snprintf (cmd, TEST_CMD_LEN, "%s -f %s -o %s -t localhost -E true -G true",
             TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    /* Test -E without -G. */
    snprintf (cmd, TEST_CMD_LEN, "%s -f %s -o %s -E true",
	      TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    /* Test -G without -E. */
    snprintf (cmd, TEST_CMD_LEN, "%s -f %s -o %s -G true",
	      TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

} END_TEST

START_TEST (test_semantic_and_validate_only_flags) {

    /* Test parsing command line arguments. */
    int ret;
    char cmd[TEST_CMD_LEN];

    /* Test invalid semantics with basic xsd  */
    snprintf (cmd, TEST_CMD_LEN, "%s -A -f %s ", TESTRUNNERLITE_BIN, 
	     TESTDATA_INVALID_SEMANTIC_XML_1);
    ret = system (cmd);
    fail_if (ret, cmd);

    /* Test invalid semantics with stricter xsd  */
    snprintf (cmd, TEST_CMD_LEN, "%s -s -A -f %s ", TESTRUNNERLITE_BIN, 
	     TESTDATA_INVALID_SEMANTIC_XML_1);
    ret = system (cmd);
    fail_unless (ret, cmd);

    /* Test invalid semantics with stricter xsd, but disable with -c  */
    snprintf (cmd, TEST_CMD_LEN, "%s -c -s -A -f %s ", TESTRUNNERLITE_BIN, 
	     TESTDATA_INVALID_SEMANTIC_XML_1);
    ret = system (cmd);
    fail_if (ret, cmd);

} END_TEST

START_TEST (test_verbosity_flags) {

    /* Test parsing verbosity arguments. */
    int ret;
    char cmd[TEST_CMD_LEN];
    char *stdout_tmp = "/tmp/testrunner-lite-stdout.log";

    /* Test -v flag. */
    snprintf (cmd, TEST_CMD_LEN, "%s -A -f %s -v > %s", TESTRUNNERLITE_BIN, 
	     TESTDATA_VALID_XML_1, stdout_tmp );
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    
    snprintf (cmd, TEST_CMD_LEN, 
	      "/bin/grep '[INFO]* Verbosity level set to: 1' %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, "cmd %s, RET=%d", cmd, ret);
    
    /* Test -vv flag. */
    snprintf (cmd, TEST_CMD_LEN, "%s -A -f %s -vv > %s", 
	     TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1, stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    snprintf (cmd, TEST_CMD_LEN,
	      "grep \"[INFO]* Verbosity level set to: 2\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Test --verbose=INFO flag. */
    snprintf (cmd, TEST_CMD_LEN, "%s -A -f %s --verbose=INFO > %s", 
	     TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1, stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    snprintf (cmd, TEST_CMD_LEN,
	      "grep \"[INFO]* Verbosity level set to: 1\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Test --verbose=DEBUG flag. */
    snprintf (cmd, TEST_CMD_LEN, "%s -A -f %s --verbose=DEBUG > %s", 
	     TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1, stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    snprintf (cmd, TEST_CMD_LEN,
	      "grep \"[INFO]* Verbosity level set to: 2\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    
    /* Test without -v or --verbose= flag. */
    snprintf (cmd, TEST_CMD_LEN, "%s -A -f %s > %s", TESTRUNNERLITE_BIN, 
	      TESTDATA_VALID_XML_1, stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    snprintf (cmd, TEST_CMD_LEN,
	      "grep \"[INFO]* Verbosity level set to:*\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret == 0, cmd);
    
    /* Test with invalid --verbose= flag. */
    snprintf (cmd, TEST_CMD_LEN,
	      "%s -A -f %s --verbose=FOO > %s", TESTRUNNERLITE_BIN, 
	      TESTDATA_VALID_XML_1, stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    snprintf (cmd, TEST_CMD_LEN,
	     "grep \"[INFO]* Verbosity level set to:*\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret == 0, cmd);
    
} END_TEST

START_TEST (test_remote_logger_flag) {

    int ret;
    char cmd[TEST_CMD_LEN];
    char *out_file = "/tmp/out.xml";

    /* Test -L without required argument */
    snprintf (cmd, TEST_CMD_LEN, "%s -f %s -o %s -L", TESTRUNNERLITE_BIN, 
	     TESTDATA_SIMPLE_XML_1, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    /* Test --logger without required argument */
    snprintf (cmd, TEST_CMD_LEN, "%s -f %s -o %s --logger", TESTRUNNERLITE_BIN, 
	     TESTDATA_SIMPLE_XML_1, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

} END_TEST

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
Suite *make_argumentparser_suite (void)
{
    /* Create suite. */
    Suite *s = suite_create ("argumentparser");

    /* Create test cases and add to suite. */
    TCase *tc;

    tc = tcase_create ("Test parsing cmd line arguments.");
    tcase_set_timeout(tc, 600);
    tcase_add_test (tc, test_parse_cmd_line_arguments);
    
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test parsing invalid cmd line arguments.");
    tcase_add_test (tc, test_parse_cmd_line_invalid_arguments);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test that -A and -s flags work correctly.");
    tcase_add_test (tc, test_semantic_and_validate_only_flags);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test verbosity flags.");
    tcase_add_test (tc, test_verbosity_flags);
    suite_add_tcase (s, tc);
    
    tc = tcase_create ("Test remote logger flag.");
    tcase_add_test (tc, test_remote_logger_flag);
    suite_add_tcase (s, tc);
    
    return s;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
