/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Contains changes by Wind River Systems, 2011-03-09
 *
 * Contact: Sampo Saaristo <sampo.saaristo@sofica.fi>
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
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <wordexp.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "testrunnerlite.h"
#include "executor.h"
#include "testdefinitionprocessor.h"
#include "remote_executor.h"
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
LOCAL char *unique_id = NULL;
/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */
#define UNIQUE_ID_FMT     "%d"
#define PID_FILE_FMT      "/var/tmp/testrunner-lite-%s.%d.pid"
#define UNIQUE_ID_MAX_LEN (HOST_NAME_MAX + 10 + 1 + 1)
#define PID_FILE_MAX_LEN  (30 + UNIQUE_ID_MAX_LEN + 10 + 1 + 1)

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
/* None */
/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL int _execute (const char *executor, const char *command);
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/** Execute the given command with executor
 * @param executor prepended to command to execute on DUT
 * @command command to execute
 * @return non-zero on failure, no return on success
 */
LOCAL int _execute (const char *executor, const char *command)
{
	int ret = 0;
	wordexp_t we;
	char **argv = NULL;
	size_t argv_size;
	size_t i;

	/* expand executor string to array of words */
	ret = wordexp(executor, &we, 0);
	if (ret) {
		LOG_MSG(LOG_ERR, "Executor word expansion failed (%d): %s",
				ret, executor);
		goto out_no_wordfree;
	}

	/* create argv from executor + command */
	argv_size = we.we_wordc + 2; /* +2 for command and NULL */
	argv = malloc(argv_size * sizeof(char *));
	if (argv == NULL) {
		fprintf(stderr, "malloc failed");
		ret = -1;
		goto out;
	}
	for (i = 0; i < we.we_wordc; i++)
		argv[i] = we.we_wordv[i];
	argv[i] = (char *)command;
	argv[i + 1] = NULL;

	ret = execvp(argv[0], argv);

	/* only reached if exec fails */
	fprintf(stderr, "Failed to exec executor: %s", executor);

out:
	wordfree(&we);
out_no_wordfree:
	if (argv)
		free(argv);
	return ret;
}
/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** Init the remote executor
 * @param executor prepended to command to execute on DUT
 * @return 0 on success
 */
int remote_executor_init (const char *executor)
{
	int ret, status;
	pid_t pid;
	char *cmd = "echo '#!/bin/sh' > /tmp/mypid.sh;"
		"echo 'echo $PPID' >> /tmp/mypid.sh;";
	unique_id = (char *)malloc (UNIQUE_ID_MAX_LEN);
	ret = gethostname(unique_id, HOST_NAME_MAX);
	if (ret) {
		LOG_MSG(LOG_ERR, "Failed to get host name: %s", 
			strerror (errno));
		strcpy (unique_id, "foo");
	}
	sprintf (unique_id + strlen(unique_id), UNIQUE_ID_FMT, getpid());

	LOG_MSG(LOG_DEBUG, "unique_id set to %s", unique_id);
	
	pid = fork();
	if (pid > 0) { 
		waitpid(pid, &status, 0);
		if (WIFEXITED(status))
			return WEXITSTATUS(status);
		return status;
	}
	if (pid < 0)
		return pid;
	
	return _execute (executor, cmd);
}
/* ------------------------------------------------------------------------- */
/** Executes a command using a remote executor
 * @param executor prepended to command to execute on DUT
 * @param command Command to execute
 * @return Does not return in success, error code from exec in case of error
 */
int remote_execute (const char *executor, const char *command)
{
	int   ret;
        char *cmd; 
	const char *casename;
	const char *setname;
	int   stepnum;
	/*
	 * Query the current set name, case name and step number 
	 * from testdefinition processor so we can put some debug 
	 * to target syslog
	 */
	casename = current_case_name();
	stepnum  = current_step_num();
	setname  = current_set_name();

        cmd = (char *)malloc (PID_FILE_MAX_LEN + 256 + strlen (command)
			      + strlen (casename) + strlen (setname));
        if (!cmd) {
                fprintf (stderr, "%s: could not allocate memory for "
                         "command %s\n", __FUNCTION__, command);
        }
	if (strlen (casename) && strlen (setname)) 
		sprintf (cmd, "logger set:%s-case:%s-step:%d || true;"
			 "sh < /tmp/mypid.sh > " 
			 PID_FILE_FMT 
			 "; if [ -e .profile ]; then source .profile >"
			 " /dev/null; fi; %s", setname, casename, stepnum, 
			 unique_id, getpid(), command);
	else
		sprintf (cmd, "sh < /tmp/mypid.sh > " 
			 PID_FILE_FMT 
			 "; if [ -e .profile ]; then source .profile >"
			 " /dev/null; fi; %s",
			 unique_id, getpid(), command);

	ret = _execute (executor, cmd);
        return ret;
}
/* ------------------------------------------------------------------------- */
/** Tries to check if remote connections are still working
 * @param executor prepended to command to execute on DUT
 * @return 0 or executor error code
 */
int remote_check_conn (const char *executor)
{
	int ret;
	char cmd[1024];

	sprintf (cmd, "%s \"echo echo from remote connection check\"", 
		 executor);
	ret = system (cmd);
	return ret;
}
/* ------------------------------------------------------------------------- */
/** Tries to kill program started by remote executor and removes temporary file
 *  @param executor prepended to command to execute on DUT
 *  @param id PID of the test step
 */
int remote_kill (const char *executor, pid_t id, int signal)
{
	int ret;
	pid_t pid;
	char cmd [PID_FILE_MAX_LEN * 3 + 80];
	char file [PID_FILE_MAX_LEN];
	int status;
	
	pid = fork();
	if (pid > 0) { 
		waitpid(pid, &status, 0);
		return status;
	}
	if (pid < 0)
		return 1;
	
	sprintf(file, PID_FILE_FMT, unique_id, id);
	sprintf (cmd, "[ -f %1$s ] && pkill -%2$d -P $(cat %1$s)",
		 file, signal);

	ret = _execute (executor, cmd);

	return ret;
}
/* ------------------------------------------------------------------------- */
/** Clean temporary file from target machine
 *  @param executor prepended to command to execute on DUT
 *  @param id PID of the test step
 *  @return 0 on success
 */
int remote_clean (const char *executor, pid_t id)
{
	int status;
	pid_t pid;
	char cmd [PID_FILE_MAX_LEN + 80];

	sprintf (cmd, "rm -f " PID_FILE_FMT, unique_id, id);

	pid = fork();

	if (pid > 0) {
		waitpid(pid, &status, 0);
		if (WIFEXITED(status))
			return WEXITSTATUS(status);
		return status;
	}
	if (pid < 0)
		return pid;

	/* child: clean up target */
	return _execute (executor, cmd);
}
/* ------------------------------------------------------------------------- */
/** Clean up
 *  @return 0 on success
 */
int remote_executor_close (void)
{

	free (unique_id);

	return 0;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
