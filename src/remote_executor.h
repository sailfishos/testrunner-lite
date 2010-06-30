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
void ssh_executor_init (const char *hostname);
/* ------------------------------------------------------------------------- */
int ssh_execute (const char *hostname, const char *command);
/* ------------------------------------------------------------------------- */
int ssh_kill (const char *hostname, pid_t id);
/* ------------------------------------------------------------------------- */
int ssh_check_conn (const char *hostname);
/* ------------------------------------------------------------------------- */
void ssh_clean (const char *hostname, pid_t id);
/* ------------------------------------------------------------------------- */
void ssh_executor_close (const char *hostname);
/* ------------------------------------------------------------------------- */
#endif                          /* REMOTE_EXECUTOR_H */
/* End of file */
