/* * This file is part of testrunnerlite *
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
typedef struct {
        void (*test_suite) (td_suite *); /**< call back for suite handler    */
	void (*test_set) (td_set *);     /**< call back for set handler      */
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
