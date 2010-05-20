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
#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <string.h>
#include <unistd.h>

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
START_TEST (test_execute_manual_step_passed)

    td_case *t_case = NULL;
    td_step *t_step = NULL;
    FILE *fp;
    int ret;
    char cmd[1024];
    char *stdout_tmp = "/tmp/testrunner-lite-manual-exec-stdout.log";
    int pipefd[2];
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);

    /* redirect stdin from a pipe */
    pipe(pipefd);
    close(0);
    dup(pipefd[0]);
    
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
    write(pipefd[1], "P\n", 2);
    execute_manual (t_step);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);

    sprintf (cmd, "grep \"This is manual test step.\" %s", stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    t_case->case_res = CASE_PASS;
    fp = freopen (stdout_tmp, "w", stdout);
    
    write(pipefd[1], "\n", 1);
    post_manual (t_case);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);

    sprintf (cmd, "grep \"PASSED.\" %s", stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    close(pipefd[0]);
    close(pipefd[1]);
    
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_execute_manual_step_failed)

    td_case *t_case = NULL;
    td_step *t_step = NULL;
    FILE *fp;
    int ret;
    char cmd[1024];
    char *stdout_tmp = "/tmp/testrunner-lite-manual-exec-stdout.log";
    int pipefd[2];
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* redirect stdin from a pipe */
    pipe(pipefd);
    close(0);
    dup(pipefd[0]);

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
    write(pipefd[1], "F\n", 2);
    execute_manual (t_step);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);

    sprintf (cmd, "grep \"This is manual test step.\" %s", stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    t_case->case_res = CASE_FAIL;
    fp = freopen (stdout_tmp, "w", stdout);
    
    write(pipefd[1], "\n", 1);
    post_manual (t_case);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);

    sprintf (cmd, "grep \"FAILED.\" %s", stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    close(pipefd[0]);
    close(pipefd[1]);
    
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_execute_manual_step_na)

    td_case *t_case = NULL;
    td_step *t_step = NULL;
    FILE *fp;
    int ret;
    char cmd[1024];
    char *stdout_tmp = "/tmp/testrunner-lite-manual-exec-stdout.log";
    int pipefd[2];
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* redirect stdin from a pipe */
    pipe(pipefd);
    close(0);
    dup(pipefd[0]);

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
    write(pipefd[1], "N\n", 2);
    execute_manual (t_step);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);

    sprintf (cmd, "grep \"This is manual test step.\" %s", stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    t_case->case_res = CASE_NA;
    fp = freopen (stdout_tmp, "w", stdout);
    
    write(pipefd[1], "\n", 1);
    post_manual (t_case);
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);

    sprintf (cmd, "grep \"N/A.\" %s", stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    close(pipefd[0]);
    close(pipefd[1]);
    
END_TEST
/* ------------------------------------------------------------------------- */

START_TEST (test_execute_manual_set)
     int ret;
     FILE *f;

     f = popen ("testrunner-lite -f /usr/share/testrunner-lite-tests/testdata/"
		 "testrunner-tests-manual-set.xml -o /tmp/res.xml", "w");
     fail_if (f == NULL, "popen() failed");
     ret = fwrite ("P\nP\n\n", 1, 5, f);
     fail_if (ret != 5, "fwrite() returned : %d", ret);
     fclose (f);
     
     ret = system ("grep PASS /tmp/res.xml");
     fail_if (ret, "/tmp/res.xml does not contain PASS");

     ret = system ("grep FAIL /tmp/res.xml");
     fail_unless (ret, "/tmp/res.xml contains FAIL");

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

    tc = tcase_create ("Test executing n/a manual step.");
    tcase_add_test (tc, test_execute_manual_step_na);
    suite_add_tcase (s, tc);


    tc = tcase_create ("Test executing manual set.");
    tcase_add_test (tc, test_execute_manual_set);
    suite_add_tcase (s, tc);

    return s;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
