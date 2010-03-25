/* * This file is part of <tool name> *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 *
 * Contact: Test-tools-dev <Test-tools-dev@projects.maemo.org>
 *
 * This software, including documentation, is protected by copyright
 * controlled by Nokia Corporation. All rights are reserved. Copying,
 * including reproducing, storing, adapting or translating, any or all of
 * this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be
 * disclosed to others without the prior written consent of Nokia.
 *
 */

#ifndef TESTRUNNERLITE_SUITES_H
#define TESTRUNNERLITE_SUITES_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
#include <check.h>

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */
#define TESTDATA_VALID_XML_1    "/usr/share/testrunner-lite-tests/testdata/sample_input_file.xml"
#define TESTDATA_INVALID_XML_1    "/usr/share/testrunner-lite-tests/testdata/invalid.xml"
#define TESTDATA_INVALID_SEMANTIC_XML_1    "/usr/share/testrunner-lite-tests/testdata/testrunner-tests-semantic_invalid.xml"
#define TESTDATA_SIMPLE_XML_1   "/usr/share/testrunner-lite-tests/testdata/simple.xml"

#define TESTDATA_GET_XML_1    "/usr/share/testrunner-lite-tests/testdata/testrunner-tests-get.xml"

#define TESTDATA_UTF8_XML_1    "/usr/share/testrunner-lite-tests/testdata/testrunner-tests-utf8.xml"

#define TESTRUNNERLITE_BIN  "/usr/bin/testrunner-lite"

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
Suite *make_testdefinitionparser_suite(void);
Suite *make_argumentparser_suite(void);
Suite *make_testresultlogger_suite(void);
Suite *make_testexecutor_suite(void);
Suite *make_features_suite(void);
/* ------------------------------------------------------------------------- */
#endif                          /* TESTRUNNERLITE_SUITES */
/* End of file */
