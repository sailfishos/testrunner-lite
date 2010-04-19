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
#include <signal.h>
#include <unistd.h>

#include "testdefinitionparser.h"
#include "testdefinitiondatatypes.h"
#include "testrunnerlite.h"
#include "testrunnerlitetestscommon.h"
#include "executor.h"

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
/* None */

/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
static int set_env_for_remote_tests() 
{
	int ret;

	ret = system ("stat ~/.ssh/myrsakey.pub");
	if (ret) { 
		ret = system ("ssh-keygen -N \"\" -f ~/.ssh/myrsakey");
		if (ret)
			goto err_out;
	}
	ret = system ("grep -f ~/.ssh/myrsakey.pub "
		      "~/.ssh/authorized_keys");
	if (ret) {
		ret = system ("cat ~/.ssh/myrsakey.pub "
			      ">> ~/.ssh/authorized_keys");
		if (ret)
			goto err_out;
	}
	    
	ret = system ("grep myrsakey ~/.ssh/config");
	if (ret) {
		ret = system ("echo \"IdentityFile=%d/.ssh/myrsakey\" >> "
			      "~/.ssh/config");
		if (ret)
			goto err_out;
	}

	return 0;
 err_out:
	fprintf (stderr, "failed to set env for remote testsing\n");
	return 1;
 }	

START_TEST (test_executor_null_command)
	exec_data edata;

	
	init_exec_data(&edata);
	edata.soft_timeout = 10;
	edata.hard_timeout = COMMON_HARD_TIMEOUT;
	fail_if (execute(NULL, &edata));

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_0_timeout)
	exec_data edata;

	
	init_exec_data(&edata);
	edata.soft_timeout = 0;
	edata.hard_timeout = 0;
	fail_if (execute(NULL, &edata));

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_stdout)
	exec_data edata;

	
	init_exec_data(&edata);
	edata.soft_timeout = 0;
	edata.hard_timeout = 0;
	fail_if (execute("pwd", &edata));
	fail_if (strlen ((char *)edata.stdout_data.buffer) == 0);
	fail_unless (strlen ((char *)edata.stderr_data.buffer) == 0);

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_stderr)
	exec_data edata;

	
	init_exec_data(&edata);
	edata.soft_timeout = 0;
	edata.hard_timeout = 0;
	fail_if (execute("cat can_of_food", &edata));
	fail_if (strlen ((char *)edata.stderr_data.buffer) == 0);
	fail_unless (strlen ((char *)edata.stdout_data.buffer) == 0);
	
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_long_input_streams)
	exec_data edata;
	
	init_exec_data(&edata);
	edata.soft_timeout = 2;
	edata.hard_timeout = 1;
	fail_if (execute("/usr/share/testrunner-lite-tests/long_output.sh", 
			 &edata));
	fail_unless (edata.result == 0);
	fail_if (edata.stdout_data.buffer == NULL);
	fail_if (edata.stderr_data.buffer == NULL);
	fail_unless (edata.stdout_data.length == 4380);
	fail_unless (edata.stderr_data.length == 2190);
	fail_unless (strlen ((char *)edata.stdout_data.buffer) == 4380);
	fail_unless (strlen ((char *)edata.stderr_data.buffer) == 2190);
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_terminating_process)
	exec_data edata;
	
	init_exec_data(&edata);
	edata.soft_timeout = 1;
	edata.hard_timeout = 1;
	fail_if (execute("/usr/lib/testrunner-lite-tests/terminating " 
			 "stdouttest stderrtest", &edata));
	fail_unless (edata.result == SIGTERM);
	fail_if (edata.stdout_data.buffer == NULL);
	fail_if (edata.stderr_data.buffer == NULL);
	fail_unless (strcmp((char*)edata.stdout_data.buffer, 
			    "stdouttest") == 0);
	fail_unless (strcmp((char*)edata.stderr_data.buffer, 
			    "stderrtest") == 0);
	fail_if(edata.failure_info.buffer == NULL);
	fail_unless (strcmp((char*)edata.failure_info.buffer, 
			    FAILURE_INFO_TIMEOUT) == 0);
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_killing_process)
	exec_data edata;
	
	init_exec_data(&edata);
	edata.soft_timeout = 1;
	edata.hard_timeout = 1;
	fail_if (execute("/usr/lib/testrunner-lite-tests/unterminating "
			 "stdouttest stderrtest", &edata));
	fail_unless (edata.result == SIGTERM || edata.result == SIGKILL);
	fail_if (edata.stdout_data.buffer == NULL);
	fail_if (edata.stderr_data.buffer == NULL);
	fail_unless (strcmp((char*)edata.stdout_data.buffer, 
			    "stdouttest") == 0);
	fail_unless (strcmp((char*)edata.stderr_data.buffer, 
			    "stderrtest") == 0);
	fail_if(edata.failure_info.buffer == NULL);
	fail_unless (strcmp((char*)edata.failure_info.buffer, 
			    FAILURE_INFO_TIMEOUT) == 0);
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_piped_command)
	exec_data edata;
	init_exec_data(&edata);
	edata.soft_timeout = 1;
	edata.hard_timeout = 1;
	fail_if (execute("echo h world | sed -e 's/h/hello/g' | grep hello", 
			 &edata));
	fail_unless (edata.result == 0);
	fail_if (edata.stdout_data.buffer == NULL);
	fail_unless (edata.stdout_data.length == strlen("hello world\n"));
	fail_unless (strcmp((char*)edata.stdout_data.buffer, 
			    "hello world\n") == 0);
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_without_output_redirection)
	exec_data edata;
	init_exec_data(&edata);
	edata.soft_timeout = 1;
	edata.hard_timeout = 1;
	edata.redirect_output = DONT_REDIRECT_OUTPUT;
	fail_if (execute("echo testing", &edata));
	fail_unless (edata.result == 0);
	fail_unless (edata.stdout_data.length == 0);
	fail_unless (edata.stderr_data.length == 0);
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_exec_data_handling)
	exec_data edata;
	init_exec_data(&edata);
	fail_if (execute("echo testing", &edata));
	fail_unless (edata.result == 0);
	fail_unless (edata.stdout_data.length == strlen("testing\n"));
	fail_unless (strcmp((char*)edata.stdout_data.buffer, "testing\n") == 0);

	clean_exec_data(&edata);
	fail_unless (edata.stdout_data.buffer == NULL);
	fail_unless (edata.stderr_data.buffer == NULL);

	init_exec_data(&edata);
	fail_if (execute("cat unexisting_foobar_file", &edata));
	fail_if (edata.result == 0);
	fail_if (edata.stderr_data.length == 0);
	fail_unless (strlen((char*)edata.stderr_data.buffer) > 0);

	clean_exec_data(&edata);
	fail_unless (edata.stdout_data.buffer == NULL);
	fail_unless (edata.stderr_data.buffer == NULL);
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_command)
	exec_data edata;
	testrunner_lite_options opts;

	opts.target_address = "localhost";
	executor_init (&opts);
	init_exec_data (&edata);
	

	fail_if (execute("echo testing", &edata));
	fail_unless (edata.result == 0);
	fail_unless (edata.stdout_data.length == strlen("testing\n"));
	fail_unless (strcmp((char*)edata.stdout_data.buffer, "testing\n") == 0);

	clean_exec_data(&edata);
	fail_unless (edata.stdout_data.buffer == NULL);
	fail_unless (edata.stderr_data.buffer == NULL);

	init_exec_data(&edata);
	fail_if (execute("cat unexisting_foobar_file", &edata));
	fail_if (edata.result == 0);
	fail_if (edata.stderr_data.length == 0);
	fail_unless (strlen((char*)edata.stderr_data.buffer) > 0);

	clean_exec_data(&edata);
	fail_unless (edata.stdout_data.buffer == NULL);
	fail_unless (edata.stderr_data.buffer == NULL);
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_terminating_process)
	exec_data edata;
	testrunner_lite_options opts;

	opts.target_address = "localhost";
	executor_init (&opts);
	
	init_exec_data(&edata);
	edata.soft_timeout = 2;
	edata.hard_timeout = 1;
	fail_if (execute("/usr/lib/testrunner-lite-tests/terminating " 
			 "stdouttest stderrtest", &edata));
	fail_unless (edata.result == 143); /* 128 + SIGKILL */
	fail_if (edata.stdout_data.buffer == NULL);
	fail_if (edata.stderr_data.buffer == NULL);
	fail_unless (strcmp((char*)edata.stdout_data.buffer, 
			    "stdouttest") == 0);
	fail_unless (strncmp((char*)edata.stderr_data.buffer, 
			     "stderrtest", strlen ("stderrtest")) == 0);
	/* sleep for a while such that remote killing has done its job */
	sleep(2);
	/* check that killing was succesfull */
	fail_if (execute("pidof terminating", &edata));
	fail_unless (edata.result == 1);

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_killing_process)
	exec_data edata;
	testrunner_lite_options opts;

	opts.target_address = "localhost";
	executor_init (&opts);

	init_exec_data(&edata);
	edata.soft_timeout = 2;
	edata.hard_timeout = 1;
	fail_if (execute("/usr/lib/testrunner-lite-tests/unterminating "
			 "stdouttest stderrtest", &edata));
	fail_unless (edata.result == 143); /* 128 + SIGKILL */
	fail_if (edata.stdout_data.buffer == NULL);
	fail_if (edata.stderr_data.buffer == NULL);
	fail_unless (strncmp((char*)edata.stdout_data.buffer, 
			     "stdouttest", strlen("stderrtest")) == 0);
	fail_unless (strncmp((char*)edata.stderr_data.buffer, 
			     "stderrtest", strlen("stderrtest")) == 0);
	/* sleep for a while such that remote killing has done its job */
	sleep(2);
	fail_if (execute("pidof unterminating", &edata));
	fail_unless (edata.result == 1);

END_TEST

START_TEST (test_remote_get)

    int ret;
    char cmd[1024];

    sprintf (cmd, "%s -f %s -o /tmp/testrunnerlitetestdir2/res.xml "
	     "-t localhost", 
	     TESTRUNNERLITE_BIN, 
	     TESTDATA_GET_XML_1);
    ret = system (cmd);
    fail_if (ret, cmd);

    sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/");
    fail_if (ret, cmd);
    
    sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest.txt");
    fail_if (ret, cmd);
    printf ("%s: remote get /tmp/gettest.txt worked\n",
	    __FUNCTION__); 
    
    sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest2.txt");
    fail_if (ret, cmd);
    printf ("%s: remote get /tmp/gettest2.txt worked\n",
	    __FUNCTION__); 

END_TEST

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
Suite *make_testexecutor_suite (void)
{
    /* Create suite. */
    Suite *s = suite_create ("testexecuter");

    /* Create test cases and add to suite. */
    TCase *tc;

    tc = tcase_create ("Test executor with null command.");
    tcase_add_test (tc, test_executor_null_command);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor with 0 timeout.");
    tcase_add_test (tc, test_executor_0_timeout);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor stdout output.");
    tcase_add_test (tc, test_executor_stdout);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor stderr output.");
    tcase_add_test (tc, test_executor_stderr);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor long input streams.");
    tcase_set_timeout (tc, 5);
    tcase_add_test (tc, test_executor_long_input_streams);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor terminating process.");
    tcase_set_timeout (tc, 5);
    tcase_add_test (tc, test_executor_terminating_process);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor killing process.");
    tcase_set_timeout (tc, 5);
    tcase_add_test (tc, test_executor_killing_process);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor piped command.");
    tcase_add_test (tc, test_executor_piped_command);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor without output redirection.");
    tcase_add_test (tc, test_executor_without_output_redirection);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor execution data handling.");
    tcase_add_test (tc, test_executor_exec_data_handling);
    suite_add_tcase (s, tc);

    if (set_env_for_remote_tests()) {
	    fprintf (stderr, "skipping remote tests\n");
	    return s;
    }
    tc = tcase_create ("Test executor remote command.");
    tcase_add_test (tc, test_executor_remote_command);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor remote terminating process.");
    tcase_set_timeout (tc, 10);
    tcase_add_test (tc, test_executor_remote_terminating_process);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor remote killing process.");
    tcase_set_timeout (tc, 10);
    tcase_add_test (tc, test_executor_remote_killing_process);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test get feature with remote host.");
    tcase_set_timeout (tc, 10);
    tcase_add_test (tc, test_remote_get);
    suite_add_tcase (s, tc);

    
    return s;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
