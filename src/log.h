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
#include "testrunnerlite.h"
/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */
#define LOG_MSG_MAX_SIZE 2048

/* ------------------------------------------------------------------------- */
/* DATA TYPES */
/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* STRUCTURES */
enum log_levels {
    LOG_LEVEL_SILENT = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVELS_COUNT, /* number of entries in enum */
};

enum log_message_types {
  LOG_ERROR = 0,
  LOG_INFO,
  LOG_DEBUG,
  LOG_WARNING,
  LOG_TYPES_COUNT /* number of entries in enum */
};


/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES */

/* ------------------------------------------------------------------------- */
void log_msg(int type, char *format, ...);
/* ------------------------------------------------------------------------- */
void log_init(testrunner_lite_options *opts);

#endif                          /* LOG_H */
/* End of file */

