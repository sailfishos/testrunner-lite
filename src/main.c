/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Sampo Saaristo <sampo.saaristo@sofica.fi>
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
#include <limits.h>

#include "testrunnerlite.h"
#include "testdefinitionparser.h"
#include "testresultlogger.h"
#include "testdefinitionprocessor.h"
#include "testfilters.h"
#include "executor.h"
#include "remote_executor.h"
#include "manual_executor.h"
#include "utils.h"
#include "hwinfo.h"
#include "log.h"
#ifdef ENABLE_EVENTS
#include "event.h"
#endif

/* ------------------------------------------------------------------------- */
/* EXTERNAL DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* EXTERNAL GLOBAL VARIABLES */
extern char* optarg;
extern int bail_out;
/* ------------------------------------------------------------------------- */
/* EXTERNAL FUNCTION PROTOTYPES */
/* None */

/* ------------------------------------------------------------------------- */
/* GLOBAL VARIABLES */
struct timeval created;
testrunner_lite_options opts;
/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL GLOBAL VARIABLES */
LOCAL hw_info hwinfo;
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
LOCAL void version();
/* ------------------------------------------------------------------------- */
LOCAL void copyright();
/* ------------------------------------------------------------------------- */
LOCAL int create_output_folder ();
/* ------------------------------------------------------------------------- */
LOCAL int parse_remote_logger(char *url, testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
LOCAL int parse_target_address(char *address, testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
LOCAL int parse_chroot_folder(char *folder, testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
LOCAL int test_chroot(char * folder);
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
	printf ("  -V, --version\tDisplay version and exit.\n");
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
	
	printf ("  -l FILTER, --filter=FILTER\n\t\t"
		"Filtering option to select tests (not) to be executed.\n\t\t"
		"E.g. '-testcase=bad_test -type=unknown' first disables\n\t\t"
		"test case named as bad_test. Next, all tests with type\n\t\t"
		"unknown are disabled. The remaining tests will be\n\t\t"
		"executed. (Currently supported filter type are: \n\t\t"
		"testset,testcase,requirement,feature and type)\n");
	printf ("  -c, --ci\tDisable validation of test "
		"definition against schema.\n");
	printf ("  -s, --semantic\n\t\tEnable validation of test "
		"definition against stricter (semantics) schema.\n");
	printf ("  -A, --validate-only\n\t\tDo only input xml validation, "
		"do not execute tests.\n");
	printf ("  -H, --no-hwinfo\n\t\tSkip hwinfo obtaining.\n");
	printf ("  -P, --print-step-output\n\t\tOutput standard streams from"
		" programs started in steps\n");
	printf ("  -S, --syslog\n\t\tWrite log messages also to syslog.\n");
	printf ("  -t [USER@]ADDRESS, --target=[USER@]ADDRESS\n\t\t"
		"Enable host-based testing. "
		"If given, commands are executed from\n\t\t"
		"test control PC (host) side. "
		"ADDRESS is the ipv4 adress of the\n\t\t"
		"system under test.\n");
	printf ("  -M, --disable-measurement-verdict\n\t\t"
		" Do not fail cases based on measurement data\n");
	printf ("  -C PATH, --chroot=PATH\n\t\t"
		"Run tests inside a chroot environment. Note that this\n\t\t"
		"doesn't change the root of the testrunner itself,\n\t\t"
		"only the tests will have the new root folder set.\n");
	return;
}
/** Print version
 */
LOCAL void version()
{
#ifdef VERSIONSTR
#define AS_STRING_(x) #x
#define AS_STRING(x) AS_STRING_(x)
	printf ("testrunner-lite version %s\n", AS_STRING(VERSIONSTR));
#else
	printf ("no version information available\n");
#endif
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
/** Create output folder based on the argument for -o
 *  @return 0 on success 1 on failure
 */
LOCAL int create_output_folder ()
{
	int len;
	char *p;
	char pwd[PATH_MAX], *cmd;
	
	if ((p = strrchr (opts.output_filename, '/'))) {
		len = p - opts.output_filename;
		opts.output_folder = (char *)malloc (len + 2);
		memset (opts.output_folder, 0x00, len + 2);
		strncpy (opts.output_folder, opts.output_filename, len + 1);

	} else {
		if (!getcwd (pwd, PATH_MAX)) {
			LOG_MSG (LOG_ERR, "%s: getcwd() failed %s\n",
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
		LOG_MSG (LOG_ERR, "%s failed to create output "
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
/** Parse chroot option argument.
 * @param folder path to change root envrionment
 * @param opts Options struct containing field(s) to store path
 * @return 0 in success, 1 on generic error, 2 if folder doesn't exist,
 *    3 if folder isn't a directory, 4 if there is a problem with the chroot
 */
LOCAL int parse_chroot_folder(char *folder, testrunner_lite_options *opts) {
	struct stat stat_buf;

	if (folder) {
		opts->chroot_folder = malloc(strlen(folder) + 1);
		strcpy(opts->chroot_folder, folder);

		// check that folder exists, is a directory and we can chroot into it
		if (stat(folder, &stat_buf) == -1) {
			fprintf(stderr, "%s: could not stat folder '%s'\n",
				PROGNAME, folder);
			return 2;
		}
		if (!S_ISDIR(stat_buf.st_mode)) {
			fprintf(stderr, "%s: '%s' is not a directory\n",
				PROGNAME, folder);
			return 3;
		}
		if (test_chroot(folder) != 0) {
			return 4;
		}

		return 0;
	} else {
		return 1;
	}
}

/* ------------------------------------------------------------------------- */
/** Test access to chroot.
 * @param folder path to change root envrionment
 * @return 0 in success, 1 on generic error, 2 if there is a problem with
 *    the chroot
 */
LOCAL int test_chroot(char * folder) {
	pid_t pid;
	int status;

	// make sure we can chroot to the folder
	pid = fork();
	if (pid > 0) {
		waitpid(pid, &status, 0);
		switch (WEXITSTATUS(status)) {
			case 1:
				fprintf(stderr, "%s: folder '%s' is inaccessible\n",
					PROGNAME, folder);
				return 2;
			case 2:
				fprintf(stderr, "%s: unable to chroot to folder '%s'\n",
					PROGNAME, folder);
				return 2;
			case 3:
				fprintf(stderr, "%s: failed to execute '/bin/sh' inside '%s'\n",
					PROGNAME, folder);
				return 2;
			default:
				break;
		}
	} else if (pid == 0) {
		if (chdir(folder) == -1) {
			exit(1);
		}
		if (chroot(".") == -1) {
			exit(2);
		}
		if (WEXITSTATUS(system("exit")) != 0) {
			exit(3);
		}
		exit(0);
	} else {
		fprintf(stderr, "Fork failed: %s\n", strerror(errno));
		return 1;
	}

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
	int h_flag = 0, a_flag = 0, m_flag = 0, A_flag = 0, V_flag = 0;
	int opt_char, option_idx;
	FILE *ifile = NULL;
	testrunner_lite_return_code retval = TESTRUNNER_LITE_OK;
	xmlChar *filter_string = NULL;
	struct option testrunnerlite_options[] =
		{
			{"help", no_argument, &h_flag, 1},
			{"version", no_argument, &V_flag, 1},
			{"file", required_argument, NULL, 'f'},
			{"output", required_argument, NULL, 'o'},
			{"format", required_argument, NULL, 'r'},
			{"environment", required_argument, NULL, 'e'},
			{"verbose", optional_argument, NULL, 'v'},
			{"syslog", no_argument, &opts.syslog_output, 1},
			{"automatic", no_argument, &a_flag, 1},
			{"manual", no_argument, &m_flag, 1},
			{"filter", required_argument, NULL, 'l'},
			{"logger", required_argument, NULL, 'L'},
			{"ci", no_argument, &opts.disable_schema},
			{"semantic", no_argument, &opts.semantic_schema},
			{"validate-only", no_argument, &A_flag},
			{"no-hwinfo", no_argument, &opts.skip_hwinfo, 1},
			{"target", required_argument, NULL, 't'},
			{"chroot", required_argument, NULL, 'C'},
			{"print-step-output", no_argument, 
			 &opts.print_step_output, 1},
			{"disable-measurement-verdict", no_argument, 
			 &opts.no_measurement_verdicts, 1},

			{0, 0, 0, 0}
		};


	LIBXML_TEST_VERSION

	memset (&opts, 0x0, sizeof(testrunner_lite_options));
        memset (&hwinfo, 0x0, sizeof(hwinfo));

	opts.output_type = OUTPUT_TYPE_XML;
	opts.run_automatic = opts.run_manual = 1;
	gettimeofday (&created, NULL);
	signal (SIGINT, handle_sigint);
	signal (SIGTERM, handle_sigterm);
	copyright();
	if (argc == 1)
		h_flag = 1;

	while (1) {
		option_idx = 0;
     
		opt_char = getopt_long (argc, argv, 
					":hVaAHSMsmcPC:f:o:e:l:r:L:t:v::",
					testrunnerlite_options, &option_idx);
		if (opt_char == -1)
			break;
		
		switch (opt_char)
		{
		case 'h':
			h_flag = 1;
			break;
		case 'V':
			V_flag = 1;
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
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
				goto OUT;
			}
			break;
		case 'f':
			ifile = fopen (optarg, "r");
			if (!ifile) {
				fprintf (stderr, "%s Failed to open %s %s\n",
					 PROGNAME, optarg, strerror (errno));
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
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
		case 'H':
			opts.skip_hwinfo = 1;
			break;
		case 'S':
			opts.syslog_output = 1;
			break;
		case 'L':
			if (parse_remote_logger(optarg, &opts) != 0) {
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
				goto OUT;
			}
			break;
		case 'l':
		        if (filter_string) {
				filter_string = xmlStrcat (filter_string,
							   BAD_CAST " ");
				filter_string = xmlStrcat (filter_string,
							   BAD_CAST optarg);
			} else
				filter_string = xmlCharStrdup (optarg);
			break;
		case 't':
			if (parse_target_address(optarg, &opts) != 0) {
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
				goto OUT;
			}
			break;
		case 'C':
			if (parse_chroot_folder(optarg, &opts) != 0) {
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
				goto OUT;
			}
		case 'P':
			opts.print_step_output = 1;
			break;
		case 'M':
			opts.no_measurement_verdicts = 1;
			break;
		case ':':
			fprintf (stderr, "%s missing argument - exiting\n",
				 PROGNAME);
			retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
			goto OUT;
			break;
		case '?':
			fprintf (stderr, "%s unknown option - exiting\n",
				 PROGNAME);
			retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
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
	if (V_flag) {
		version();
		goto OUT;
	}

	if (opts.chroot_folder && opts.target_address) {
		fprintf (stderr,
			"%s: -C and -t are mutually exclusive\n",
			PROGNAME);
		retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
		goto OUT;
	}
	
	if (m_flag && a_flag) {
		fprintf (stderr, 
			 "%s: -a and -m are mutually exclusive\n",
			 PROGNAME);
		retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
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
		retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
		goto OUT;
	}
	/*
	 * Initialize logging.
	 */
	log_init (&opts);
	/*
	** Log version if we have it
	*/
#ifdef VERSIONSTR
#define AS_STRING_(x) #x
#define AS_STRING(x) AS_STRING_(x)
	LOG_MSG (LOG_INFO, "Version %s", AS_STRING(VERSIONSTR));
#endif
	/*
	 * Initialize filters if specified.
	 */
	if (filter_string) {
	        init_filters();
		if (parse_filter_string ((char *)filter_string) != 0) {
		        LOG_MSG (LOG_ERR, "filter parsing failed .. exiting");
			retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
			goto OUT;
		}
	}
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
		retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
		goto OUT;
	}
	if (create_output_folder(&opts)) {
		retval = TESTRUNNER_LITE_OUTPUT_FOLDER_CREATE_FAIL;
		goto OUT;
	}

	if (!opts.environment) {
		opts.environment = (char *)malloc (strlen ("hardware") + 1);
		strcpy (opts.environment, "hardware");
	}

        /*
	** Initialize the reader
	*/
	retval = td_reader_init(&opts);
	if (retval) {
		retval = TESTRUNNER_LITE_XML_READER_FAIL;
		goto OUT;
	}
	/*
	** Obtain hardware info
	*/
	if (!opts.skip_hwinfo)
		read_hwinfo (&hwinfo);
	
	/*
	** Initialize result logger
	*/
	retval =  init_result_logger(&opts, &hwinfo);
	if (retval) {
		retval = TESTRUNNER_LITE_RESULT_LOGGING_FAIL;
		goto OUT;
	}
#ifdef ENABLE_EVENTS
	init_event_system();
#endif
	/*
	** Process test definition
	*/
	td_process();

	executor_close();
#ifdef ENABLE_EVENTS
	cleanup_event_system();
#endif
	td_reader_close();
	close_result_logger();
	LOG_MSG (LOG_INFO, "Results were written to: %s", opts.output_filename);
	LOG_MSG (LOG_INFO, "Finished!");
	cleanup_filters();
	log_close();
 OUT:
	if (opts.input_filename) free (opts.input_filename);
	if (opts.output_filename) free (opts.output_filename);
	if (opts.output_folder) free (opts.output_folder);
	if (opts.environment) free (opts.environment);
	if (opts.remote_logger) free (opts.remote_logger);
	if (opts.target_address) free (opts.target_address);
	if (filter_string) free (filter_string);
	if (bail_out == 255+SIGINT) {
		signal (SIGINT, SIG_DFL);
		raise (SIGINT);
	} else if (bail_out == 255+SIGTERM) {
		signal (SIGTERM, SIG_DFL);
		raise (SIGTERM);
	} else if (bail_out) retval = TESTRUNNER_LITE_SSH_FAIL;

	return retval; 
}	


/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
