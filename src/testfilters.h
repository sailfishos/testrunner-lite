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

#ifndef TESTFILTERS_H
#define TESTFILTERS_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
#include <libxml/xmlstring.h>
#include <libxml/list.h>
#include "testrunnerlite.h"
#include "testdefinitiondatatypes.h"
/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* DATA TYPES */
/* ------------------------------------------------------------------------- */
/** Test filter */
typedef struct test_filter_ {
	int         exclude; /**< set if the filter is negated with - */ 
	xmlChar    *key;  /**< filter key */
	xmlListPtr  value_list; /**< filter value list */
        int       (*filter) (struct test_filter_ *, const void *); /**< filter 
								    function */
} test_filter;
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
void init_filters();
/* ------------------------------------------------------------------------- */
int parse_filter_string (char *);
/* ------------------------------------------------------------------------- */
int filter_suite (td_suite *);
/* ------------------------------------------------------------------------- */
int filter_set (td_set *);
/* ------------------------------------------------------------------------- */
int filter_case (td_case *);
/* ------------------------------------------------------------------------- */
void cleanup_filters();
/* ------------------------------------------------------------------------- */

#endif                          /* TESTFILTER_H */
/* End of file */
