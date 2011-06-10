/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <libxml/xmlwriter.h>
#include <dlfcn.h>
#include "testrunnerlite.h"
#include "testdefinitiondatatypes.h"
#include "executor.h"
#include "hwinfo.h"
#include "log.h"

/* ------------------------------------------------------------------------- */
/* EXTERNAL DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* EXTERNAL GLOBAL VARIABLES */
extern int bail_out;

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
LOCAL unsigned char *exec_command (const char *c);
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/** Execute the command provided by plugin
 *  @param cmd the command
 *  @return stdout output of the  command
 */
LOCAL unsigned char *exec_command (const char *cmd)
{
	char *p;
	exec_data edata;

	memset (&edata, 0x0, sizeof (exec_data));
	
	if (cmd == NULL || !strlen (cmd))
		return NULL;

	if (bail_out)
		return NULL;

	init_exec_data (&edata);
	edata.soft_timeout = DEFAULT_TIMEOUT;
	edata.hard_timeout = COMMON_HARD_TIMEOUT;
	LOG_MSG (LOG_INFO, "Getting HW information");
	execute (cmd, &edata);
	
	if (edata.result) {
		LOG_MSG (LOG_ERR, "%s:%s():%d:%s\n", PROGNAME, __FUNCTION__,
			 edata.result, (char *)edata.stderr_data.buffer ?
			 (char *)edata.stderr_data.buffer : 
			 "no info available");
		free (edata.stderr_data.buffer);
		free (edata.stdout_data.buffer);
		return NULL;
	}
	p = strchr  ((char *)edata.stdout_data.buffer, '\n');
	if (p) *p ='\0';
	p = edata.stdout_data.buffer;
	edata.stdout_data.buffer = NULL;
	clean_exec_data(&edata);

	return p;
}

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** Obtain hardware info and save it to argument
 * @param hi container for hardware info
 * @return 0 if basic information can be obtained 1 otherwise
 */
int read_hwinfo (hw_info *hi)
{
	void *plugin;
        const char *(*hwinfo_product) ();
        const char *(*hwinfo_hw_build) ();
        const char *(*hwinfo_extra) ();

	memset (hi, 0x0, sizeof (hw_info));

	plugin = dlopen ("/usr/lib/testrunner-lite-hwinfo.so",
			  RTLD_NOW | RTLD_LOCAL);
	if  (!plugin) {
		LOG_MSG (LOG_WARNING, "failed to load hwinfo plugin %s",
			 dlerror());
		return 1;
	}

	hwinfo_product = (const char*(*)())dlsym(plugin, "hwinfo_product");
	if (!hwinfo_product) {
		LOG_MSG (LOG_WARNING, "no function for hwinfo_product");
	} else {
		hi->product = exec_command (hwinfo_product());
	}

	hwinfo_hw_build = (const char*(*)())dlsym(plugin, "hwinfo_hw_build");
	if (!hwinfo_hw_build) {
		LOG_MSG (LOG_WARNING, "no function for hwinfo_hw_build");
	} else {
		hi->hw_build = exec_command (hwinfo_hw_build());
	}

	hwinfo_extra = (const char*(*)())dlsym(plugin, "hwinfo_extra");
	if (hwinfo_extra) {
		hi->extra = exec_command (hwinfo_extra());
	}

	dlclose (plugin);

	return 0;
}
/* ------------------------------------------------------------------------- */
/** Print hardware info
 * @param hi hw_info data
 */
void print_hwinfo (hw_info *hi)
{
	printf ("Hardware Info:\n");
	printf ("%s %s %s\n", 
		(char *)(hi->product ? hi->product : 
			 (unsigned char *)"<none>"), 
		(char *)(hi->hw_build ? hi->hw_build : 
			 (unsigned char *)"<none>"), 
		(char *)(hi->extra ? hi->extra : 
			 (unsigned char *)"")); 
}
/* ------------------------------------------------------------------------- */
/** Free the allocated data from hw_info
 * @param hi hw_info data
 */
void clean_hwinfo (hw_info *hi)
{
	
        if (hi->product) free (hi->product); 
	if (hi->hw_build) free (hi->hw_build);
	if (hi->extra) free (hi->extra);

	return;
} 

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
