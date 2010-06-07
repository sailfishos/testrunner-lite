/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
START_TEST (test_parse_cmd_line_arguments)

    /* Test parsing command line arguments. */
    int ret;
    char cmd[1024];
    char *out_file = "/tmp/testrunner-lite-tests/testrunner-lite.out.xml";
    
    /* Test -f and -o flag. */
    sprintf (cmd, "%s -a -f %s -o %s", TESTRUNNERLITE_BIN, 
	     TESTDATA_VALID_XML_1,  out_file);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    sprintf (cmd, "stat %s", out_file);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    /* Test -A  */
    sprintf (cmd, "%s -a -f %s -A", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    /* Test -S  */
    sprintf (cmd, "%s -a -f %s -A -S", TESTRUNNERLITE_BIN, 
	     TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    /* Test -A -s flag. */
    sprintf (cmd, "%s -a -f %s -A --semantic", TESTRUNNERLITE_BIN, 
	     TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    /* Test -r text */
    sprintf (cmd, "%s -a -f %s -o %s -r text", TESTRUNNERLITE_BIN, 
	     TESTDATA_VALID_XML_1, out_file);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
END_TEST

START_TEST (test_parse_cmd_line_invalid_arguments)

    /* Test parsing command line arguments. */
    int ret;
    char cmd[128];
    char *out_file = "/tmp/out.xml";

    /* Test -f flag without argument. */
    sprintf (cmd, "%s -f", TESTRUNNERLITE_BIN);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    memset (cmd, 0, sizeof(cmd));
    
    /* Test invalid flag. */
    sprintf (cmd, "%s -x %s", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    memset (cmd, 0, sizeof(cmd));

    /* Test -o flag only. */
    sprintf (cmd, "%s -o %s", TESTRUNNERLITE_BIN, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    /* Test -a and -m both. */
    sprintf (cmd, "%s -f  %s -a -m", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    /* Test invalid -r. */
    sprintf (cmd, "%s -f %s -o %s -r foo", TESTRUNNERLITE_BIN, 
	     TESTDATA_VALID_XML_1, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    /* Test invalid -e without argument. */
    sprintf (cmd, "%s -f %s -o %s -e", TESTRUNNERLITE_BIN, 
	     TESTDATA_VALID_XML_1, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

END_TEST

START_TEST (test_semantic_and_validate_only_flags)

    /* Test parsing command line arguments. */
    int ret;
    char cmd[1024];

    /* Test invalid semantics with basic xsd  */
    sprintf (cmd, "%s -A -f %s ", TESTRUNNERLITE_BIN, 
	     TESTDATA_INVALID_SEMANTIC_XML_1);
    ret = system (cmd);
    fail_if (ret, cmd);

    /* Test invalid semantics with stricter xsd  */
    sprintf (cmd, "%s -s -A -f %s ", TESTRUNNERLITE_BIN, 
	     TESTDATA_INVALID_SEMANTIC_XML_1);
    ret = system (cmd);
    fail_unless (ret, cmd);

    /* Test invalid semantics with stricter xsd, but disable with -c  */
    sprintf (cmd, "%s -c -s -A -f %s ", TESTRUNNERLITE_BIN, 
	     TESTDATA_INVALID_SEMANTIC_XML_1);
    ret = system (cmd);
    fail_if (ret, cmd);

END_TEST

START_TEST (test_verbosity_flags)

    /* Test parsing verbosity arguments. */
    int ret;
    char cmd[1024];
    char *stdout_tmp = "/tmp/testrunner-lite-stdout.log";
    FILE *fp;
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* Test -v flag. */
    sprintf (cmd, "%s -A -f %s -v", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);
    
    sprintf (cmd, "grep \"[INFO]* Verbosity level set to: 1\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* Test -vv flag. */
    sprintf (cmd, "%s -A -f %s -vv", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);
    
    sprintf (cmd, "grep \"[INFO]* Verbosity level set to: 2\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* Test --verbose=INFO flag. */
    sprintf (cmd, "%s -A -f %s --verbose=INFO", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);
    
    sprintf (cmd, "grep \"[INFO]* Verbosity level set to: 1\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* Test --verbose=DEBUG flag. */
    sprintf (cmd, "%s -A -f %s --verbose=DEBUG", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);
    
    sprintf (cmd, "grep \"[INFO]* Verbosity level set to: 2\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* Test without -v or --verbose= flag. */
    sprintf (cmd, "%s -A -f %s", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);
    
    sprintf (cmd, "grep \"[INFO]* Verbosity level set to:*\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret == 0, cmd);
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* Test with invalid --verbose= flag. */
    sprintf (cmd, "%s -A -f %s --verbose=FOO", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);
    
    sprintf (cmd, "grep \"[INFO]* Verbosity level set to:*\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret == 0, cmd);
    
END_TEST

START_TEST (test_remote_logger_flag)
    int ret;
    char cmd[128];
    char *out_file = "/tmp/out.xml";

    /* Test -L without required argument */
    sprintf (cmd, "%s -f %s -o %s -L", TESTRUNNERLITE_BIN, 
	     TESTDATA_SIMPLE_XML_1, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

    /* Test --logger without required argument */
    sprintf (cmd, "%s -f %s -o %s --logger", TESTRUNNERLITE_BIN, 
	     TESTDATA_SIMPLE_XML_1, out_file);
    ret = system (cmd);
    fail_unless (ret != 0, cmd);

END_TEST

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
