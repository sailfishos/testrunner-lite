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
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
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
LOCAL xmlChar *get_comments ();
/* ------------------------------------------------------------------------- */
LOCAL int check_user_input(char *buff, int *result);
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/** Check that user input is valid.
 * @param buff user input
 * @param result [OUT] pass result for caller 0 for pass 1 for fail
 * @return 0 for valid input 1 for invalid
 */
LOCAL int check_user_input(char *buff, int *result)
{
	char *p;

	*result = 1;
	p = strchr (buff, '\n');
	if (p)
		*p = '\0';

	if (strlen (buff) == 0)
		return 1;

	if (strlen (buff) == 1) {
		if (buff[0] == 'p' || buff[0] == 'P') {
			*result = 0;
			return 0;
		}
		if (buff[0] == 'f' || buff[0] == 'F') {
			*result = 1;
			return 0;
		}
	}

	if (strlen (buff) != 4)
		return 1;
	
	if (!strcasecmp (buff, "pass")) {
		*result = 0;
		return 0;
	}
		
	if (!strcasecmp (buff, "fail")) {
		*result = 1;
		return 0;
	}

	return 1;
}
/* ------------------------------------------------------------------------- */
/** Prompt user for comments
 * @return the comments in buffer or NULL in case of failure
 */
LOCAL xmlChar *get_comments ()
{
	char buff [4096], *p;
	buff [0] = '\0';
	xmlChar *ret = NULL;
	printf ("Please enter additional comments (ENTER to finish): ");
	p = fgets (buff, 4096, stdin);
	if (p)  {
		p = strchr (buff, '\n');
		if (p)
			*p = '\0';
		ret = xmlCharStrdup (buff);
	}
	return ret;
}
/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** Print information before manual test case execution
 *  @param c test case data
 */
void pre_manual (td_case *c)
{
	printf ("\nDescription of test case:\n%s\n",
		(char *)((char *)c->gen.description ? 
			 (char *)c->gen.description : " "));

}
/* ------------------------------------------------------------------------- */
/** Execute manual test step
 * @param step test step data
 * @return 0 if step is passed 
 */
int execute_manual (td_step *step)
{
	char buff [256], *p;
	int ret = 0;

	step->start = time (NULL);
	printf ("--- Execute test step ---\n");
	printf ("Description: ");
	if (step->step)
		printf ("%s\n", step->step);
	
	buff [0] = '\0';
	printf ("Please enter the result ([P/p]ass or [F/f]ail): ");
	p = fgets (buff, 256, stdin);
	while (check_user_input(buff, &ret)) {
		printf ("Invalid input.\n");
		p = fgets (buff, 256, stdin);
	}
	step->end = time (NULL);
	
	return ret;
}
/* ------------------------------------------------------------------------- */
/** Do manual test case post processing.
 *  @param c test case data
 */
void post_manual (td_case *c)
{
	printf ("--- Test steps executed, case is %s ---\n",
		c->passed ? "PASSED" : "FAILED");

	c->comment = get_comments();
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
