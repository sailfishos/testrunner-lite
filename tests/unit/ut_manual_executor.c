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
#define TEST_CMD_LEN 1024
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
    FILE *fp, *fp2;
    int ret;
    char cmd[TEST_CMD_LEN];
    char *stdout_tmp = "/tmp/testrunner-lite-manual-exec-stdout.log";
    int pipefd[2], written;
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);

    /* redirect stdin from a pipe */
    ret = pipe(pipefd);
    fail_if (ret);
    close(0);
    ret = dup(pipefd[0]);
    fail_if (ret < 0);

    t_case = td_case_create();
    t_case->gen.description = (xmlChar*)"This is manual test case.";

    /* Execute print information. */
    pre_manual (t_case);
    
    /* Back to terminal. */
    fp2 = freopen ("/dev/tty", "w", stdout);
    snprintf (cmd, TEST_CMD_LEN, "grep -q \"This is manual test case.\" %s", 
	      stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* Execute manual step. */
    t_step = td_step_create();
    t_step->step = (xmlChar*)"This is manual test step.";
    
    /* Make test case pass. */
    written = write(pipefd[1], "P\n", 2);
    fail_if (written < 2);
    execute_manual (t_step);
    
    /* Back to terminal. */
    fp2 = freopen ("/dev/tty", "w", stdout);

    snprintf (cmd, TEST_CMD_LEN, "grep -q \"This is manual test step.\" %s", 
	      stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    t_case->case_res = CASE_PASS;
    fp = freopen (stdout_tmp, "w", stdout);
    
    written = write(pipefd[1], "\n", 1);
    fail_if (written < 0);
    post_manual (t_case);
    
    /* Back to terminal. */
    fp2 = freopen ("/dev/tty", "w", stdout);

    snprintf (cmd, TEST_CMD_LEN, "grep -q \"PASSED.\" %s", stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    close(pipefd[0]);
    close(pipefd[1]);
    
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_execute_manual_step_failed)

    td_case *t_case = NULL;
    td_step *t_step = NULL;
    FILE *fp, *fp2;
    int ret;
    char cmd[TEST_CMD_LEN];
    char *stdout_tmp = "/tmp/testrunner-lite-manual-exec-stdout.log";
    int pipefd[2], written;
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* redirect stdin from a pipe */
    ret = pipe(pipefd);
    fail_if (ret < 0);
    close(0);
    ret = dup(pipefd[0]);
    fail_if (ret < 0);

    t_case = td_case_create();
    t_case->gen.description = (xmlChar*)"This is manual test case.";

    /* Execute print information. */
    pre_manual (t_case);
    
    /* Back to terminal. */
    fp2 = freopen ("/dev/tty", "w", stdout);
    snprintf (cmd, TEST_CMD_LEN, "grep -q \"This is manual test case.\" %s", 
	      stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* Execute manual step. */
    t_step = td_step_create();
    t_step->step = (xmlChar*)"This is manual test step.";
    
    /* Make test case fail. */
    written = write(pipefd[1], "F\n", 2);
    fail_if (written < 2);
    execute_manual (t_step);
    
    /* Back to terminal. */
    fp2 = freopen ("/dev/tty", "w", stdout);

    snprintf (cmd, TEST_CMD_LEN, "grep -q \"This is manual test step.\" %s", 
	      stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    t_case->case_res = CASE_FAIL;
    fp = freopen (stdout_tmp, "w", stdout);
    
    written = write(pipefd[1], "\n", 1);
    fail_if (written < 1);
    post_manual (t_case);
    
    /* Back to terminal. */
    fp2 = freopen ("/dev/tty", "w", stdout);

    snprintf (cmd, TEST_CMD_LEN, "grep -q \"FAILED.\" %s", stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    close(pipefd[0]);
    close(pipefd[1]);
    
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_execute_manual_step_na)

    td_case *t_case = NULL;
    td_step *t_step = NULL;
    FILE *fp, *fp2;
    int ret;
    char cmd[TEST_CMD_LEN];
    char *stdout_tmp = "/tmp/testrunner-lite-manual-exec-stdout.log";
    int pipefd[2], written;
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* redirect stdin from a pipe */
    ret = pipe(pipefd);
    fail_if (ret);
    close(0);
    ret = dup(pipefd[0]);
    fail_if (ret < 0);

    t_case = td_case_create();
    t_case->gen.description = (xmlChar*)"This is manual test case.";

    /* Execute print information. */
    pre_manual (t_case);
    
    /* Back to terminal. */
    fp2 = freopen ("/dev/tty", "w", stdout);
    snprintf (cmd, TEST_CMD_LEN, "grep -q \"This is manual test case.\" %s", 
	      stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* Execute manual step. */
    t_step = td_step_create();
    t_step->step = (xmlChar*)"This is manual test step.";
    
    /* Make test case N/A. */
    written = write(pipefd[1], "N\n", 2);
    fail_if (written < 2);
    execute_manual (t_step);
    
    /* Back to terminal. */
    fp2 = freopen ("/dev/tty", "w", stdout);

    snprintf (cmd, TEST_CMD_LEN,"grep -q \"This is manual test step.\" %s", 
	      stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    t_case->case_res = CASE_NA;
    fp = freopen (stdout_tmp, "w", stdout);
    
    written = write(pipefd[1], "\n", 1);
    fail_if (written < 1);
    post_manual (t_case);
    
    /* Back to terminal. */
    fp2 = freopen ("/dev/tty", "w", stdout);

    snprintf (cmd, TEST_CMD_LEN, "grep -q \"N/A.\" %s", stdout_tmp);
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    close(pipefd[0]);
    close(pipefd[1]);
    
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_execute_manual_set)
     int ret;
     FILE *f;

     f = popen (TESTRUNNERLITE_BIN 
		" -f "
		TESTDATA_MANUAL_SET
		" -o /tmp/res.xml", "w");
     fail_if (f == NULL, "popen() failed");
     ret = fwrite ("P\nP\ntestcomment\n", 1, strlen("P\nP\ntestcomment\n"), f);
     fail_if (ret != strlen("P\nP\ntestcomment\n"), 
	      "fwrite() returned : %d", ret);
     fclose (f);
     
     ret = system ("grep -q PASS /tmp/res.xml");
     fail_if (ret, "/tmp/res.xml does not contain PASS");

     ret = system ("grep -q FAIL /tmp/res.xml");
     fail_unless (ret, "/tmp/res.xml contains FAIL");

     ret = system ("grep -q testcomment /tmp/res.xml");
     fail_if (ret, "comment not found from result");

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_execute_semi_auto)
     int ret;
     FILE *f;

     f = popen (TESTRUNNERLITE_BIN
		" -f "
		TESTDATA_SEMI_AUTO
		" -o /tmp/res.xml", "w");
     fail_if (f == NULL, "popen() failed");
     ret = fwrite ("P\n\n", 1, 5, f);
     fail_if (ret != 5, "fwrite() returned : %d", ret);
     fclose (f);
     
     ret = system ("grep -q PASS /tmp/res.xml");
     fail_if (ret, "/tmp/res.xml does not contain PASS");

     ret = system ("grep -q FAIL /tmp/res.xml");
     fail_unless (ret, "/tmp/res.xml contains FAIL");

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_execute_manual_case_no_steps)
     int ret;
     FILE *f;

     f = popen (TESTRUNNERLITE_BIN
		" -f "
		TESTDATA_MANUAL_NO_STEPS
		" -o /tmp/res.xml -v", "w");
     fail_if (f == NULL, "popen() failed");
     ret = fwrite ("F\n\n", 1, 5, f);
     fail_if (ret != 5, "fwrite() returned : %d", ret);
     fclose (f);
     
     ret = system ("grep -q FAIL /tmp/res.xml");
     fail_if (ret, "/tmp/res.xml does not contain PASS");

     ret = system ("grep -q PASS /tmp/res.xml");
     fail_unless (ret, "/tmp/res.xml contains FAIL");
    
END_TEST
START_TEST (test_execute_manual_empty_steps)
     int ret;
     FILE *f;

     f = popen (TESTRUNNERLITE_BIN
		" -f "
		TESTDATA_MANUAL_EMPTY_STEPS
		" -o /tmp/res.xml -v", "w");
     fail_if (f == NULL, "popen() failed");
     sleep (1);
     ret = fwrite ("P\n", 1, 3 , f);
     fail_if (ret != 3, "fwrite() returned : %d", ret);
     sleep (1);
     ret = fwrite ("\nP\n", 1, 4, f);
     fail_if (ret != 4, "fwrite() returned : %d", ret);
     sleep (1);
     ret = fwrite ("\nC\n", 1, 4, f);
     fail_if (ret != 4, "fwrite() returned : %d", ret);
     fclose (f);
     
     ret = system ("grep -q PASS /tmp/res.xml");
     fail_if (ret, "/tmp/res.xml does not contain PASS");

     ret = system ("grep -q FAIL /tmp/res.xml");
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

    tc = tcase_create ("Test executing semi automatic set.");
    tcase_add_test (tc, test_execute_semi_auto);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executing manual case with no steps.");
    tcase_add_test (tc, test_execute_manual_case_no_steps);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executing manual case with empty steps.");
    tcase_add_test (tc, test_execute_manual_empty_steps);
    suite_add_tcase (s, tc);

    return s;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
