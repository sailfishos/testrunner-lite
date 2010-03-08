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
    sprintf (cmd, "%s -f %s -o %s", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1, 
	     out_file);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    sprintf (cmd, "stat %s", out_file);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    /* Test -A  */
    sprintf (cmd, "%s -f %s -A", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    /* Test -A -s flag. */
    sprintf (cmd, "%s -f %s -A --semantic", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    /* Test -r text */
    sprintf (cmd, "%s -f %s -o %s -r text", TESTRUNNERLITE_BIN, TESTDATA_VALID_XML_1, out_file);
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

    
    return s;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
