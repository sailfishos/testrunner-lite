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

#ifndef TESTDEFINITIONPARSER_H
#define TESTDEFINITIONPARSER_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
#include "testrunnerlite.h"
#include "testdefinitiondatatypes.h"
#include <libxml/xmlstring.h>

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */

/* ------------------------------------------------------------------------- */
/* DATA TYPES */
/* ------------------------------------------------------------------------- */
/** Callbacks for parser. Parser uses these to pass parsed data to caller. */
typedef struct {
        void (*test_suite) (td_suite *); /**< callback for suite handler    */
        void (*test_suite_end) (); /**< callback for suite end handler */
        void (*test_suite_description) (char *); /**< callback for suite 
						    description */

	void (*test_set) (td_set *);     /**< callback for set handler      */
} td_parser_callbacks;
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
int parse_test_definition(testrunner_lite_options *);
/* ------------------------------------------------------------------------- */
int td_reader_init(testrunner_lite_options *);
/* ------------------------------------------------------------------------- */
void td_reader_close(void);
/* ------------------------------------------------------------------------- */
int td_register_callbacks(td_parser_callbacks *);
/* ------------------------------------------------------------------------- */
int td_next_node(void);
/* ------------------------------------------------------------------------- */

#endif                          /* TESTDEFINITIONPARSER_H */
/* End of file */
