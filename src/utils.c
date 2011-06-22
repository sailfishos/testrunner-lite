/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Contains changes by Wind River Systems, 2011-03-09
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
#include <ctype.h>

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

const unsigned long unicode_rangemin[] = {
	0,
	0,
	0x80,
	0x800,
	0x10000
};

const unsigned long unicode_rangemax[] = {
	0,
	0,
	0x7FF,
	0xFFFF,
	0x10FFFF
};

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

static inline int is_cont_byte(unsigned char data)
{
	return data >> 6 == 0x02;
}

static inline int decode_utf8_length(unsigned char data)
{
	int n;

	for (n = 2; n < 7; ++n) {
		if (((data >> (7-n)) & 0x07) == 0x06) {
			return n;
		}
	}

	return 0;
}

static inline int utf8_decode(const unsigned char **data, int maxlen)
{
	const unsigned char *p = *data;
	unsigned long codepoint = 0L;
	int n, l;

	l = decode_utf8_length(*p);

	if (l > 1 && l < maxlen + 1) {
		codepoint = *p++ & 0x1F >> (l-2);
		for (n = l - 1; n && *p; --n, ++p) {
			if (!is_cont_byte(*p)) {
				return 0;
			}
			codepoint <<= 6;
			codepoint += *p & 0x3F;
		}

		/* null character before decoding completed */
		if (n) {
			return 0;
		}

		/* check code point value ranges */
		if (codepoint < unicode_rangemin[l]) {
			return 0;
		}

		if (codepoint > unicode_rangemax[l]) {
			return 0;
		}

	} else {
		return 0;
	}

	*data = p;
	return 1;
}


/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */ 
/** Replace a substring with another string.
 * Given a string, replace up to one occurrence of a substring with another
 * string.
 * @param orig The original string
 * @param from The substring to replace
 * @param to The substring to substitute in place of the old substring
 */
char *replace_string (const char *orig, const char *from, const char *to)
{
	char *from_pos;
	char *p;
	char *result;
	size_t len_before;
	size_t result_size;

	if (orig == NULL || from == NULL || to == NULL)
		return NULL;

	result_size = strlen(orig) + 1;
	from_pos = strstr(orig, from);
	if (from_pos != NULL)
		result_size = result_size - strlen(from) + strlen(to);

	result = (char *)malloc (result_size);
	if (result == NULL) {
		fprintf (stderr, "malloc failed");
		return NULL;
	}

	if (from_pos == NULL) {
		/* nothing to replace */
		strncpy(result, orig, result_size);
		return result;
	}

	len_before = from_pos - orig;

	/* copy original string before match */
	p = strncpy(result, orig, result_size);

	/* insert new string */
	p = strncpy(p + len_before, to, result_size - len_before);

	/* append original string after match */
	strncpy(p + strlen(to), from_pos + strlen(from), 
		result_size - len_before - strlen(to));

	return result;
}

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
/* ------------------------------------------------------------------------- */
/** Check if a string list contains certain value
 * @param *list List of values delimited by a character defined in delim
 * @param *value The value searched from the list
 * @param *delim Set of delimiter characters
 * @return 1 if the value is found from the list of 0 if not
 */
int list_contains(const char *list, const char *value, const char* delim)
{
	int ret = 0;
	char *str = strdup(list);
	char *c = strtok(str, delim);

	while (c != NULL) {
		if (strcmp(value, c) == 0) {
			ret = 1;
			break;
		}
		c = strtok(NULL, delim);
	}

	free(str);
	return ret;
}

/** 
 * Check data is valid UTF-8. If invalid data is found, function
 * returns immediately. Maximum number of allowed bytes for a UTF-8
 * sequence is given in argument maxlen (normally 4 when maximum code
 * point value is U+10FFFF). Validation ends when null character is found.
 * 
 * @param data Data to be validated with terminating null
 * @param maxlen Maximum allowed bytes in a UTF-8 sequence (1-4)
 * 
 * @return 1 when data is valid UTF-8 and 0 if not
 */
int utf8_validity_check(const unsigned char *data, int maxlen)
{
	const unsigned char *p = data;

	if (maxlen < 1 || maxlen > 4) {
		return 0;
	}

	while (*p != '\0') {
		if (isascii(*p)) {
			++p;
			continue;
		} else if (is_cont_byte(*p)) {
			/* unexpected continuation byte */
			return 0;
		} else if (!utf8_decode(&p, maxlen)) {
			return 0;
		}
	}

	return 1;
}


/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */

