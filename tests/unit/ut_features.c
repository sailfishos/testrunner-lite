/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include "testrunnerlitetestscommon.h"
#include "executor.h"
#include "hwinfo.h"
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
START_TEST (test_ctrl_char_strip)
    exec_data edata;
    char cmd[1024];
    const char test_str[] = {'t',0x02,'e','s','t','f',0x06,0x07,0x08,0x09,
			     'o',0x0B,'o',0x0C,0x0E,0x0F,0x10,0x11,0x12,
			     0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,
			     0x1C,0x1D,0x1E,0x1F,0x7F,'b','a','r'};

     const char valid_str[] = {'t',' ','e','s','t','f',' ',' ',' ',' ',
			       'o',' ','o',' ',' ',' ',' ',' ',' ',
			       ' ',' ',' ',' ',' ',' ',' ',' ',' ',
			       ' ',' ',' ',' ',' ','b','a','r', '\0'};

    
    testrunner_lite_options opts;

    memset (&opts, 0x0, sizeof (opts));
    executor_init (&opts);

    init_exec_data(&edata);
    edata.soft_timeout = 0;
    edata.hard_timeout = 0;
    sprintf (cmd, "echo -e %s", test_str); 
    fail_if (execute(cmd, &edata));
    fail_if (strlen ((char *)edata.stdout_data.buffer) == 0);
    fail_unless (strlen ((char *)edata.stderr_data.buffer) == 0);
    fail_if (strncmp ((char *)edata.stdout_data.buffer, valid_str, 
		      strlen (valid_str)),
	     "FAIL: stdout %s != %s", edata.stdout_data.buffer, valid_str);

END_TEST

START_TEST (test_logging)

    char *stdout_tmp = "/tmp/testrunner-lite-stdout.log";
    char cmd[1024];
    char message[LOG_MSG_MAX_SIZE + 1];
    int ret;
    FILE *fp;
    testrunner_lite_options opts;
    memset (&opts, 0x0, sizeof (testrunner_lite_options)); 
    memset (message, 'a', LOG_MSG_MAX_SIZE);
    message [LOG_MSG_MAX_SIZE - 1] = '\0';
    message [LOG_MSG_MAX_SIZE - 2] = 'b';

    /* Set verbosity to INFO. */
    opts.log_level = LOG_LEVEL_INFO;
    log_init (&opts);
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* Log INFO, WARNING and ERROR messages. */
    LOG_MSG (LOG_INFO, "INFO message: %s\n", "This works.");
    LOG_MSG (LOG_WARNING, "WARNING message: %s\n", "This works.");
    LOG_MSG (LOG_ERR, "ERROR message: %s\n", "This works.");
    LOG_MSG (LOG_INFO, message);
    memset (message, 'b', LOG_MSG_MAX_SIZE + 1);
    message [LOG_MSG_MAX_SIZE] = '\0';
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);

    // And verify messages. */
    sprintf (cmd, "grep \"[INFO]* INFO message: This works.\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    sprintf (cmd, "grep \"[WARNING]* WARNING message: This works.\" %s", 
	     stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    sprintf (cmd, "grep \"[ERROR]* ERROR message: This works.\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    sprintf (cmd, "grep aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);

    sprintf (cmd, "grep bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb %s", stdout_tmp); 
    ret = system (cmd);
    fail_unless (ret != 0, cmd);


    /* Try to log DEBUG message with INFO verbosity (should not succeed.)*/
    LOG_MSG (LOG_DEBUG, "DEBUG message: %s\n", "This should not work.");
    freopen ("/dev/tty", "w", stdout);
    
    sprintf (cmd, "grep \"[DEBUG]* DEBUG message: This should not work.\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret == 0, cmd);
    
    /* Set verbosity to DEBUG. */
    opts.log_level = LOG_LEVEL_DEBUG;
    log_init (&opts);
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    /* Log INFO, WARNING and ERROR messages. */
    LOG_MSG (LOG_INFO, "INFO message: %s\n", "This works.");
    LOG_MSG (LOG_WARNING, "WARNING message: %s\n", "This works.");
    LOG_MSG (LOG_ERR, "ERROR message: %s\n", "This works.");
    LOG_MSG (LOG_DEBUG, "DEBUG message: %s\n", "This works.");
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);

    // And verify messages. */
    sprintf (cmd, "grep \"[INFO]* INFO message: This works.\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    sprintf (cmd, "grep \"[WARNING]* WARNING message: This works.\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    sprintf (cmd, "grep \"[ERROR]* ERROR message: This works.\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    sprintf (cmd, "grep \"[DEBUG]* DEBUG message: This works.\" %s", stdout_tmp); 
    ret = system (cmd);
    fail_if (ret != 0, cmd);
    
    /* Set verbosity to SILENT. */
    opts.log_level = LOG_LEVEL_SILENT;
    log_init (&opts);
    
    /* Forward stdout temporarily to a file. */
    fp = freopen (stdout_tmp, "w", stdout);
    
    LOG_MSG (LOG_INFO, "INFO message: %s\n", "Silent mode.");
    
    /* Back to terminal. */
    freopen ("/dev/tty", "w", stdout);
    
    sprintf (cmd, "grep \"[INFO]* INFO message: Silent mode.\" %s", stdout_tmp); 
    
    ret = system (cmd);
    fail_if (ret == 0, cmd);
    
END_TEST

/* Opens tcp server socket, listens for a connection, reads, and writes simple ack
 * @param portno TCP port number of socket
 * @param buffer Buffer to store data from client socket
 * @param length Max length of data to read in buffer
 * @param error Buffer to store error description in case of an error occurs
 */
static void run_server_socket(int portno, char* buffer, int length, char* error) {
    struct sockaddr_in serv_addr, cli_addr;
    const char reply_message[] = "Message saved";
    int serverfd = 0, clientfd = 0;
    socklen_t clilen = 0;
    int reuseaddr_option = 1;

    serverfd = socket(AF_INET, SOCK_STREAM, 0);

    if (serverfd > 0) {
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	/* Try to avoid "Address already in use" error from bind
	 * in case there is socket in state TIME_WAIT */
	setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, 
		   &reuseaddr_option, sizeof(reuseaddr_option));

	if (bind(serverfd, (struct sockaddr *)&serv_addr,
		 sizeof(serv_addr)) == 0) {
	    listen(serverfd, 5);
	    clilen = sizeof(cli_addr);
	    clientfd = accept(serverfd, (struct sockaddr *) &cli_addr, 
			       &clilen);
	    if (clientfd > 0) {
		if (read(clientfd, buffer, length) < 0) {
		    strcpy(error, strerror(errno));
		}

		/* create a dummy reply, the same as from CITA */
		write(clientfd, reply_message, strlen(reply_message));
		close(clientfd);
	    } else {
		strcpy(error, strerror(errno));
	    }
	} else {
	    strcpy(error, strerror(errno));
	}

    } else {
	strcpy(error, strerror(errno));
    }

    if (serverfd > 0) {
	close(serverfd);
    }
}

START_TEST (test_remote_logging)
{
    testrunner_lite_options opts;
    char buffer[1024];
    char error[128];
    const char sample_message[] = "Remote logger test data";
    const char sample_message_url_enc[] = "Remote%20logger%20test%20data";
    const char logger[] = "127.0.0.1";
    pid_t pid = 0;

    fail_if((pid = fork()) < 0);

    if (pid == 0) {
	/* child process to generate a log message */
	memset (&opts, 0, sizeof(testrunner_lite_options));
	opts.log_level = LOG_LEVEL_INFO;
	opts.remote_logger = malloc(strlen(logger)+1);
	strcpy(opts.remote_logger, logger);

	/* wait for parent's server socket to be opened */
	usleep(200000);

	log_init(&opts);
	LOG_MSG(LOG_INFO, "%s", sample_message);
	log_close();

	free(opts.remote_logger);
	exit(0);
    }

    memset(buffer, 0, sizeof(buffer));
    memset(error, 0, sizeof(error));

    run_server_socket(80, buffer, sizeof(buffer) - 1, error);

    /* wait child terminates */
    wait(NULL);

    fail_if(strlen(error) > 0, error);

    /* Check that buffer contains at least something we expected */
    fail_if(strstr(buffer, "HTTP") == NULL);
    fail_if(strstr(buffer, sample_message_url_enc) == NULL);
}
END_TEST

START_TEST (test_remote_logging_command)
{
    char buffer[1024];
    char error[128];
    char logger_option[128];
    int portno = 5678;
    pid_t pid = 0;

    fail_if((pid = fork()) < 0);

    if (pid == 0) {
	/* child process to run testrunner-lite with --logger option */

	/* wait for parent's server socket to be opened */
	usleep(200000);

	sprintf(logger_option, "--logger=http://127.0.0.1:%d", portno);

	execl(TESTRUNNERLITE_BIN,
	      TESTRUNNERLITE_BIN,
	      "-f", TESTDATA_SIMPLE_XML_1,
	      "-o", "/tmp/loggertestout.xml",
	      "-v",
	      logger_option,
	      (char*)NULL);
	/* should never reach this point */
	exit(1);
    }

    memset(buffer, 0, sizeof(buffer));
    memset(error, 0, sizeof(error));

    run_server_socket(portno, buffer, sizeof(buffer) - 1, error);

    /* wait child terminates */
    wait(NULL);

    fail_if(strlen(error) > 0, error);

    /* Check that buffer contains at least something we expected */
    fail_if(strstr(buffer, "HTTP") == NULL);
    fail_if(strstr(buffer, "levelname") == NULL);
}
END_TEST

START_TEST (test_hwinfo)
     
     hw_info hi;
     testrunner_lite_options opts;

     memset (&opts, 0x0, sizeof (opts));
     executor_init (&opts);

     memset (&hi, 0x0, sizeof (hw_info));
     fail_if (read_hwinfo(&hi));
	      
     fail_unless (hi.product);
     fail_unless (hi.hw_build);

     print_hwinfo (&hi);

     clean_hwinfo (&hi);
    
END_TEST

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
Suite *make_features_suite (void)
{
    /* Create suite. */
    Suite *s = suite_create ("tests for various testrunner-lite features");

    /* Create test cases and add to suite. */
    TCase *tc;

    tc = tcase_create ("Test stripping of ctrl chars.");
    tcase_set_timeout(tc, 600);
    tcase_add_test (tc, test_ctrl_char_strip);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test logging.");
    tcase_add_test (tc, test_logging);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test remote logging.");
    tcase_add_test (tc, test_remote_logging);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test remote logging command.");
    tcase_add_test (tc, test_remote_logging_command);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Test hw info.");
    tcase_add_test (tc, test_hwinfo);
    suite_add_tcase (s, tc);

    return s;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
