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
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "testrunnerlite.h"
#include "testdefinitionparser.h"
#include "testresultlogger.h"
#include "executor.h"
#include "hwinfo.h"
#include "log.h"

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
hw_info hwinfo;
int passcount = 0;
int casecount = 0;


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
LOCAL int create_output_folder ();
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
 *  @return 1 if step is passed 0 if not
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
			log_msg (LOG_INFO, "FAILURE INFO %s",
				 step->failure_info);
		}

		step->return_code = edata.result;
		step->start = edata.start_time;
		step->end = edata.end_time;
		if (step->return_code != step->expected_result) {
			log_msg (LOG_INFO, "STEP: %s return %d expected %d\n",
				 step->step, step->return_code, 
				 step->expected_result);
			fail = 1;
		}
	}
	if (fail)
		c->passed = 0;
	

	return !fail;
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
	
	log_msg (LOG_INFO, "Starting test case %s", c->gen.name);
	casecount++;
	

	c->passed = 1;
	c->gen.timeout = c->gen.timeout ? c->gen.timeout : 
		(set->gen.timeout ? 
		 set->gen.timeout : current_suite->gen.timeout);
	if (c->gen.timeout == 0)
		c->gen.timeout = 90; /* the default one */
	 
	if (xmlListSize (set->pre_steps) > 0) {
		log_msg (LOG_INFO, "Executing pre steps");
		xmlListWalk (set->pre_steps, step_execute, data);
	}
	
	/* execute test steps only if pre-steps passed */
	if (c->passed) {
		xmlListWalk (c->steps, step_execute, data);
		if (xmlListSize (set->post_steps) > 0) {
			log_msg (LOG_INFO, "Executing post steps");
			xmlListWalk (set->post_steps, step_execute, data);
		}
	}
	log_msg (LOG_INFO, "Finished test case Result: %s", c->passed ?
		 "PASS" : "FAIL");
	passcount += c->passed;
	
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Process get data. execute steps in case.
 *  @param data case data
 *  @param user set data
 *  @return 1 always
 */
LOCAL int process_get (const void *data, const void *user) 
{

	xmlChar *fname = (xmlChar *)data;
	td_set *set = (td_set *)user;
	xmlChar *command;
	exec_data edata;

	/*
	** Compose command 
	*/
	memset (&edata, 0x0, sizeof (exec_data));
	edata.soft_timeout = set->gen.timeout ? set->gen.timeout : 90;
	edata.hard_timeout = edata.soft_timeout + 5;

	command = (xmlChar *)malloc (strlen ("cp ") + strlen ((char *)fname) +
				     strlen (opts.output_folder) + 2);
	sprintf ((char *)command, "cp %s %s", (char *)fname, 
		 opts.output_folder);
	/*
	** Execute it
	*/
	execute((char*)command, &edata);

	if (edata.result) {
		fprintf (stderr, "%s: %s failed: %s\n", PROGNAME, command,
			 (char *)(edata.stderr_data.buffer ?
				  edata.stderr_data.buffer : 
				  BAD_CAST "no info available"));
	}
	/*
	** Inspect results
	*/
	if (edata.stdout_data.buffer) free (edata.stdout_data.buffer);
	if (edata.stderr_data.buffer) free (edata.stderr_data.buffer);
	if (edata.failure_info.buffer) free (edata.failure_info.buffer);
	free (command);

	return 1;
}
/* ------------------------------------------------------------------------- */
/** Do processing on suite, currently just writes the pre suite tag to results
 *  @param s suite data
 */
LOCAL void process_suite (td_suite *s)
{
	log_msg (LOG_INFO, "Test suite: %s", s->gen.name);

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
	log_msg (LOG_INFO, "Test set: %s", s->gen.name);

	/*
	** Check that the set is supposed to be executed in the current env
	*/
	if (xmlListSize(s->environments) > 0) {
	        if (!xmlListSearch (s->environments, opts.environment)) {
			goto skip;
		}
	}
	xmlListWalk (s->cases, process_case, s);
	xmlListWalk (s->gets, process_get, s);
	s->environment = xmlCharStrdup (opts.environment);
	
	write_pre_set_tag (s);
	write_post_set_tag (s);
 skip:
	td_set_delete (s);
	return;
}
/* ------------------------------------------------------------------------- */
/** Create output folder based on the argument for -o
 *  @return 0 on success 1 on failure
 */
LOCAL int create_output_folder ()
{
	int len;
	char *p;
	char *pwd, *cmd;
	
	if ((p = strrchr (opts.output_filename, '/'))) {
		len = p - opts.output_filename;
		opts.output_folder = (char *)malloc (len + 2);
		memset (opts.output_folder, 0x00, len + 2);
		strncpy (opts.output_folder, opts.output_filename, len + 1);

	} else {
		pwd = getenv ("PWD");
		if (!pwd) {
			fprintf (stderr, "%s: getenv() failed %s\n",
				 PROGNAME, strerror (errno));
			return 1;
		}
		opts.output_folder = (char *)malloc (strlen (pwd) + 2);
		strcpy (opts.output_folder, pwd);
		opts.output_folder[strlen(pwd)] = '/';
		opts.output_folder[strlen(pwd) + 1] = '\0';
	}
	
	cmd = (char *)malloc (strlen(opts.output_folder) + 
			      strlen("mkdir -p ") + 1);
	sprintf (cmd, "mkdir -p %s", opts.output_folder);

	if  (system (cmd)) {
		fprintf (stderr, "%s failed to create ouput "
			 "directory %s\n",
			 PROGNAME, opts.output_folder);
		free (cmd);
		return 1;
	}
	
	free (cmd);
	
	return 0;
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
	FILE *ifile = NULL;
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
        memset (&hwinfo, 0x0, sizeof(hwinfo));

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
	** FIXME
	*/
	if (v_flag)
	    log_set_verbosity_level (LOG_DEBUG);
	else
	    log_set_verbosity_level (LOG_INFO);
	    
		
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

	if (!opts.output_filename) {
		fprintf (stderr, 
			 "%s: mandatory option missing -o output_file\n",
			 PROGNAME);
		retval = EXIT_FAILURE;
		goto OUT;
	}
	if (create_output_folder(&opts)) {
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
	** Obtain hardware info
	*/
	read_hwinfo (&hwinfo);
	
	/*
	** Initialize result logger
	*/
	retval =  init_result_logger(&opts, &hwinfo);
	if (retval)
		goto OUT;
	
	/*
	** Call td_next_node untill error occurs or the end of data is reached
	*/
	log_msg (LOG_INFO, "Starting to run tests...");

	while (td_next_node() == 0);
	log_msg (LOG_INFO, "Finished running tests.");
	
	td_reader_close();
	close_result_logger();
	log_msg (LOG_INFO, "Executed %d cases. Passed %d Failed %d",
		 casecount, passcount, casecount - passcount);
	log_msg (LOG_INFO, "Results were written to: %s", opts.output_filename);
	log_msg (LOG_INFO, "Finished!");
	
OUT:
	if (opts.input_filename) free (opts.input_filename);
	if (opts.output_filename) free (opts.output_filename);
	if (opts.output_folder) free (opts.output_folder);
	if (opts.environment) free (opts.environment);
	
	return retval;
}	


/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
