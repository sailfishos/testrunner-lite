/* * This file is part of testrunner-lite *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 *
 * Contact: Sampo Saaristo <ext-sampo.1.saaristo@nokia.com>
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
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <libxml/xmlwriter.h>
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
LOCAL unsigned char *get_sysinfo (const char *);
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/** Execute sysinfo-tool --get for given key
 *  @param key to be passed to sysinfo-tool e.g. "component/product"
 *  @return stdout output of the sysinfo-tool command
 */
LOCAL unsigned char *get_sysinfo (const char *key)
{
	char *cmd, *p;
	exec_data edata;
	memset (&edata, 0x0, sizeof (exec_data));
	init_exec_data (&edata);
	
	edata.soft_timeout = 5;
	edata.hard_timeout = COMMON_HARD_TIMEOUT;

	cmd = (char *)malloc (strlen ("sysinfo-tool --get ") + 
			      strlen (key) + 1);
	sprintf (cmd, "sysinfo-tool --get %s", key);
	execute (cmd, &edata);
	
	if (edata.result) {
		log_msg (LOG_ERROR, "%s:%s():%d:%s\n", PROGNAME, __FUNCTION__,
			 edata.result, (char *)edata.stderr_data.buffer ?
			 (char *)edata.stderr_data.buffer : 
			 "no info available");
		free (edata.stderr_data.buffer);
		free (edata.stdout_data.buffer);
		free (cmd);
		return NULL;
	}
	p = strchr  ((char *)edata.stdout_data.buffer, '\n');
	if (p) *p ='\0';
	free (cmd);
	
	return edata.stdout_data.buffer;
}

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/** Obtain hardware info and save it to argument
 * @param hi container for hardware info
 * @return 0 if basic information can be obtained 1 otherwise
 */
int read_hwinfo (hw_info *hi)
{
	memset (hi, 0x0, sizeof (hw_info));
	
	hi->product = get_sysinfo("component/product");
	hi->hw_build = get_sysinfo("component/hw-build");
	if (!hi->product || !hi->hw_build) {
		fprintf (stderr, "%s: Failed to read basic HW "
			 "information from sysinfo.\n", PROGNAME);
		return 1;
	} else {
		log_msg (LOG_INFO, "Hardware: Product: %s HWbuild: %s",
			 hi->product, hi->hw_build);
	}
	
	hi->nolo = get_sysinfo("component/nolo");
	hi->boot_mode = get_sysinfo("component/boot-mode");
	hi->production_sn = get_sysinfo("device/production-sn");
	hi->product_code = get_sysinfo("device/product-code");
	hi->basic_product_code = get_sysinfo("device/basic-product-code");
	
	if (!hi->nolo)
		return 0;
	
	log_msg(LOG_INFO, "Hardware: Nolo: %s"
		" Boot_mode: %s"
		" Production_sn: %s" 
		" Product_code: %s"
		" Basic_product_code: %s",
		hi->nolo, 
		hi->boot_mode ? hi->boot_mode : (unsigned char *)"unknown",
		hi->production_sn ? hi->production_sn : 
		(unsigned char *)"unknown",
		hi->product_code ? hi->product_code : 
		(unsigned char *)"unknown",
		hi->basic_product_code ? hi->product_code : 
		(unsigned char *)"unknown");
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Print hardware info
 * @param hi hw_info data
 */
void print_hwinfo (hw_info *hi)
{
	printf ("Hardware Info:\n");
	printf ("%s %s %s %s\n", 
		(char *)(hi->product ? hi->product : 
			 (unsigned char *)"<none>"), 
		(char *)(hi->hw_build ? hi->hw_build : 
			 (unsigned char *)"<none>"), 
		(char *)(hi->nolo ? hi->nolo : 
			 (unsigned char *)"<none>"), 
		(char *)(hi->boot_mode ? hi->boot_mode : 
			 (unsigned char *)"<none>"));
	printf ("%s %s %s\n", 
		(char *)(hi->production_sn ? hi->production_sn : 
			 (unsigned char *)"<none>") , 
		(char *)(hi->product_code ? hi->product_code : 
			 (unsigned char *)"<none>"),
		(char *)(hi->basic_product_code ? hi->basic_product_code : 
			 (unsigned char *)"<none>"));
}
/* ------------------------------------------------------------------------- */
/** Free the allocated data from hw_info
 * @param hi hw_info data
 */
void clean_hwinfo (hw_info *hi)
{
	
        if (hi->product) free (hi->product); 
	if (hi->hw_build) free (hi->hw_build);
	if (hi->nolo) free (hi->nolo);
	if (hi->boot_mode) free (hi->boot_mode);
	if (hi->production_sn) free (hi->production_sn);
	if (hi->product_code) free (hi->product_code);
	if (hi->basic_product_code) free (hi->basic_product_code);
	return;
} 

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
