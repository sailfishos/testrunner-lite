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
#include <stdio.h>
#include <errno.h>
#include <string.h>
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
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** Executes a command using ssh 
 * @param command Command to execute
 * @return Does not return in success, error code from exec in case of error
 */
int ssh_execute (const char *hostname, const char *command)
{
	int ret;

	ret = execl(SSHCMD, SSHCMD, hostname, 
		    "echo \$\$ > /tmp/testrunner.pid;", command, (char*)NULL);

	LOG_MSG(LOG_ERROR, "execl() failed %s", strerror (errno));

	return ret;
}
/* ------------------------------------------------------------------------- */
/** Tries to kill program started by ssh
 *  
 */
int ssh_softkill (const char *hostname)
{
	int ret;


	ret = execl(SSHCMD, SSHCMD, hostname, 
		    "cat /tmp/testrunner.pid | xargs pkill -P ", (char*)NULL);

	LOG_MSG(LOG_ERROR, "execl() failed %s", strerror (errno));

	return ret;
}
/* ------------------------------------------------------------------------- */
/** Tries to kill -9 program started by ssh
 *  
 */
int ssh_raidkill (const char *hostname)
{
	int ret;


	ret = execl(SSHCMD, SSHCMD, hostname, 
		    "cat /tmp/testrunner.pid | xargs pkill -9 -P ", 
		    (char*)NULL);

	LOG_MSG(LOG_ERROR, "execl() failed %s", strerror (errno));

	return ret;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
