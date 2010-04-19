/* * This file is part of testrunner-lite *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 *
 * Contact: Sampo Saaristo <test-tools-dev@projects.maemo.org>
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
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include "testrunnerlite.h"
#include "executor.h"
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
/* ------------------------------------------------------------------------- */
/* LOCAL GLOBAL VARIABLES */
LOCAL char *unique_id = NULL;
/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */
#define UNIQUE_ID_FMT     "%s.%d"
#define PID_FILE_FMT      "/tmp/testrunner-lite-%s.%d.pid"
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
void ssh_executor_init ()
{
	int ret;
	
	unique_id = (char *)malloc (UNIQUE_ID_MAX_LEN);
	ret = gethostname(unique_id, HOST_NAME_MAX);
	if (ret) {
		LOG_MSG(LOG_ERROR, "Failed to get host name: %s", 
			strerror (errno));
		strcpy (unique_id, "foo");
	}
	sprintf (unique_id, UNIQUE_ID_FMT, unique_id, getpid());

	LOG_MSG(LOG_DEBUG, "unique_id set to %s", unique_id);
	
}
/* ------------------------------------------------------------------------- */
/** Executes a command using ssh 
 * @param hostname SUT address 
 * @param command Command to execute
 * @return Does not return in success, error code from exec in case of error
 */
int ssh_execute (const char *hostname, const char *command)
{
	int ret;
	char pre_cmd [PID_FILE_MAX_LEN + 80];
	
	sprintf (pre_cmd, "echo $$ > " PID_FILE_FMT ";", unique_id, getpid());
	
	ret = execl(SSHCMD, SSHCMD, SSHCMDARGS, hostname, 
		    pre_cmd, command, (char*)NULL);

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
	char cmd [PID_FILE_MAX_LEN * 2 + 80];
	char file [PID_FILE_MAX_LEN];
	
	pid = fork();
	if (pid > 0) 
	    return 0;
	if (pid < 0)
	    return 1;
	
	sprintf(file, PID_FILE_FMT, unique_id, id);
	sprintf (cmd, "cat %1$s | xargs pkill -9 -P; rm -f %1$s", file);
	
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
	int ret;
	pid_t pid;
	char cmd [PID_FILE_MAX_LEN + 80];

	sprintf (cmd, "rm -f " PID_FILE_FMT, unique_id, id);

	pid = fork();

	if (pid) {
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
