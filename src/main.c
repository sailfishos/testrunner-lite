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
LOCAL void print_suite(td_suite *);
/* ------------------------------------------------------------------------- */
LOCAL void print_set(td_set *);
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
	return;
}
/* ------------------------------------------------------------------------- */
LOCAL int step_print (const void *data, const void *user) {

	td_step *step = (td_step *)data;
	if (step->step) printf ("\t%s\n", step->step);

	return 1;
}
/* ------------------------------------------------------------------------- */
LOCAL void output_processor(int stdout_fd, int stderr_fd) {
	char out[1024];
	char err[1024];
	ssize_t bytes = 0;
	char* p = out;

	while ((bytes = read(stdout_fd, p, 128))) {
		if(bytes < 0) {
			break;
		}
		p += bytes;
	}
	*p = '\0';

	p = err;
	while ((bytes = read(stderr_fd, p, 128))) {
		if(bytes < 0) {
			break;
		}
		p += bytes;
	}
	*p = '\0';

	printf("stdout:\n");
	printf("%s", out);
	printf("stderr:\n");
	printf("%s", err);

}
/* ------------------------------------------------------------------------- */
LOCAL int step_execute (const void *data, const void *user) {

	td_step *step = (td_step *)data;
	if (step->step) {
		printf ("\t%s\n", step->step);
		execute(step->step, output_processor);
	}
	return 1;
}
/* ------------------------------------------------------------------------- */
LOCAL int case_print (const void *data, const void *user) {

	td_case *c = (td_case *)data;
	printf ("\tCASE: %s\n", c->name);
	if (c->timeout) printf ("\ttimeout: %lu\n", c->timeout);
	printf ("\tsteps:\n");
	xmlListWalk (c->steps, step_execute, NULL);

	return 1;
}
/* ------------------------------------------------------------------------- */
/** Just print out the suite given as parameter
 *  @param td_suite suite data
 */
LOCAL void print_suite (td_suite *s)
{
	printf ("SUITE = name:%s\n", s->name); 
	write_pre_suite_tag (s);
	current_suite = s;
	
}
/* ------------------------------------------------------------------------- */
LOCAL void end_suite ()
{
	write_post_suite_tag ();
	td_suite_delete (current_suite);
}

/* ------------------------------------------------------------------------- */
LOCAL void print_set (td_set *s)
{
	printf ("SET: %s\n", s->name); 
	printf ("\tPre-steps:\n"); 
	xmlListWalk (s->pre_steps, step_print, NULL);
	printf ("\tPost-steps:\n"); 
	xmlListWalk (s->post_steps, step_print, NULL);
	printf ("\tPost-steps:\n"); 
	xmlListWalk (s->cases, case_print, NULL);
	write_pre_set_tag (s);
	write_post_set_tag ();

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
	int h_flag = 0, v_flag = 0, a_flag = 0, m_flag = 0;
	int opt_char, option_idx;
	FILE *ifile = NULL, *ofile = NULL;
	int retval = EXIT_SUCCESS;
	testrunner_lite_options opts;
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
			{"ci", no_argument, &opts.disable_schema}
		};


	LIBXML_TEST_VERSION

	memset (&opts, 0x0, sizeof(testrunner_lite_options));
        memset (&cbs, 0x0, sizeof(td_parser_callbacks));
	opts.output_type = OUTPUT_TYPE_XML;
	
	while (1) {
		option_idx = 0;
     
		opt_char = getopt_long (argc, argv, "hvamcf:o:e:l:r:",
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
		}
	}
	
	if (h_flag) {
		usage();
		goto OUT;
	}

	if (!ifile) {
		fprintf (stderr, 
			 "%s: mandatory option missing -f input_file\n",
			 PROGNAME);
		retval = EXIT_FAILURE;
		goto OUT;
	}
		
	if (!ofile) {
		fprintf (stderr, 
			 "%s: mandatory option missing -o output_file\n",
			 PROGNAME);
		retval = EXIT_FAILURE;
		goto OUT;
	}

	retval = parse_test_definition (&opts);
	if (retval)
		goto OUT;

	/*
	** Set callbacks for parser
	*/
	cbs.test_suite = print_suite;
	cbs.test_suite_end = end_suite;
	cbs.test_set = print_set;

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
	
	return retval;
}	


/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
