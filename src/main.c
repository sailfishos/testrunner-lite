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

/* ------------------------------------------------------------------------- */
/* INCLUDE FILES */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "testrunnerlite.h"
#include "testdefinitionparser.h"
#include "testresultlogger.h"
#include "executor.h"

/* ------------------------------------------------------------------------- */
/* EXTERNAL DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* EXTERNAL GLOBAL VARIABLES */
extern char* optarg;

/* ------------------------------------------------------------------------- */
/* EXTERNAL FUNCTION PROTOTYPES */
/* None */

/* ------------------------------------------------------------------------- */
/* GLOBAL VARIABLES */
/* None */

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL GLOBAL VARIABLES */
td_suite *current_suite = NULL;
testrunner_lite_options opts;

/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL void usage();
/* ------------------------------------------------------------------------- */
LOCAL void process_suite(td_suite *);
/* ------------------------------------------------------------------------- */
LOCAL void process_set(td_set *);
/* ------------------------------------------------------------------------- */
LOCAL int process_case (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int step_execute (const void *, const void *);
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/** Print short help.
 */
LOCAL void usage()
{
	printf ("\nUsage: testrunner-lite [options]\n");
	printf ("Example: testrunner-lite -f tests.xml -o ~/results.xml "
		"-e hardware\n");
	printf ("\nOptions:\n");
	printf ("  -h, --help\tShow this help message and exit.\n");
	printf ("  -f FILE, --file=FILE\tInput file with test definitions "
		"in XML (required).\n");
	printf ("  -o FILE, --output=FILE\n\t\t"
		"Output file for test results (required).\n");
	printf ("  -r FORMAT, --format=FORMAT\n\t\t"
		"Output file format. FORMAT can be xml or text.\n\t\t"
		"Default: xml\n");
	printf ("  -e ENVIRONMENT, --environment=ENVIRONMENT\n\t\t"
		"Target test environment. Default: hardware\n");
	printf ("  -v, --verbose\tEnable verbosity mode to show "
		"debug messages.\n");
	printf ("  -a, --automatic\tEnable only automatic tests "
		"to be executed.\n");
	printf ("  -m, --manual\tEnable only manual tests to be executed.\n");
	
	printf ("  -l FILTER, --filter=FILTER\n\t\t"
		"Filtering option to select tests (not) to be executed.\n\t\t"
		"E.g. '-testcase=bad_test -type=unknown' first disables\n\t\t"
		"test case named as bad_test. Next, all tests with type\n\t\t"
		"unknown are disabled. The remaining tests will be\n\t\t"
		"executed.\n");
	printf ("  -c, --ci\tDisable validation of test "
		"definition against schema.\n");
	printf ("  -s, --semantic\n\t\tEnable validation of test "
		"definition against stricter (semantics) schema.\n");
	printf ("  -A, --validate-only\n\t\tDo only input xml validation, do not execute tests.\n");

	return;
}
/* ------------------------------------------------------------------------- */
/** Process step data. execute one step from case.
 *  @param data step data
 *  @param user case data
 *  @return 1 always
 */
LOCAL int step_execute (const void *data, const void *user) 
{
	int fail = 0;
	td_step *step = (td_step *)data;
	td_case *c = (td_case *)user;
	exec_data edata;

	memset (&edata, 0x0, sizeof (exec_data));

	if (c->gen.manual && !opts.run_manual) 
		return 1;
	 
	if (!c->gen.manual && !opts.run_automatic)
		return 1;
	
	init_exec_data(&edata);
	
	edata.soft_timeout = c->gen.timeout;
	edata.hard_timeout = edata.soft_timeout + 5;

	if (step->step) {
		execute((char*)step->step, &edata);

		if (step->stdout_) free (step->stdout_);
		if (step->stderr_) free (step->stderr_);
		if (step->failure_info) free (step->failure_info);

		if (edata.stdout_data.buffer) {
			step->stdout_ = edata.stdout_data.buffer;
		}
		if (edata.stderr_data.buffer) {
			step->stderr_ = edata.stderr_data.buffer;
		}
		if (edata.failure_info.buffer) {
			step->failure_info = edata.failure_info.buffer;
		}

		step->return_code = edata.result;
		step->start = edata.start_time;
		step->end = edata.end_time;
		if (step->return_code != step->expected_result) {
			printf ("step %s return %d expected %d\n",
				step->step, step->return_code, 
				step->expected_result);
			fail = 1;
		}
	}
	if (fail)
		c->passed = 0;
	

	return 1;
}
/* ------------------------------------------------------------------------- */
/** Process case data. execute steps in case.
 *  @param data case data
 *  @param user set data
 *  @return 1 always
 */
LOCAL int process_case (const void *data, const void *user) 
{

	td_case *c = (td_case *)data;
	td_set *set = (td_set *)user;

	c->passed = 1;
	c->gen.timeout = c->gen.timeout ? c->gen.timeout : 
		(set->gen.timeout ? 
		 set->gen.timeout : current_suite->gen.timeout);
	if (c->gen.timeout == 0)
		c->gen.timeout = 90; /* the default one */
	 
	    
	xmlListWalk (set->pre_steps, step_execute, data);
	/* execute test steps only if pre-steps passed */
	if (c->passed)
		xmlListWalk (c->steps, step_execute, data);
	xmlListWalk (set->post_steps, step_execute, data);

	return 1;
}
/* ------------------------------------------------------------------------- */
/** Do processing on suite, currently just writes the pre suite tag to results
 *  @param s suite data
 */
LOCAL void process_suite (td_suite *s)
{
	write_pre_suite_tag (s);
	current_suite = s;
	
}
/* ------------------------------------------------------------------------- */
/** Suite end function, write suite and delete the current_suite
 */
LOCAL void end_suite ()
{
	write_post_suite_tag ();
	td_suite_delete (current_suite);
	current_suite = NULL;
}
/* ------------------------------------------------------------------------- */
/** Process set data. Walk through cases and free set when done.
 *  @param s set data
 */
LOCAL void process_set (td_set *s)
{
	/*
	** Check that the set is supposed to be executed in the current env
	*/
	if (xmlListSize(s->environments) > 0) {
	        if (!xmlListSearch (s->environments, opts.environment)) {
			goto skip;
		}
	}
	xmlListWalk (s->cases, process_case, s);
	write_pre_set_tag (s);
	write_post_set_tag (s);
 skip:
	td_set_delete (s);
	return;
}
/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** main() for testrunnerlite - handle command line switches and call parser
 *  @param argc argument count
 *  @param argv arguments
 *  @param envp environment
 *  @return EXIT_SUCCESS if all is well
 */
int main (int argc, char *argv[], char *envp[])
{
	int h_flag = 0, v_flag = 0, a_flag = 0, m_flag = 0, A_flag = 0;
	int opt_char, option_idx;
	FILE *ifile = NULL, *ofile = NULL;
	int retval = EXIT_SUCCESS;
	td_parser_callbacks cbs;

	struct option testrunnerlite_options[] =
		{
			{"help", no_argument, &h_flag, 1},
			{"file", required_argument, NULL, 'f'},
			{"output", required_argument, NULL, 'o'},
			{"format", required_argument, NULL, 'r'},
			{"environment", required_argument, NULL, 'e'},
			{"verbose", no_argument, &v_flag, 1},
			{"automatic", no_argument, &a_flag, 1},
			{"manual", no_argument, &m_flag, 1},
			{"filter", required_argument, NULL, 'l'},
			{"ci", no_argument, &opts.disable_schema},
			{"semantic", no_argument, &opts.semantic_schema},
			{"validate-only", no_argument, &A_flag}

		};


	LIBXML_TEST_VERSION

	memset (&opts, 0x0, sizeof(testrunner_lite_options));
        memset (&cbs, 0x0, sizeof(td_parser_callbacks));
	opts.output_type = OUTPUT_TYPE_XML;
	opts.run_automatic = opts.run_manual = 1;
	
	while (1) {
		option_idx = 0;
     
		opt_char = getopt_long (argc, argv, ":hvaAsmcf:o:e:l:r:",
					testrunnerlite_options, &option_idx);
		if (opt_char == -1)
			break;
		
		switch (opt_char)
		{
		case 'h':
			h_flag = 1;
			break;
		case 'v':
			v_flag = 1;
			break;
		case 'a':
			a_flag = 1;
			break;
		case 'm':
			m_flag = 1;
			break;
		case 'c':
			opts.disable_schema = 1;
			break;
		case 's':
			opts.semantic_schema = 1;
			break;
		case 'r':
			if (!strcmp (optarg, "xml"))
				opts.output_type = OUTPUT_TYPE_XML;
			else if (!strcmp (optarg, "text"))
				opts.output_type = OUTPUT_TYPE_TXT;
			else {
				fprintf (stderr, "%s Unknown format %s\n",
					 PROGNAME, optarg);
				retval = EXIT_FAILURE;
				goto OUT;
			}
			break;
		case 'f':
			ifile = fopen (optarg, "r");
			if (!ifile) {
				fprintf (stderr, "%s Failed to open %s %s\n",
					 PROGNAME, optarg, strerror (errno));
				retval = EXIT_FAILURE;
				goto OUT;
			}
			fclose (ifile);
			opts.input_filename = malloc (strlen (optarg) + 1);
			strcpy (opts.input_filename, optarg); 
			break;
		case 'o':
			ofile = fopen (optarg, "w");
			if (!ofile) {
				fprintf (stderr, "%s Failed to open %s %s\n",
					 PROGNAME, optarg, strerror (errno));
				retval = EXIT_FAILURE;
				goto OUT;
			}
			fclose (ofile);
			opts.output_filename = malloc (strlen (optarg) + 1);
			strcpy (opts.output_filename, optarg); 
			break;
		case 'e':
			opts.environment = malloc (strlen (optarg) + 1);
			strcpy (opts.environment, optarg); 
			break;
		case 'A':
			A_flag = 1;
			break;
		case ':':
			fprintf (stderr, "%s missing argument - exiting\n",
				 PROGNAME);
			retval = EXIT_FAILURE;
			goto OUT;
			break;
		case '?':
			fprintf (stderr, "%s unknown option - exiting\n",
				 PROGNAME);
			retval = EXIT_FAILURE;
			goto OUT;
			break;

		}
	}
	
	/*
	 * Do some post-validation for the options
	 */
	if (h_flag) {
		usage();
		goto OUT;
	}
	
	if (m_flag && a_flag) {
		fprintf (stderr, 
			 "%s: -a and -m are mutually exclusive\n",
			 PROGNAME);
		retval = EXIT_FAILURE;
		goto OUT;
	}

	if (m_flag) 
		opts.run_automatic = 0;
	if (a_flag)
		opts.run_manual = 0;

	if (!ifile) {
		fprintf (stderr, 
			 "%s: mandatory option missing -f input_file\n",
			 PROGNAME);
		retval = EXIT_FAILURE;
		goto OUT;
	}
	/*
	 * Validate the input xml
	 */
	retval = parse_test_definition (&opts);
	if (A_flag) {
		printf ("%s: %s %s\n", PROGNAME, opts.input_filename, retval ?
			"fails to validate" : "validates");
		goto OUT;
	}
	if (retval)
		goto OUT;

	if (!ofile) {
		fprintf (stderr, 
			 "%s: mandatory option missing -o output_file\n",
			 PROGNAME);
		retval = EXIT_FAILURE;
		goto OUT;
	}

	if (!opts.environment) {
		opts.environment = (char *)malloc (strlen ("hardware") + 1);
		strcpy (opts.environment, "hardware");
	}

	
	/*
	** Set callbacks for parser
	*/
	cbs.test_suite = process_suite;
	cbs.test_suite_end = end_suite;
	cbs.test_set = process_set;

	retval = td_register_callbacks (&cbs);
	
	/*
	** Initialize the reader
	*/
	retval = td_reader_init(&opts);
	if (retval)
		goto OUT;
	/*
	** Initialize result logger
	*/
	retval =  init_result_logger(&opts);
	if (retval)
		goto OUT;
	
	/*
	** Call td_next_node untill error occurs or the end of data is reached
	*/
	while (td_next_node() == 0);
	
	td_reader_close();
	close_result_logger();
	
OUT:
	if (opts.input_filename) free (opts.input_filename);
	if (opts.output_filename) free (opts.output_filename);
	if (opts.environment) free (opts.environment);
	
	return retval;
}	


/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
