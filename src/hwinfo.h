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

#ifndef HWINFO_H
#define HWINFO_H

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
typedef struct {
        char *product; 
	char *hw_build;
        char *nolo;
        char *boot_mode;
        char *production_sn;
        char *product_code; 
	char *basic_product_code;
} hw_info;
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
int read_hwinfo (hw_info *);
/* ------------------------------------------------------------------------- */
void print_hwinfo (hw_info *);
/* ------------------------------------------------------------------------- */
void clean_hwinfo (hw_info *);
/* ------------------------------------------------------------------------- */
#endif                          /* HWINFO_H */
/* End of file */
