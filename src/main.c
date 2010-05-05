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
#include <signal.h>
#include <ctype.h>

#include "testrunnerlite.h"
#include "testdefinitionparser.h"
#include "testresultlogger.h"
#include "executor.h"
#include "manual_executor.h"
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
struct timeval created;
testrunner_lite_options opts;
char *global_failure = NULL;
int bail_out = 0;
/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL GLOBAL VARIABLES */
LOCAL td_suite *current_suite = NULL;
LOCAL hw_info hwinfo;
LOCAL int passcount = 0;
LOCAL int casecount = 0;
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
LOCAL void copyright();
/* ------------------------------------------------------------------------- */
LOCAL void process_suite(td_suite *);
/* ------------------------------------------------------------------------- */
LOCAL void process_set(td_set *);
/* ------------------------------------------------------------------------- */
LOCAL int process_case (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int case_result_na (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int process_get (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int step_execute (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int step_result_na (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int step_post_process (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int create_output_folder ();
/* ------------------------------------------------------------------------- */
LOCAL unsigned int trim_filename(char *, char *);
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
	printf ("  -v, -vv, --verbose[={INFO|DEBUG}]\n\t\t"
		"Enable verbosity mode; -v and --verbose=INFO "
		"are equivalent\n\t\t"
		"outputting INFO, ERROR and WARNING messages.\n\t\t"
		"Similarly -vv and --verbose=DEBUG "
		"are equivalent, outputting\n\t\t"
		"also debug messages. Default behaviour is silent mode.\n");
	printf("  -L, --logger=URL\n\t\t"
	       "Remote HTTP logger for log messages. URL format is\n\t\t"
	       "[http://]host[:port][/path/], "
	       "where host may be a hostname\n\t\t"
	       "or an IPv4 address.\n");
	printf ("  -a, --automatic\tEnable only automatic tests "
		"to be executed.\n");
	printf ("  -m, --manual\tEnable only manual tests to be executed.\n");
	
#if 0 /* do not advertise features we do not have yet .. */
	printf ("  -l FILTER, --filter=FILTER\n\t\t"
		"Filtering option to select tests (not) to be executed.\n\t\t"
		"E.g. '-testcase=bad_test -type=unknown' first disables\n\t\t"
		"test case named as bad_test. Next, all tests with type\n\t\t"
		"unknown are disabled. The remaining tests will be\n\t\t"
		"executed.\n");
#endif 
	printf ("  -c, --ci\tDisable validation of test "
		"definition against schema.\n");
	printf ("  -s, --semantic\n\t\tEnable validation of test "
		"definition against stricter (semantics) schema.\n");
	printf ("  -A, --validate-only\n\t\tDo only input xml validation, "
		"do not execute tests.\n");
	printf ("  -t [USER@]ADDRESS, --target=[USER@]ADDRESS\n\t\t"
		"Enable host-based testing. "
		"If given, commands are executed from\n\t\t"
		"test control PC (host) side. "
		"ADDRESS is the ipv4 adress of the\n\t\t"
		"system under test.\n");
    
	return;
}
/* ------------------------------------------------------------------------- */
/** Display license information.
 */
LOCAL void copyright () {
        printf ("testrunner-lite, © Nokia 2010 All rights reserved,\n"
                "licensed under the Gnu Lesser General Public License "
		"version 2.1,\n"
                "Contact: Ville Ilvonen <ville.p.ilvonen@nokia.com>\n");
}

/* ------------------------------------------------------------------------- */
/** Process step data. execute one step from case.
 *  @param data step data
 *  @param user case data
 *  @return 1 if step is passed 0 if not
 */
LOCAL int step_execute (const void *data, const void *user) 
{
	int res = CASE_PASS;
	td_step *step = (td_step *)data;
	td_case *c = (td_case *)user;
	exec_data edata;

	memset (&edata, 0x0, sizeof (exec_data));

	if (bail_out) {
		res = CASE_NA;
		step->return_code = bail_out;
		if (global_failure)
			step->failure_info = xmlCharStrdup (global_failure);
		goto out;
	}

	if (c->gen.manual) {
		res = execute_manual (step);
		goto out;
	}

	init_exec_data(&edata);

	if (c->dummy) {
		/* Pre or post step */
		edata.redirect_output = DONT_REDIRECT_OUTPUT;
	} else {
		edata.redirect_output = REDIRECT_OUTPUT;
	}
	edata.soft_timeout = c->gen.timeout;
	edata.hard_timeout = COMMON_HARD_TIMEOUT;

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
			LOG_MSG (LOG_INFO, "FAILURE INFO %s",
				 step->failure_info);
		}
		
		step->pgid = edata.pgid; 
		step->has_result = 1;
		step->return_code = edata.result;
		step->start = edata.start_time;
		step->end = edata.end_time;
		/*
		** Post and pre steps fail only if the expected result is 
		*  specified
		*/
		if (c->dummy) {
			if (step->has_expected_result &&
			    (step->return_code != step->expected_result)) {
				LOG_MSG (LOG_INFO, 
					 "STEP: %s return %d expected %d\n",
					 step->step, step->return_code, 
					 step->expected_result);
				res = CASE_FAIL;
			}
		} else if (step->return_code != step->expected_result) {
			LOG_MSG (LOG_INFO, "STEP: %s return %d expected %d\n",
				 step->step, step->return_code, 
				 step->expected_result);
			res = CASE_FAIL;
		}
	}
 out:
	if (res != CASE_PASS)
		c->case_res = res;
	

	return !res;
}

/* ------------------------------------------------------------------------- */
/** Set N/A result for test step
 *  @param data step data
 *  @param user failure info string
 *  @return 1 always
 */
LOCAL int step_result_na (const void *data, const void *user) 
{
	td_step *step = (td_step *)data;
	char *failure_info = (char *)user;
	
	step->has_result = 0; /* causes the result to be interpreted as N/A */
	step->failure_info = xmlCharStrdup (failure_info);

	return 1;
}

/* ------------------------------------------------------------------------- */
/** Do step post processing. Mainly to ascertain that no dangling processes are
 *  left behind.
 *  @param data step data
 *  @param user case data
 *  @return 1 always
 */
LOCAL int step_post_process (const void *data, const void *user) 
{
	td_step *step = (td_step *)data;
	td_case *c = (td_case *)user;

	/* No post processing for manual steps ... */
	if (c->gen.manual) 
		goto out;
	/* ... or filtered ones ... */
	if (c->filtered)
		goto out;

	/* ... or ones that are not run ... */
	if (!step->start)
		goto out;

	/* ... or ones that do not have process group ... */
	if (!step->start)
		goto out;

	kill_pgroup(step->pgid, SIGKILL);
 out:
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
	
	if (c->gen.manual && !opts.run_manual) {
		LOG_MSG(LOG_DEBUG, "Skipping manual case %s",
			c->gen.name);
		c->filtered = 1;
		return 1;
	}
	if (!c->gen.manual && !opts.run_automatic) {
		LOG_MSG(LOG_DEBUG, "Skipping automatic case %s",
			c->gen.name);
		c->filtered = 1;
		return 1;
	}

	LOG_MSG (LOG_INFO, "Starting test case %s", c->gen.name);
	casecount++;

	c->case_res = CASE_PASS;
	if (c->gen.timeout == 0)
		c->gen.timeout = COMMON_SOFT_TIMEOUT; /* the default one */
	
	if (c->gen.manual && opts.run_manual)
		pre_manual (c);

	xmlListWalk (c->steps, step_execute, data);
	xmlListWalk (c->steps, step_post_process, data);

	if (c->gen.manual && opts.run_manual)
		post_manual (c);

	LOG_MSG (LOG_INFO, "Finished test case Result: %s", 
		 case_result_str(c->case_res));
	passcount += (c->case_res == CASE_PASS);
	
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Set case result to N/A
 *  @param data case data
 *  @param user failure info
 *  @return 1 always
 */
LOCAL int case_result_na (const void *data, const void *user)
{

	td_case *c = (td_case *)data;
	
	LOG_MSG (LOG_DEBUG, "Setting N/A result for case %s", c->gen.name);

	c->case_res = CASE_NA;

	xmlListWalk (c->steps, step_result_na, user);
	
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

	xmlChar *rawfname = (xmlChar *)data;
	xmlChar *command;
	char *fname;
	exec_data edata;
	char    *remote = opts.target_address;

	memset (&edata, 0x0, sizeof (exec_data));
	init_exec_data(&edata);
	edata.soft_timeout = COMMON_SOFT_TIMEOUT;
	edata.hard_timeout = COMMON_HARD_TIMEOUT;

	fname = malloc (strlen((char *)rawfname) + 1);
	trim_filename((char *)rawfname, fname);

	/*
	** Compose command 
	*/
	if (remote) {
		opts.target_address = NULL; /* execute locally */
		command = (xmlChar *)malloc (strlen ("scp ") + 
					     strlen (fname) +
					     strlen (opts.output_folder) +
					     strlen (remote) + 4);
		sprintf ((char *)command, "scp %s:%s %s", remote, fname, 
			 opts.output_folder);

	} else {
		command = (xmlChar *)malloc (strlen ("cp ") + 
					     strlen (fname) +
					     strlen (opts.output_folder) + 2);
		sprintf ((char *)command, "cp %s %s", fname,
			 opts.output_folder);
	}
	LOG_MSG (LOG_DEBUG, "%s:  Executing command: %s", PROGNAME, 
		 (char*)command);
	/*
	** Execute it
	*/
	execute((char*)command, &edata);

	if (edata.result) {
		LOG_MSG (LOG_ERROR, "%s: %s failed: %s\n", PROGNAME, command,
			 (char *)(edata.stderr_data.buffer ?
				  edata.stderr_data.buffer : 
				  BAD_CAST "no info available"));
	}
	opts.target_address = remote;
	if (edata.stdout_data.buffer) free (edata.stdout_data.buffer);
	if (edata.stderr_data.buffer) free (edata.stderr_data.buffer);
	if (edata.failure_info.buffer) free (edata.failure_info.buffer);
	free (command);
	free (fname);

	return 1;
}
/* ------------------------------------------------------------------------- */
/** Do processing on suite, currently just writes the pre suite tag to results
 *  @param s suite data
 */
LOCAL void process_suite (td_suite *s)
{
	LOG_MSG (LOG_INFO, "Test suite: %s", s->gen.name);

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
	td_case dummy;

	LOG_MSG (LOG_INFO, "Test set: %s", s->gen.name);

	s->environment = xmlCharStrdup (opts.environment);
	write_pre_set_tag (s);
	/*
	** Check that the set is supposed to be executed in the current env
	*/
	if (xmlListSize(s->environments) > 0) {
	        if (!xmlListSearch (s->environments, opts.environment)) {
			goto skip;
		}
	}

	if (xmlListSize (s->pre_steps) > 0) {
		memset (&dummy, 0x0, sizeof (td_case));
		dummy.case_res = CASE_PASS;
		dummy.dummy = 1;
		dummy.gen.timeout = 0; /* No timeout for pre steps */
		LOG_MSG (LOG_INFO, "Executing pre steps");
		xmlListWalk (s->pre_steps, step_execute, &dummy);
		if (dummy.case_res != CASE_PASS) {
			LOG_MSG (LOG_ERROR, "Pre steps failed. "
				 "Test set %s aborted.", s->gen.name); 
			xmlListWalk (s->cases, case_result_na, 
				     global_failure ? global_failure :
				     "pre_steps failed");
			goto skip;
		}
	}
	
	xmlListWalk (s->cases, process_case, s);
	xmlListWalk (s->gets, process_get, s);
	if (xmlListSize (s->post_steps) > 0) {
		LOG_MSG (LOG_INFO, "Executing post steps");
		memset (&dummy, 0x0, sizeof (td_case));
		dummy.case_res = CASE_PASS;
		dummy.dummy = 1;
		/* Default timeout for post steps */
		dummy.gen.timeout = COMMON_SOFT_TIMEOUT;
		xmlListWalk (s->post_steps, step_execute, &dummy);
		if (dummy.case_res == CASE_FAIL)
			LOG_MSG (LOG_ERROR, 
				 "Post steps failed for %s.", s->gen.name);
	}

 skip:
	
	write_post_set_tag (s);
	if (xmlListSize (s->pre_steps) > 0)
		xmlListWalk (s->pre_steps, step_post_process, &dummy);
	if (xmlListSize (s->post_steps) > 0)
		xmlListWalk (s->post_steps, step_post_process, &dummy);
	xml_end_element();
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
			LOG_MSG (LOG_ERROR, "%s: getenv() failed %s\n",
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
		LOG_MSG (LOG_ERROR, "%s failed to create output "
			 "directory %s\n",
			 PROGNAME, opts.output_folder);
		free (cmd);
		return 1;
	}
	
	free (cmd);
	
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Parse remote logger option argument. Currently nothing to parse.
 * @param url Remote logger URL option argument
 * @param opts Options struct containing field(s) to store url
 * @return 0 in success, 1 on failure
 */
LOCAL int parse_remote_logger(char *url, testrunner_lite_options *opts) {
	if (url) {
		opts->remote_logger = malloc(strlen(url) + 1);
		strcpy(opts->remote_logger, url);
		return 0;
	} else {
		return 1;
	}

}

/* ------------------------------------------------------------------------- */
/** Parse target address option argument.
 * @param address SUT address.
 * @param opts Options struct containing field(s) to store url
 * @return 0 in success, 1 on failure
 */
LOCAL int parse_target_address(char *address, testrunner_lite_options *opts) {
    if (address) {
        opts->target_address = malloc(strlen(address) + 1);
        strcpy(opts->target_address, address);
        return 0;
    } else {
        return 1;
    }

}

/* ------------------------------------------------------------------------- */
/** Trim string of whitespace and control characters.
 * Remove unwanted whitespace, linefeeds etc. (using isspace()) from the
 * beginning and end of the string (until the first/last non-whitespace
 * character) and control characters (using iscntrl()) from the middle.
 * @param ins The input string. Must not be null.
 * @param outs The output string. Must be at least as long as the input string
 and not null.
 * @return Length of the output string
 */
LOCAL unsigned int trim_filename(char *ins, char *outs)
{
	unsigned int ins_i = 0;
	unsigned int ins_end = 0;
	unsigned int outs_i = 0;

	/* make sure input and output strings exist */
	if (ins == 0 || outs == 0) {
		return 0;
	}

	ins_end = strlen(ins);

	/* test if the string is empty */
	if (ins_end == 0) {
		return 0;
	}

	/* find the first non-whitespace character */
	while (1) {
		if (ins_i >= ins_end)
			break;
		if (isspace(ins[ins_i]))
			ins_i += 1;
		else
			break;
	}

	/* find the last non-whitespace character */
	while (1) {
		if (ins_end <= ins_i)
			break;
		if (isspace(ins[ins_end - 1]))
			ins_end -= 1;
		else
			break;
	}

	/* Copy trimmed string to output */
	while (ins_i < ins_end) {
		/* check and skip control characters */
		if (!iscntrl(ins[ins_i])) {
			outs[outs_i] = ins[ins_i];
			outs_i += 1;
		}
		ins_i += 1;
	}
	/* add null termination */
	outs[outs_i] = 0;

	return outs_i;
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
	int h_flag = 0, a_flag = 0, m_flag = 0, A_flag = 0;
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
			{"verbose", optional_argument, NULL, 'v'},
			{"automatic", no_argument, &a_flag, 1},
			{"manual", no_argument, &m_flag, 1},
			{"filter", required_argument, NULL, 'l'},
			{"logger", required_argument, NULL, 'L'},
			{"ci", no_argument, &opts.disable_schema},
			{"semantic", no_argument, &opts.semantic_schema},
			{"validate-only", no_argument, &A_flag},
			{"target", required_argument, NULL, 't'},
			{0, 0, 0, 0}
		};


	LIBXML_TEST_VERSION

	memset (&opts, 0x0, sizeof(testrunner_lite_options));
        memset (&cbs, 0x0, sizeof(td_parser_callbacks));
        memset (&hwinfo, 0x0, sizeof(hwinfo));
	
	opts.output_type = OUTPUT_TYPE_XML;
	opts.run_automatic = opts.run_manual = 1;
	gettimeofday (&created, NULL);

	copyright();
	if (argc == 1)
		h_flag = 1;

	while (1) {
		option_idx = 0;
     
		opt_char = getopt_long (argc, argv, ":haAsmcf:o:e:l:r:L:t:v::",
					testrunnerlite_options, &option_idx);
		if (opt_char == -1)
			break;
		
		switch (opt_char)
		{
		case 'h':
			h_flag = 1;
			break;
		case 'v':
			if (opts.log_level != 0)
				break;
            
			if (optarg) {
				if (!strcmp (optarg, "INFO"))
					opts.log_level = LOG_LEVEL_INFO;
				if (!strcmp (optarg, "DEBUG") 
				    || !strcmp (optarg, "v"))
					opts.log_level = LOG_LEVEL_DEBUG;
			}
			else {
				opts.log_level = LOG_LEVEL_INFO;
			}
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
		case 'L':
			if (parse_remote_logger(optarg, &opts) != 0) {
				retval = EXIT_FAILURE;
				goto OUT;
			}
			break;
		case 't':
			if (parse_target_address(optarg, &opts) != 0) {
				retval = EXIT_FAILURE;
				goto OUT;
			}
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
	 * Set logging level.
	 */
	log_init (&opts);
	/*
	 * Set remote execution options.
	 */
	executor_init (&opts);
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
	LOG_MSG (LOG_INFO, "Starting to run tests...");

	while (td_next_node() == 0);
	LOG_MSG (LOG_INFO, "Finished running tests.");
	executor_close();
	td_reader_close();
	close_result_logger();
	LOG_MSG (LOG_INFO, "Executed %d cases. Passed %d Failed %d",
		 casecount, passcount, casecount - passcount);
	LOG_MSG (LOG_INFO, "Results were written to: %s", opts.output_filename);
	LOG_MSG (LOG_INFO, "Finished!");
	log_close();
OUT:
	if (opts.input_filename) free (opts.input_filename);
	if (opts.output_filename) free (opts.output_filename);
	if (opts.output_folder) free (opts.output_folder);
	if (opts.environment) free (opts.environment);
	if (opts.remote_logger) free (opts.remote_logger);
	if (opts.target_address) free (opts.target_address);
	if (bail_out) retval = bail_out;

	return retval; 
}	


/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
