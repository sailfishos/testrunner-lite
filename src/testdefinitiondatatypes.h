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

#ifndef TESTDEFINITIONDATATYPES_H
#define TESTDEFINITIONDATATYPES_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
#include "testrunnerlite.h"
#include <libxml/xmlstring.h>
#include <libxml/list.h>

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */

/* ------------------------------------------------------------------------- */
/* DATA TYPES */
/* ------------------------------------------------------------------------- */
typedef struct {
	xmlChar *name;
        xmlChar *domain;
        xmlChar *suite_type;
        xmlChar *level;
        xmlChar *timeout;
        xmlChar *requirement;
        xmlChar *description;
} td_suite;
/* ------------------------------------------------------------------------- */
typedef struct {
	xmlChar   *name;
	xmlListPtr pre_steps;
	xmlListPtr post_steps;
	xmlListPtr cases;
	xmlListPtr environments;
	xmlListPtr gets;
	td_suite  *suite;
} td_set;
/* ------------------------------------------------------------------------- */
typedef struct {
	xmlChar *step;
	xmlChar *c_data;
} td_step;

/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
td_set *td_set_create();
/* ------------------------------------------------------------------------- */
td_step *td_step_create();
/* ------------------------------------------------------------------------- */
void td_step_delete(xmlLinkPtr);
/* ------------------------------------------------------------------------- */

#endif                          /* TESTDEFINITIONDATATYPES_H */
/* End of file */
