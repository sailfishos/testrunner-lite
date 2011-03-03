/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Contains changes by Wind River Systems, 2011-03-09
 *
 * Contact: Sampo Saaristo <sampo.saaristo@sofica.fi>
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

#ifndef REMOTE_EXECUTOR_H
#define REMOTE_EXECUTOR_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
#include "testrunnerlite.h"

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
/* ------------------------------------------------------------------------- */
int remote_executor_init (const char *executor);
/* ------------------------------------------------------------------------- */
int remote_execute (const char *executor, const char *command);
/* ------------------------------------------------------------------------- */
int remote_kill (const char *executor, pid_t id);
/* ------------------------------------------------------------------------- */
int remote_check_conn (const char *executor);
/* ------------------------------------------------------------------------- */
int remote_clean (const char *executor, pid_t id);
/* ------------------------------------------------------------------------- */
int remote_executor_close (void);
/* ------------------------------------------------------------------------- */
#endif                          /* REMOTE_EXECUTOR_H */
/* End of file */
