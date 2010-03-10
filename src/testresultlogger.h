/* * This file is part of testrunner-lite *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 *
 * Contact: John Doe <John.Doe@nokia.com>
 *
 * This software, including documentation, is protected by copyright
 * controlled by Nokia Corporation. All rights are reserved. Copying,
 * including reproducing, storing, adapting or translating, any or all of
 * this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be
 * disclosed to others without the prior written consent of Nokia.
 *
 */

#ifndef TESTRESULTLOGGER_H
#define TESTRESULTLOGGER_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
#include "testrunnerlite.h"
#include "testdefinitiondatatypes.h"
#include "hwinfo.h"

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* DATA TYPES */
/* ------------------------------------------------------------------------- */
/* None */

/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES */
int init_result_logger (testrunner_lite_options *, hw_info *);
/* ------------------------------------------------------------------------- */
void close_result_logger (void);
/* ------------------------------------------------------------------------- */
int write_pre_suite_tag (td_suite *);
/* ------------------------------------------------------------------------- */
int write_post_suite_tag ();
/* ------------------------------------------------------------------------- */
int write_pre_set_tag (td_set *);
/* ------------------------------------------------------------------------- */
int write_post_set_tag (td_set *);
/* ------------------------------------------------------------------------- */

#endif                          /* TESTRESULTLOGGER_H */
/* End of file */
