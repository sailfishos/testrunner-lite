/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Sampo Saaristo <ext-sampo.2.saaristo@nokia.com>
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
#define SSHCMD      "/usr/bin/ssh"
#define SSHCMDARGS  "-o StrictHostKeyChecking=no",\
                    "-o PasswordAuthentication=no"
#define SSHCMDARGS_STR  "-o StrictHostKeyChecking=no " \
                        "-o PasswordAuthentication=no"
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
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** Init the ssh executor
 */
void ssh_executor_init (const char *hostname)
{
	int ret, status;
	pid_t pid;
	char *cmd = "echo '#!/bin/sh' > /tmp/mypid.sh;"
		" echo 'echo $PPID' >> /tmp/mypid.sh; chmod +x /tmp/mypid.sh";
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
		return;
	}
	if (pid < 0)
		return;
	

	ret = execl(SSHCMD, SSHCMD, SSHCMDARGS, hostname, 
		    cmd, (char*)NULL);
	
}
/* ------------------------------------------------------------------------- */
/** Executes a command using ssh 
 * @param hostname SUT address 
 * @param command Command to execute
 * @return Does not return in success, error code from exec in case of error
 */
int ssh_execute (const char *hostname, const char *command)
{
	int   ret;
        char *cmd; 
	char *casename;
	char *setname;
	int   stepnum;
	/*
	 * Query the current set name, case name and step number 
	 * from testdefinition processor so we can put some debug 
	 * to target syslog
	 */
	casename = current_case_name();
	stepnum  = current_step_num();
	setname  = current_set_name();

        cmd = (char *)malloc (PID_FILE_MAX_LEN + 130 + strlen (command)
			      + strlen (casename) + strlen (setname));
        if (!cmd) {
                fprintf (stderr, "%s: could not allocate memory for "
                         "command %s\n", __FUNCTION__, command);
        }
	if (strlen (casename) && strlen (setname)) 
		sprintf (cmd, "logger set:%s-case:%s-step:%d;/tmp/mypid.sh > " 
			 PID_FILE_FMT 
			 ";source .profile > /dev/null; %s",
			 setname, casename, stepnum, unique_id, getpid(), command);
	else
		sprintf (cmd, "/tmp/mypid.sh > " 
			 PID_FILE_FMT 
			 ";source .profile > /dev/null; %s",
			 unique_id, getpid(), command);

        /* cmd can not be freed since execl does not return here */
        ret = execl(SSHCMD, SSHCMD, SSHCMDARGS, hostname, 
                    cmd, (char*)NULL);


        return ret;
}
/* ------------------------------------------------------------------------- */
/** Tries to check if ssh connections are still working
 * @param hostname SUT address 
 * @return 0 or ssh error code
 */
int ssh_check_conn (const char *hostname)
{
	int ret;
	char cmd[1024];
	
	sprintf (cmd, "%s %s %s echo foo", SSHCMD, SSHCMDARGS_STR, hostname);
	ret = system (cmd);
	return ret;
}
/* ------------------------------------------------------------------------- */
/** Tries to kill program started by ssh and removes temporary file
 *  @param hostname SUT address 
 *  @param id PID of the test step
 */
int ssh_kill (const char *hostname, pid_t id)
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
	sprintf (cmd, "[ -f %1$s ] && pkill -9 -P $(cat %1$s); rm -f %1$s", 
		 file);

	ret = execl(SSHCMD, SSHCMD, SSHCMDARGS, hostname, 
		    cmd, (char*)NULL);

	return ret;
}
/* ------------------------------------------------------------------------- */
/** Clean temporary file from target machine
 *  @param hostname SUT address 
 *  @param id PID of the test step
 */
void ssh_clean (const char *hostname, pid_t id)
{
	int ret, status;
	pid_t pid;
	char cmd [PID_FILE_MAX_LEN + 80];

	sprintf (cmd, "rm -f " PID_FILE_FMT, unique_id, id);

	pid = fork();

	if (pid) {
		waitpid(pid, &status, 0);
		return;
	}

	/* child: clean up target */
	ret = execl(SSHCMD, SSHCMD, SSHCMDARGS, hostname, 
		    cmd, (char*)NULL);

	return;
}
/* ------------------------------------------------------------------------- */
/** Clean up
 *  @param hostname SUT address 
 */
void ssh_executor_close (const char *hostname)
{
	free (unique_id);
	return;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
