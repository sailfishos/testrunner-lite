/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Riku Halonen <riku.halonen@nokia.com>
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
#include <stdlib.h>
#include <sys/time.h>

#include "testrunnerlitetestscommon.h"

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
struct timeval created;

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

typedef Suite *(make_suite(void));

typedef struct {
	char* name;
	make_suite *func;
} suite_functions_t;

suite_functions_t suite_functions[] = {
	{ "argumentparser", make_argumentparser_suite },
	{ "features", make_features_suite },
	{ "filters", make_testfilter_suite },
	{ "manual_executor", make_manualtestexecutor_suite },
	{ "testdefinitionparser", make_testdefinitionparser_suite },
	{ "testexecutor", make_testexecutor_suite },
	{ "testresultlogger", make_testresultlogger_suite }
};

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
/* None */

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
int main (int argc, char **argv)
{
	int number_failed;
	int i;
	Suite *s = suite_create ("master");
	SRunner *sr = srunner_create (s);
	gettimeofday (&created, NULL);

	if (argc > 1) {
		for (i = 0;i < sizeof(suite_functions)/
				sizeof(suite_functions_t);i++) {
			if (strcmp(suite_functions[i].name, argv[1]) == 0) {

				srunner_add_suite
					(sr, suite_functions[i].func());
			}
		}
	} else {
		for (i = 0;i < sizeof(suite_functions)/
				sizeof(suite_functions_t);i++) {
			srunner_add_suite(sr, suite_functions[i].func());
		}
	}

	srunner_run_all (sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
