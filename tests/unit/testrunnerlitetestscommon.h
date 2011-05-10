/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Riku Halonen <riku.halonen@nokia.com>
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

#ifndef TESTRUNNERLITE_SUITES_H
#define TESTRUNNERLITE_SUITES_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
#include <check.h>
#include "testrunnerlite.h"
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
#define TESTDATA_ENTITY_SUBSTITUTION    "/usr/share/testrunner-lite-tests/testdata/entity_substitution.xml"
#define TESTDATA_NON_UTF8_XML_1 "/usr/share/testrunner-lite-tests/testdata/testrunner-tests-non-utf8.xml"
#define TESTDATA_BG_XML "/usr/share/testrunner-lite-tests/testdata/testrunner-tests-bg.xml"
#define TESTDATA_PRE_STEP_FAIL_XML  "/usr/share/testrunner-lite-tests/testdata/testrunner-tests-pre_step-fail.xml"
#define TESTDATA_FILTER_TESTS_XML  "/usr/share/testrunner-lite-tests/testdata/filter_tests.xml"
#define TESTDATA_ENVIRONMENT_TESTS_XML  "/usr/share/testrunner-lite-tests/testdata/testrunner-tests-environment.xml"
#define TESTDATA_RESUME_TEST_XML  "/usr/share/testrunner-lite-tests/testdata/resumetest.xml"
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
Suite *make_manualtestexecutor_suite(void);
Suite *make_testfilter_suite(void);
/* ------------------------------------------------------------------------- */
#endif                          /* TESTRUNNERLITE_SUITES */
/* End of file */
