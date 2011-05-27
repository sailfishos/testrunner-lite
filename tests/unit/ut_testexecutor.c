/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Contains changes by Wind River Systems, 2011-03-09
 *
 * Contact: Sami Lahtinen <ext-sami.t.lahtinen@nokia.com>
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
#include <sys/types.h>
#include <sys/wait.h>

#include "testdefinitionparser.h"
#include "testdefinitiondatatypes.h"
#include "testrunnerlite.h"
#include "testrunnerlitetestscommon.h"
#include "remote_executor.h"
#include "executor.h"
#include "log.h"

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
#define LOG_LEVEL 0
#define DEFAULT_REMOTE_EXECUTOR "/usr/bin/ssh -o StrictHostKeyChecking=no " \
		"-o PasswordAuthentication=no localhost"
#define DEFAULT_REMOTE_EXECUTOR_PORT "/usr/bin/ssh -o StrictHostKeyChecking=no " \
		"-o PasswordAuthentication=no -p 22 localhost"
#define DEFAULT_REMOTE_GETTER "/usr/bin/scp localhost:'<FILE>' '<DEST>'"
#define LOCAL_SSH_RULE " -p tcp -d 127.0.0.1 --dport 22 -j DROP"
#define SSH_KEY = "~/.ssh/myrsakey"

/* ------------------------------------------------------------------------- */
/* MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL GLOBAL VARIABLES */
td_suite *suite;
td_set   *set;
char  *suite_description;
static volatile sig_atomic_t sigusr1_flag = 0;
static volatile sig_atomic_t sigchld_flag = 0;
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
	/* Fedora has issues with default permissions */
	ret = system ("chmod og-rwx ~/.ssh/config ~/.ssh/authorized_keys");
	if (ret)
		goto err_out;

	return 0;
 err_out:
	fprintf (stderr, "failed to set env for remote testsing\n");
	return 1;
 }	

START_TEST (test_executor_null_command)
	exec_data edata;
	testrunner_lite_options opts;

	memset (&opts, 0x0, sizeof (opts));
	executor_init (&opts);

	init_exec_data(&edata);
	edata.soft_timeout = 10;
	edata.hard_timeout = COMMON_HARD_TIMEOUT;
	fail_if (execute(NULL, &edata));

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_0_timeout)
	exec_data edata;
	testrunner_lite_options opts;

	memset (&opts, 0x0, sizeof (opts));
	executor_init (&opts);

	init_exec_data(&edata);
	edata.soft_timeout = 0;
	edata.hard_timeout = 0;
	fail_if (execute(NULL, &edata));

END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_stdout)
	exec_data edata;
	testrunner_lite_options opts;

	memset (&opts, 0x0, sizeof (opts));
	executor_init (&opts);
	
	init_exec_data(&edata);
	edata.soft_timeout = 0;
	edata.hard_timeout = 0;
	fail_if (execute("pwd", &edata));
	fail_if (strlen ((char *)edata.stdout_data.buffer) == 0);
	fail_unless (strlen ((char *)edata.stderr_data.buffer) == 0);
	executor_close();
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_stderr)
	exec_data edata;
	testrunner_lite_options opts;

	memset (&opts, 0x0, sizeof (opts));
	executor_init (&opts);
	
	init_exec_data(&edata);
	edata.soft_timeout = 0;
	edata.hard_timeout = 0;
	fail_if (execute("cat can_of_food", &edata));
	fail_if (strlen ((char *)edata.stderr_data.buffer) == 0);
	fail_unless (strlen ((char *)edata.stdout_data.buffer) == 0);
	executor_close();
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_long_input_streams)
	exec_data edata;
	testrunner_lite_options opts;

	memset (&opts, 0x0, sizeof (opts));
	executor_init (&opts);
	
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
	testrunner_lite_options opts;

	memset (&opts, 0x0, sizeof (opts));
	executor_init (&opts);
	
	init_exec_data(&edata);
	edata.soft_timeout = 1;
	edata.hard_timeout = 1;
	fail_if (execute("/usr/lib/testrunner-lite-tests/terminating " 
			 "stdouttest stderrtest", &edata));
	fail_unless (edata.result == SIGTERM);
	fail_if (edata.stdout_data.buffer == NULL);
	fail_if (edata.stderr_data.buffer == NULL);
	fail_unless (strncmp((char*)edata.stdout_data.buffer, 
			     "stdouttest", strlen("stdouttest")) == 0);
	fail_unless (strncmp((char*)edata.stderr_data.buffer, 
			     "stderrtest", strlen("stderrtest")) == 0);
	fail_if(edata.failure_info.buffer == NULL);
	fail_if (strstr((char*)edata.failure_info.buffer, 
			    FAILURE_INFO_TIMEOUT) == NULL);
	executor_close();
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_killing_process)
	exec_data edata;
	testrunner_lite_options opts;

	memset (&opts, 0x0, sizeof (opts));
	executor_init (&opts);
	
	init_exec_data(&edata);
	edata.soft_timeout = 1;
	edata.hard_timeout = 1;
	fail_if (execute("/usr/lib/testrunner-lite-tests/unterminating "
			 "stdouttest stderrtest", &edata));
	fail_unless (edata.result == SIGTERM || edata.result == SIGKILL);
	fail_if (edata.stdout_data.buffer == NULL);
	fail_if (edata.stderr_data.buffer == NULL);
	fail_unless (strncmp((char*)edata.stdout_data.buffer, 
			     "stdouttest", strlen("stdouttest")) == 0);
	fail_unless (strncmp((char*)edata.stderr_data.buffer, 
			     "stderrtest", strlen("stderrtest")) == 0);
	fail_if(edata.failure_info.buffer == NULL);
	fail_if (strstr((char*)edata.failure_info.buffer, 
			    FAILURE_INFO_TIMEOUT) == NULL);
	executor_close();
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_piped_command)
	exec_data edata;
	testrunner_lite_options opts;

	memset (&opts, 0x0, sizeof (opts));
	executor_init (&opts);

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
	executor_close();
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_without_output_redirection)
	exec_data edata;
	testrunner_lite_options opts;

	memset (&opts, 0x0, sizeof (opts));
	executor_init (&opts);

	init_exec_data(&edata);
	edata.soft_timeout = 1;
	edata.hard_timeout = 1;
	edata.redirect_output = DONT_REDIRECT_OUTPUT;
	fail_if (execute("echo testing", &edata));
	fail_unless (edata.result == 0);
	fail_unless (edata.stdout_data.length == 0);
	fail_unless (edata.stderr_data.length == 0);
	executor_close();
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_exec_data_handling)
	exec_data edata;
	testrunner_lite_options opts;

	memset (&opts, 0x0, sizeof (opts));
	executor_init (&opts);

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
	executor_close();
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_command)
	exec_data edata;
	testrunner_lite_options opts;
	opts.target_address = "localhost";
	opts.target_port = 0;
	opts.remote_executor = DEFAULT_REMOTE_EXECUTOR;
	opts.remote_getter = DEFAULT_REMOTE_GETTER;
#ifdef ENABLE_LIBSSH2
	opts.libssh2 = 0;
#endif
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

	/* Give time for either remote_clean or remote_kill called by execute.
	   They have forked new process to do cleanup */
	sleep(1);
	executor_close();
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_command_port)
	exec_data edata;
	testrunner_lite_options opts;
	opts.target_address = "localhost";
	opts.target_port = 22;
	opts.remote_executor = DEFAULT_REMOTE_EXECUTOR_PORT;
	opts.remote_getter = DEFAULT_REMOTE_GETTER;

#ifdef ENABLE_LIBSSH2
	opts.libssh2 = 0;
#endif
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

	/* Give time for either remote_clean or remote_kill called by execute.
	   They have forked new ssh process to do cleanup */
	sleep(1);
	executor_close();
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_long_command)
	exec_data edata;
	testrunner_lite_options opts;
#ifdef ENABLE_LIBSSH2
	opts.libssh2 = 0;
#endif
#define TEST_STRING_SIZE 5000
	char test_string [TEST_STRING_SIZE];
	char command [TEST_STRING_SIZE + 100];
	opts.target_address = "localhost";
	opts.target_port = 0;
	opts.remote_executor = DEFAULT_REMOTE_EXECUTOR;
	opts.remote_getter = DEFAULT_REMOTE_GETTER;

	executor_init (&opts);
	init_exec_data (&edata);
	edata.soft_timeout = 5;
	edata.hard_timeout = 4;

	memset (test_string , 'c', TEST_STRING_SIZE);
	test_string [TEST_STRING_SIZE - 1] = '\0';
	sprintf (command, "echo %s; sleep 2", test_string);
	fail_if (execute(command, &edata));
	fail_unless (edata.result == 0, "result = %d", edata.result);
	fail_unless (edata.stdout_data.length == TEST_STRING_SIZE, 
		     "length %d",
		     edata.stdout_data.length);
	fail_unless (strncmp((char*)edata.stdout_data.buffer, test_string,
			     TEST_STRING_SIZE -1) == 0,
		     edata.stdout_data.buffer);

	/* Give time for either remote_clean or remote_kill called by execute.
	   They have forked new ssh process to do cleanup */
	sleep(1);
	executor_close();
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_terminating_process)
	exec_data edata;
	testrunner_lite_options opts;
	opts.target_address = "localhost";
	opts.target_port = 0;
	opts.remote_executor = DEFAULT_REMOTE_EXECUTOR;
	opts.remote_getter = DEFAULT_REMOTE_GETTER;
#ifdef ENABLE_LIBSSH2
	opts.libssh2 = 0;
#endif
	executor_init (&opts);
	
	init_exec_data(&edata);
	edata.soft_timeout = 2;
	edata.hard_timeout = 1;
	fail_if (execute("/usr/lib/testrunner-lite-tests/terminating " 
			 "stdouttest stderrtest", &edata));
	/* 128 + SIGKILL or 255 depends on ssh*/
	fail_unless (edata.result == 143 ||
		     edata.result == 255, "result=%d", edata.result);
	fail_if (edata.stdout_data.buffer == NULL);
	fail_if (edata.stderr_data.buffer == NULL);
	fail_if (strstr((char*)edata.stdout_data.buffer, "stdouttest") == 0, (char*)edata.stdout_data.buffer);
	fail_if (strstr((char*)edata.stderr_data.buffer, 
			     "stderrtest") == 0,
		     (char*)edata.stderr_data.buffer);
	/* sleep for a while such that remote killing has done its job */
	sleep(2);
	/* check that killing was succesfull */
	fail_if (execute("PATH=$PATH:/sbin/ pidof terminating", &edata));
	fail_unless (edata.result == 1);

	/* Give time for either remote_clean or remote_kill called by execute.
	   They have forked new ssh process to do cleanup */
	sleep(1);
	executor_close();
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_killing_process)
	exec_data edata;
	testrunner_lite_options opts;
	opts.target_address = "localhost";
	opts.target_port = 0;
	opts.remote_executor = DEFAULT_REMOTE_EXECUTOR;
	opts.remote_getter = DEFAULT_REMOTE_GETTER;
#ifdef ENABLE_LIBSSH2
	opts.libssh2 = 0;
#endif
	executor_init (&opts);

	init_exec_data(&edata);
	edata.soft_timeout = 2;
	edata.hard_timeout = 1;
	fail_if (execute("/usr/lib/testrunner-lite-tests/unterminating "
			 "stdouttest stderrtest", &edata));
	/* 128 + SIGKILL or 255 depends on ssh */
	fail_unless (edata.result == 137 ||
		     edata.result == 255, "result %d", edata.result); 
	fail_if (edata.stdout_data.buffer == NULL);
	fail_if (edata.stderr_data.buffer == NULL);
	fail_if (strstr((char*)edata.stdout_data.buffer, 
			     "stdouttest") == 0,
		     edata.stdout_data.buffer);
	fail_if (strstr((char*)edata.stderr_data.buffer, 
			     "stderrtest") == 0,
		     edata.stderr_data.buffer);
	/* sleep for a while such that remote killing has done its job */
	sleep(2);
	int ret = system("pidof unterminating");
	fail_unless (ret);
	executor_close();
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST(test_executor_remote_test_bg_process_cleanup)
     int ret;
     char cmd[1024];
     char *out_file = "/tmp/testrunner-lite-tests/testrunner-lite.out.xml";
     
     sprintf (cmd, "%s -v -f %s -o %s -tlocalhost", TESTRUNNERLITE_BIN, 
	      TESTDATA_BG_XML,  out_file);
     ret = system (cmd);
     fail_if (ret != 0, cmd);
     ret = system("pidof unterminating");
     fail_unless (ret);
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_conn_check)
        int ret = remote_check_conn (DEFAULT_REMOTE_EXECUTOR);
	fail_if (ret, "ret=%d", ret);
END_TEST
/* ------------------------------------------------------------------------- */

void resume_signal_handler(int sig)
{
	if (sig == SIGUSR1) {
		sigusr1_flag = 1;
	} else if (sig == SIGCHLD) {
		sigchld_flag = 1;
	}
}

/** 
 * Ensure blocking of ssh port with iptables is removed, e.g, in case
 * a test case timeouts and blocking can not be removed in the test case
 * 
 */
void teardown_resume_test()
{
	int status;
	status = system("/sbin/iptables -D INPUT" LOCAL_SSH_RULE);
}

/** 
 * Runs testrunner-lite with --resume option, generates ssh connection failure
 * by using iptables, restores connection and handles required signaling with
 * SIGUSR1.
 * 
 * @param optionarg Argument value for option --resume
 * @param retval Excpected return value for tesrunner-lite
 * 
 * @return PID of forked testrunner-lite process
 */
pid_t run_resume_test(const char *optionarg, int retval)
{
	char result_file[128];
	char resume_option[128];
	pid_t pid = 0;
	int status = 0;
	siginfo_t info;
	sigset_t mask;
	sigset_t origmask;
	void (*origchld)(int);
	void (*origusr1)(int);

	/* block SIGCHLD until sigsuspend is called and set a handler so we can
	   catch too early return of forked testrunner-lite process */
	origchld = signal(SIGCHLD, resume_signal_handler);
	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, &origmask);

	fail_if((pid = fork()) < 0);

	if (pid == 0) {
		/* undo SIGCHLD changes here in the child process */
		sigprocmask(SIG_UNBLOCK, &mask, NULL);
		signal(SIGCHLD, origchld);
		sprintf(result_file, "/tmp/testrunner-lite-tests/"
			"trlitetestout%d.xml", getpid());
		sprintf(resume_option, "--resume=%s", optionarg);
		execl(TESTRUNNERLITE_BIN,
		      TESTRUNNERLITE_BIN,
		      "-f", TESTDATA_RESUME_TEST_XML,
		      "-o", result_file,
		      "-v",
		      "-t 127.0.0.1",
		      resume_option,
		      (char*)NULL);
		/* execl should not return */
	}

	/* setup handler */
	origusr1 = signal(SIGUSR1, resume_signal_handler);

	/* sleep until testrunner-lite is executing sleep step */
	sleep(10);
	/* cause ssh connection failure by blocking incoming packets
	   to port 22 on localhost */
	status = system("/sbin/iptables -A INPUT" LOCAL_SSH_RULE);

	/* wait for a signal */
	while (1) {
		sigsuspend(&origmask);
		if (sigchld_flag) {
			/* child has terminated. it's either iptables or tr-lite */
			sigchld_flag = 0;
			info.si_pid = 0;
			if (waitid(P_PID, pid, &info,
				   WEXITED | WNOHANG | WNOWAIT) == 0) {
				/* continue only if testrunner-lite not terminated */
				if (info.si_pid != pid) {
					continue;
				}
			}
		}
		break;
	}

	/* restore original handlers */
	signal(SIGUSR1, origusr1);
	signal(SIGCHLD, origchld);
	/* ... and no need to unblock SIGCHLD anymore */
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	/* remove port blocking */
	status = system("/sbin/iptables -D INPUT" LOCAL_SSH_RULE);

	/* test if SIGUSR1 has been received */
	if (sigusr1_flag) {
		/* signal testrunner-lite to continue */
		kill(pid, SIGUSR1);
	}

	/* wait testrunner-lite returns */
	fail_if(waitpid(pid, &status, 0) < 0);

	fail_unless(WIFEXITED(status) && WEXITSTATUS(status) == retval,
		    "Unexpected return value %d from testrunner-lite",
		    WEXITSTATUS(status));

	fail_unless(sigusr1_flag, "SIGSUR1 not received from testrunner-lite");

	return pid;
}

START_TEST (test_executor_remote_resume_exit)
	char result_file[128];
	pid_t pid = 0;
	int status = 0;

	/* clean possible old files */
	unlink("/tmp/testrunner-lite-tests/resumetest1.txt");
	unlink("/tmp/testrunner-lite-tests/resumetest2.txt");

	/* run resume test body */
	pid = run_resume_test("exit", 2);

	/* do some checks on results */
	status = system("[ -f /tmp/testrunner-lite-tests/resumetest1.txt ]");
	fail_unless(WEXITSTATUS(status) == 0 ,"resumetest1.txt does not exist");

	status = system("grep set1case1 /tmp/testrunner-lite-tests/resumetest1.txt");
	fail_unless(WEXITSTATUS(status) == 0, "contents of resumetest1.txt");

	status = system("[ ! -f /tmp/testrunner-lite-tests/resumetest2.txt ]");
	fail_unless(WEXITSTATUS(status) == 0, "resumetest2.txt should not exist");

	/* clean */
	sprintf(result_file, "/tmp/testrunner-lite-tests/"
			"trlitetestout%d.xml", pid);
	unlink(result_file);
	unlink("/tmp/testrunner-lite-tests/resumetest1.txt");
	unlink("/tmp/testrunner-lite-tests/resumetest2.txt");
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_resume_continue)
	char result_file[128];
	pid_t pid = 0;
	int status = 0;

	/* clean possible old files */
	unlink("/tmp/testrunner-lite-tests/resumetest1.txt");
	unlink("/tmp/testrunner-lite-tests/resumetest2.txt");

	/* run resume test body */
	pid = run_resume_test("continue", 0);

	/* do some checks on results */
	status = system("[ -f /tmp/testrunner-lite-tests/resumetest1.txt ]");
	fail_unless(WEXITSTATUS(status) == 0 ,"resumetest1.txt does not exist");

	status = system("grep set1case1 /tmp/testrunner-lite-tests/resumetest1.txt");
	fail_unless(WEXITSTATUS(status) == 0, "contents of resumetest1.txt");

	status = system("[ -f /tmp/testrunner-lite-tests/resumetest2.txt ]");
	fail_unless(WEXITSTATUS(status) == 0, "resumetest2.txt does not exist");

	status = system("grep set2case1 /tmp/testrunner-lite-tests/resumetest2.txt");
	fail_unless(WEXITSTATUS(status) == 0, "contents of resumetest2.txt");

	/* clean */
	sprintf(result_file, "/tmp/testrunner-lite-tests/"
			"trlitetestout%d.xml", pid);
	unlink(result_file);
	unlink("/tmp/testrunner-lite-tests/resumetest1.txt");
	unlink("/tmp/testrunner-lite-tests/resumetest2.txt");
END_TEST
/* ------------------------------------------------------------------------- */
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
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
    
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest2.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest3.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest4.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/get\\ test5.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
END_TEST

START_TEST(test_remote_hwinfo)
     int ret;
     char cmd[1024];
     char *out_file = "/tmp/testrunner-lite-tests/testrunner-lite.out.xml";

     sprintf(cmd, "%s -v -f %s -o %s -i localhost:22", TESTRUNNERLITE_BIN,
	      TESTDATA_SIMPLE_XML_1,  out_file);
     ret = system (cmd);
     fail_if (ret != 0, cmd);
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_remote_custom_key)

     int ret;
     char cmd[1024];
     
     sprintf (cmd, "%s -f %s -o /tmp/testrunnerlitetestdir2/res.xml "
	      "-t localhost -k ~/.ssh/myrsakey", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_SIMPLE_XML_1);
     ret = system (cmd);
     fail_if (ret, cmd);
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_remote_custom_key_full_path)

     int ret;
     char cmd[1024];
     char *homepath;
     homepath = getenv("HOME");
     sprintf (cmd, "%s -f %s -o /tmp/testrunnerlitetestdir2/res.xml "
	      "-t localhost -k %s/.ssh/myrsakey", 
              TESTRUNNERLITE_BIN, 
              TESTDATA_SIMPLE_XML_1, homepath);
     ret = system (cmd);
     fail_if (ret, cmd);
END_TEST


#ifdef ENABLE_LIBSSH2

/* ------------------------------------------------------------------------- */
START_TEST (test_remote_custom_key_get)

     int ret;
     char cmd[1024];
     
     sprintf (cmd, "%s -f %s -o /tmp/testrunnerlitetestdir2/res.xml "
	      "-t localhost -k ~/.ssh/myrsakey", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_GET_XML_1);
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
    
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest2.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest3.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest4.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/get\\ test5.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_remote_custom_key_port_get)

     int ret;
     char cmd[1024];
     
     sprintf (cmd, "%s -f %s -o /tmp/testrunnerlitetestdir2/res.xml "
	      "-t localhost:22 -k ~/.ssh/myrsakey", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_GET_XML_1);
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
    
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest2.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest3.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest4.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/get\\ test5.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_remote_custom_key_full_path_get)

     int ret;
     char cmd[1024];
     char *homepath;
     homepath = getenv("HOME");
     sprintf (cmd, "%s -f %s -o /tmp/testrunnerlitetestdir2/res.xml "
              "-t localhost -k %s/.ssh/myrsakey", 
              TESTRUNNERLITE_BIN, 
              TESTDATA_GET_XML_1, homepath);
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
    
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest2.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest3.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest4.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/get\\ test5.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
END_TEST


/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_libssh2_command)
	exec_data edata;
	testrunner_lite_options opts;
	memset (&opts, 0x0, sizeof (opts));
	opts.libssh2 = 1;
	opts.log_level = LOG_LEVEL;
	opts.username = getenv("LOGNAME");
	opts.ssh_key = "~/.ssh/myrsakey";
	opts.target_address = "localhost";
	opts.target_port = 0;
	executor_init (&opts);
	log_init(&opts);
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
	executor_close();
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_libssh2_default_key)
/* Just to see if using default key causes problem.
   Not trying to connect */
	testrunner_lite_options opts;
	memset (&opts, 0x0, sizeof (opts));
	opts.libssh2 = 1;
	opts.log_level = LOG_LEVEL;
	opts.username = getenv("LOGNAME");
	opts.ssh_key = "";
	opts.target_address = "localhost";
	opts.target_port = 0;
	printf("a\n");
	executor_init (&opts);
	printf("b\n");
	log_init(&opts);
	printf("c\n");
	executor_close();
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_libssh2_nonexisting_key)
    testrunner_lite_options opts;
    int ret = 0;
    memset (&opts, 0x0, sizeof (opts));
    opts.libssh2 = 1;
    opts.log_level = LOG_LEVEL;
    opts.username = getenv("LOGNAME");
    opts.ssh_key = "foobarbar";
    opts.target_address = "localhost";
    opts.target_port = 0;
    ret = executor_init (&opts);
    fail_unless (ret != 0);
    log_init(&opts);
	executor_close();

    memset (&opts, 0x0, sizeof (opts));
    opts.libssh2 = 1;
    opts.log_level = LOG_LEVEL;
    opts.username = getenv("LOGNAME");
    opts.ssh_key = "~/foobarbar";
    opts.target_address = "localhost";
    opts.target_port = 0;
    ret = executor_init (&opts);
    fail_unless (ret != 0);
    log_init(&opts);
	executor_close();
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_libssh2_command_port)
	exec_data edata;
	testrunner_lite_options opts;
	memset (&opts, 0x0, sizeof (opts));
	opts.libssh2 = 1;
	opts.log_level = LOG_LEVEL;
	opts.username = getenv("LOGNAME");
	opts.ssh_key = "~/.ssh/myrsakey";
	opts.target_address = "localhost";
	opts.target_port = 22;
	executor_init (&opts);
	log_init(&opts);
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
	executor_close();
END_TEST
/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_libssh2_long_command)
	exec_data edata;
	testrunner_lite_options opts;
#define TEST_STRING_SIZE 5000
	char test_string [TEST_STRING_SIZE];
	char command [TEST_STRING_SIZE + 100];
	memset (&opts, 0x0, sizeof (opts));
	opts.libssh2 = 1;
	opts.log_level = LOG_LEVEL;
	opts.username = getenv("LOGNAME");
	opts.ssh_key = "~/.ssh/myrsakey";
	opts.target_address = "localhost";
	opts.target_port = 0;
	executor_init (&opts);
	log_init(&opts);
	init_exec_data (&edata);
	edata.soft_timeout = 5;
	edata.hard_timeout = 5;

	memset (test_string , 'c', TEST_STRING_SIZE);
	test_string [TEST_STRING_SIZE - 1] = '\0';
	sprintf (command, "echo %s; sleep 2", test_string);
	fail_if (execute(command, &edata));
	fail_unless (edata.result == 0, "result = %d", edata.result);
	fail_unless (edata.stdout_data.length == TEST_STRING_SIZE, 
		     "length %d",
		     edata.stdout_data.length);
	fail_unless (strncmp((char*)edata.stdout_data.buffer, test_string,
			     TEST_STRING_SIZE -1) == 0,
		     edata.stdout_data.buffer);
	executor_close();
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_libssh2_quotes_command)
    exec_data edata;
    testrunner_lite_options opts;
    memset (&opts, 0x0, sizeof (opts));
    char *output = "i like pipes and quotes";
    char *command = "[ \"|/bin/echo %s\" = \"$(cat /proc/cpuinfo)\" ];"
	"echo i like pipes and quotes";
    opts.libssh2 = 1;
    opts.log_level = LOG_LEVEL;
    opts.username = getenv("LOGNAME");
    opts.ssh_key = "~/.ssh/myrsakey";
    opts.target_address = "localhost";
    opts.target_port = 0;
    log_init(&opts);
    executor_init (&opts);
    init_exec_data (&edata);
    edata.soft_timeout = 5;
    edata.hard_timeout = 5;
    fail_if (execute(command, &edata));
    fail_unless (strncmp((char*)edata.stdout_data.buffer, 
                         output,
                         strlen(output)) == 0);
    executor_close();
END_TEST


/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_libssh2_terminating_process)
	exec_data edata;
	testrunner_lite_options opts;
	int ret;
	/* Clean hanging processes */
	ret = system("killall -9 terminating unterminating");

	memset (&opts, 0x0, sizeof (opts));
	opts.target_address = "localhost";
	opts.target_port = 0;
	opts.libssh2 = 1;
	opts.log_level = LOG_LEVEL;
	opts.username = getenv("LOGNAME");
	opts.ssh_key = "~/.ssh/myrsakey";
	executor_init (&opts);
	log_init(&opts);
	init_exec_data(&edata);
	edata.soft_timeout = 5;
	edata.hard_timeout = 5;
	fail_if (execute("/usr/lib/testrunner-lite-tests/terminating " 
			 "stdouttest stderrtest", &edata));

	fail_if (edata.stdout_data.buffer == NULL);
	fail_if (edata.stderr_data.buffer == NULL);
	fail_if (strstr((char*)edata.stdout_data.buffer, "stdouttest") == 0, 
	         (char*)edata.stdout_data.buffer);
	fail_if (strstr((char*)edata.stderr_data.buffer, 
			     "stderrtest") == 0,
		     (char*)edata.stderr_data.buffer);

	fail_if (edata.signaled != SIGTERM);
	executor_close();

	/* check that killing was succesfull */
	fail_unless (system("pidof terminating"));
END_TEST


/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_libssh2_killing_process)
	exec_data edata;
	testrunner_lite_options opts;
	int ret;
	/* Clean hanging processes */
	ret = system("killall -9 terminating unterminating");

	memset (&opts, 0x0, sizeof (opts));
	opts.libssh2 = 1;
	opts.log_level = LOG_LEVEL;
	opts.username = getenv("LOGNAME");
	opts.ssh_key = "~/.ssh/myrsakey";
	opts.target_address = "localhost";
	opts.target_port = 0;
	executor_init (&opts);
	init_exec_data(&edata);
	log_init(&opts);
	edata.soft_timeout = 5;
	edata.hard_timeout = 5;
	fail_if (execute("/usr/lib/testrunner-lite-tests/unterminating "
			 "stdouttest stderrtest", &edata));
	/* 128 + SIGKILL or 255 depends on ssh */
	/* Let's skip this until we get recent enough
	   libssh2 in to production (get_exit_signal required) 
	fail_unless (edata.result == 143 ||
		     edata.result == 255, "result %d", edata.result); 
	fail_if (edata.stdout_data.buffer == NULL);
	fail_if (edata.stderr_data.buffer == NULL);
	*/
	fail_if (strstr((char*)edata.stdout_data.buffer, 
			     "stdouttest") == 0,
		     edata.stdout_data.buffer);
	fail_if (strstr((char*)edata.stderr_data.buffer, 
			     "stderrtest") == 0,
		     edata.stderr_data.buffer);
	fail_if (edata.signaled != SIGKILL);
	executor_close();
	/* Check that process doesn't exist anymore */
	fail_unless (system("pidof unterminating"));
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_libssh2_bg_process)
	exec_data edata;
	testrunner_lite_options opts;
	int ret;
	/* Clean hanging processes */
	ret = system("killall -9 terminating unterminating");

	memset (&opts, 0x0, sizeof (opts));
	opts.libssh2 = 1;
	opts.log_level = LOG_LEVEL;
	opts.username = getenv("LOGNAME");
	opts.ssh_key = "~/.ssh/myrsakey";
	opts.target_address = "localhost";
	opts.target_port = 0;
	executor_init (&opts);
	init_exec_data(&edata);
	log_init(&opts);
	edata.soft_timeout = 20;
	edata.hard_timeout = 5;
	fail_if (execute("/usr/lib/testrunner-lite-tests/terminating aa bb &"
			 , &edata));
	/* Check that the process stayed in bg */
	fail_if (execute("PATH=$PATH:/sbin/ pidof terminating", &edata));
	fail_if (edata.stdout_data.buffer == NULL);
	executor_close();
	/* Check that session closing actually terminates the bg process too */
	executor_init (&opts);
	init_exec_data(&edata);
	fail_if (execute("PATH=$PATH:/sbin/ pidof terminating", &edata));
	fail_unless (edata.result == 1);
	executor_close();
END_TEST


/* ------------------------------------------------------------------------- */
START_TEST (test_executor_remote_libssh2_daemon_process)
	exec_data edata;
	testrunner_lite_options opts;
	int ret;
	/* Clean hanging processes */
	ret = system("killall -9 trlite-test-daemon");

	memset (&opts, 0x0, sizeof (opts));
	opts.libssh2 = 1;
	opts.log_level = LOG_LEVEL;
	opts.username = getenv("LOGNAME");
	opts.ssh_key = "~/.ssh/myrsakey";
	opts.target_address = "localhost";
	opts.target_port = 0;
	executor_init (&opts);
	init_exec_data(&edata);
	log_init(&opts);
	edata.soft_timeout = 20;
	edata.hard_timeout = 1;
	fail_if (execute("/usr/lib/testrunner-lite-tests/trlite-test-daemon"
			 , &edata));
	/* Check that the process stayed in bg */
	fail_if (execute("PATH=$PATH:/sbin/ pidof trlite-test-daemon", &edata));
	fail_if (edata.stdout_data.buffer == NULL);
	executor_close();
	/* Check that session closing actually terminates the daemon process too */
	executor_init (&opts);
	init_exec_data(&edata);
	fail_if (execute("PATH=$PATH:/sbin/ pidof trlite-test-daemon", &edata));
	executor_close();
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_remote_libssh2_get)

     int ret;
     char cmd[1024];
     
     /* get doesn't use libssh2, but username parsing differs with -t option */

     sprintf (cmd, "%s -f %s -o /tmp/testrunnerlitetestdir2/res.xml "
	      "-n localhost -k ~/.ssh/myrsakey", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_GET_XML_1);
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
    
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest2.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest3.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest4.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/get\\ test5.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_remote_libssh2_get_username)

     int ret;
     char cmd[1024];

     char *username = getenv("LOGNAME");

     /* get doesn't use libssh2, but username parsing differs with -t option */

     sprintf (cmd, "%s -f %s -o /tmp/testrunnerlitetestdir2/res.xml "
	      "-n %s@localhost -k ~/.ssh/myrsakey", 
              TESTRUNNERLITE_BIN, 
              TESTDATA_GET_XML_1,
              username);
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
    
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest2.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest3.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest4.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/get\\ test5.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_remote_libssh2_custom_key_get)

     int ret;
     char cmd[1024];
     
     sprintf (cmd, "%s -f %s -o /tmp/testrunnerlitetestdir2/res.xml "
	      "-n localhost -k ~/.ssh/myrsakey", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_GET_XML_1);
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
    
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest2.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest3.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest4.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/get\\ test5.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
END_TEST


/* ------------------------------------------------------------------------- */
START_TEST (test_remote_libssh2_custom_key_port_get)

     int ret;
     char cmd[1024];
     
     sprintf (cmd, "%s -f %s -o /tmp/testrunnerlitetestdir2/res.xml "
	      "-n localhost:22 -k ~/.ssh/myrsakey", 
	      TESTRUNNERLITE_BIN, 
	      TESTDATA_GET_XML_1);
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
    
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest2.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest3.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest4.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/get\\ test5.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
END_TEST

/* ------------------------------------------------------------------------- */
START_TEST (test_remote_libssh2_custom_key_full_path_get)

     int ret;
     char cmd[1024];
     char *homepath;
     homepath = getenv("HOME");
     sprintf (cmd, "%s -f %s -o /tmp/testrunnerlitetestdir2/res.xml "
              "-n localhost -k %s/.ssh/myrsakey", 
              TESTRUNNERLITE_BIN, 
              TESTDATA_GET_XML_1, homepath);
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
    
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest2.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
     
     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest3.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/gettest4.txt");
     ret = system (cmd);
     fail_if (ret, cmd);

     sprintf (cmd, "stat /tmp/testrunnerlitetestdir2/get\\ test5.txt");
     ret = system (cmd);
     fail_if (ret, cmd);
END_TEST

#endif /* ENABLE_LIBSSH2 */

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
Suite *make_testexecutor_suite (void)
{
    /* Create suite. 1*/
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

    tc = tcase_create ("Test executor remote command with port.");
    tcase_add_test (tc, test_executor_remote_command_port);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor remote long command.");
    tcase_set_timeout (tc, 10);
    tcase_add_test (tc, test_executor_remote_long_command);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor remote terminating process.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_terminating_process);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor remote killing process.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_killing_process);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor remote bg process cleanup.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_test_bg_process_cleanup);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test get feature with remote host.");
    tcase_set_timeout (tc, 25);
    tcase_add_test (tc, test_remote_get);
    suite_add_tcase (s, tc);
    
    tc = tcase_create ("Test ssh connection check routine.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_conn_check);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test resume testrun feature with --resume=exit.");
    tcase_set_timeout (tc, 80);
    tcase_add_unchecked_fixture (tc, NULL, teardown_resume_test);
    tcase_add_test (tc, test_executor_remote_resume_exit);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test resume testrun feature with --resume=continue.");
    tcase_set_timeout (tc, 80);
    tcase_add_unchecked_fixture (tc, NULL, teardown_resume_test);
    tcase_add_test (tc, test_executor_remote_resume_continue);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test obtaining hwinfo remotely.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_remote_hwinfo);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test custom ssh key.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_remote_custom_key);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test custom ssh key full path.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_remote_custom_key_full_path);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test custom ssh key get.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_remote_custom_key_get);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test custom ssh key port get.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_remote_custom_key_port_get);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test custom ssh key get full path.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_remote_custom_key_full_path_get);
    suite_add_tcase (s, tc);

#ifdef ENABLE_LIBSSH2
    tc = tcase_create ("Test executor remote libssh2 command.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_libssh2_command);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor remote libssh2 default key.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_libssh2_default_key);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor remote libssh2 nonexisting key");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_libssh2_nonexisting_key);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor remote libssh2 command with port.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_libssh2_command_port);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor remote libssh2 long command.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_libssh2_long_command);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor remote libssh2 quotes command.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_libssh2_quotes_command);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor remote libssh2 terminating process.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_libssh2_terminating_process);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test executor remote libssh2 killing process.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_libssh2_killing_process);
    suite_add_tcase (s, tc);

	tc = tcase_create ("Test executor remote libssh2 background process.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_libssh2_bg_process);
    suite_add_tcase (s, tc);

	tc = tcase_create ("Test executor remote libssh2 daemon process.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_executor_remote_libssh2_daemon_process);
    suite_add_tcase (s, tc);

	tc = tcase_create ("Test remote libssh2 get.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_remote_libssh2_get);
    suite_add_tcase (s, tc);

	tc = tcase_create ("Test remote libssh2 get username.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_remote_libssh2_get_username);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test libssh2 custom ssh key get.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_remote_libssh2_custom_key_get);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test libssh2 custom ssh key port get.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_remote_libssh2_custom_key_port_get);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test libssh2 custom ssh key get full path.");
    tcase_set_timeout (tc, 20);
    tcase_add_test (tc, test_remote_libssh2_custom_key_full_path_get);
    suite_add_tcase (s, tc);

#endif /* ENABLE_LIBSSH2 */

    return s;
}


/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
