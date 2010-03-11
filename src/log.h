/* * This file is part of testrunner-lite *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 *
 * Contact: 
 *
 * This software, including documentation, is protected by copyright
 * controlled by Nokia Corporation. All rights are reserved. Copying,
 * including reproducing, storing, adapting or translating, any or all of
 * this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be
 * disclosed to others without the prior written consent of Nokia.
 *
 */

#ifndef LOG_H
#define LOG_H


/* ------------------------------------------------------------------------- */
/* INCLUDES */
#include <sys/time.h>
#include <time.h>
#include <stdio.h>

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */
/* none */

/* ------------------------------------------------------------------------- */
/* DATA TYPES */
/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* STRUCTURES */
enum log_message_types {
  LOG_ERROR = 0,
  LOG_INFO,
  LOG_DEBUG,
  LOG_TYPES_COUNT // number of entries in enum
};


/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES */

/* ------------------------------------------------------------------------- */
void log_msg(int type, char *format, ...);
/* ------------------------------------------------------------------------- */
void set_verbosity_level(int level);

#endif                          /* LOG_H */
/* End of file */

