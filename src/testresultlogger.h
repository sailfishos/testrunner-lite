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

#ifndef TESTRESULTLOGGER_H
#define TESTRESULTLOGGER_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
#include "testrunnerlite.h"
#include "testdefinitiondatatypes.h"
#include "hwinfo.h"

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
#define FAILURE_INFO_MAX 64

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
int write_td_start (td_td *);
/* ------------------------------------------------------------------------- */
int write_td_start (td_td *);
/* ------------------------------------------------------------------------- */
int write_pre_suite (td_suite *);
/* ------------------------------------------------------------------------- */
int write_post_suite ();
/* ------------------------------------------------------------------------- */
int write_pre_set (td_set *);
/* ------------------------------------------------------------------------- */
int write_post_set (td_set *);
/* ------------------------------------------------------------------------- */
int xml_end_element (void);
/* ------------------------------------------------------------------------- */
#endif                          /* TESTRESULTLOGGER_H */
/* End of file */
