/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Raimo Gratseff <ext-raimo.gratseff@nokia.com>
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
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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

/* Match these to log.h log_message_types */
/* None */

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
/* None */


/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
/* None */

/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/* None */

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */ 
/** Trim string of whitespace and control characters.
 * Remove unwanted whitespace, linefeeds etc. (using isspace()) from the
 * beginning and end of the string (until the first/last non-whitespace
 * character) and control characters (using iscntrl()) from the middle.
 * @param ins The input string. Must not be null.
 * @param outs The output string. Must be at least as long as the input string
 *        and not null.
 * @return Length of the output string
 */
unsigned int trim_string (char *ins, char *outs)
{
	unsigned int ins_i = 0;
	unsigned int ins_end = 0;
	unsigned int outs_i = 0;

	/* make sure input and output strings exist */
	if (ins == 0 || outs == 0) {
		return 0;
	}

	ins_end = strlen(ins);

	/* test if the string is empty */
	if (ins_end == 0) {
		return 0;
	}

	/* find the first non-whitespace character */
	while (1) {
		if (ins_i >= ins_end)
			break;
		if (isspace(ins[ins_i]))
			ins_i += 1;
		else
			break;
	}

	/* find the last non-whitespace character */
	while (1) {
		if (ins_end <= ins_i)
			break;
		if (isspace(ins[ins_end - 1]))
			ins_end -= 1;
		else
			break;
	}

	/* Copy trimmed string to output */
	while (ins_i < ins_end) {
		/* check and skip control characters */
		if (!iscntrl(ins[ins_i])) {
			outs[outs_i] = ins[ins_i];
			outs_i += 1;
		}
		ins_i += 1;
	}
	/* add null termination */
	outs[outs_i] = 0;

	return outs_i;
}


/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

