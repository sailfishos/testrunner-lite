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
#include <stdio.h>
#include <check.h>
#include <string.h>

#include "testrunnerlitetestscommon.h"
#include "testdefinitiondatatypes.h"
#include "manual_executor.h"

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
char *user_input;

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
char *fgets (char *str, int num, FILE *stream)
{ 
    str[0] = user_input;
    return str;
}
/* ------------------------------------------------------------------------- */
START_TEST (test_execute_manual_step_passed)

    td_case *t_case = NULL;
    td_step *t_step = NULL;
    FILE *fp;
    int ret;
    char cmd[1024];
    char *stdout_tmp = "/tmp/testrunner-lite-manual-exec-stdout.log";
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    t_case = td_case_create();
    t_case->gen.description = (xmlChar*)"This is manual test case.";

    /* Execute print information. */
    pre_manual (t_case);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);
    sprintf (cmd, "grep \"This is manual test case.\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* Execute manual step. */
    t_step = td_step_create();
    t_step->step = (xmlChar*)"This is manual test step.";
    
    /* Make test case pass. */
    user_input = 'P';
    execute_manual (t_step);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);

    sprintf (cmd, "grep \"This is manual test step.\" %s", stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    t_case->passed = 1;
    fp = freopen (stdout_tmp, "w", stdout);
    
    user_input = '\n';
    post_manual (t_case);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);

    sprintf (cmd, "grep \"PASSED.\" %s", stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_execute_manual_step_failed)

    td_case *t_case = NULL;
    td_step *t_step = NULL;
    FILE *fp;
    int ret;
    char cmd[1024];
    char *stdout_tmp = "/tmp/testrunner-lite-manual-exec-stdout.log";
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    t_case = td_case_create();
    t_case->gen.description = (xmlChar*)"This is manual test case.";

    /* Execute print information. */
    pre_manual (t_case);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);
    sprintf (cmd, "grep \"This is manual test case.\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* Execute manual step. */
    t_step = td_step_create();
    t_step->step = (xmlChar*)"This is manual test step.";
    
    /* Make test case fail. */
    user_input = 'F';
    execute_manual (t_step);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);

    sprintf (cmd, "grep \"This is manual test step.\" %s", stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    t_case->passed = 0;
    fp = freopen (stdout_tmp, "w", stdout);
    
    user_input = '\n';
    post_manual (t_case);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);

    sprintf (cmd, "grep \"FAILED.\" %s", stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
END_TEST

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
Suite *make_manualtestexecutor_suite (void)
{
    /* Create suite. */
    Suite *s = suite_create ("manual_executor");

    /* Create test cases and add to suite. */
    TCase *tc;

    tc = tcase_create ("Test executing passed manual step.");
    tcase_add_test (tc, test_execute_manual_step_passed);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executing failing manual step.");
    tcase_add_test (tc, test_execute_manual_step_failed);
    suite_add_tcase (s, tc);

    return s;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
