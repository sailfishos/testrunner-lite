/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Contains changes by Wind River Systems, 2011-03-09
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
#include <sys/wait.h>
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
#define SSH_REMOTE_EXECUTOR "/usr/bin/ssh -o StrictHostKeyChecking=no " \
		"-o PasswordAuthentication=no -o ServerAliveInterval=5 " \
		"-o ServerAliveCountMax=1 -o ConnectTimeout=7 %s %s %s"
#define SCP_REMOTE_GETTER "/usr/bin/scp %s %s %s:'<FILE>' '<DEST>'"
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
LOCAL int create_output_folder ();
/* ------------------------------------------------------------------------- */
LOCAL int parse_remote_logger(char *url, testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
LOCAL int parse_target_address(char *address, testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
LOCAL int parse_remote_getter(char *getter, testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
LOCAL int parse_default_ssh_executor(testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
LOCAL int parse_default_scp_getter(testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
LOCAL int parse_chroot_folder(char *folder, testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
LOCAL int test_chroot(char * folder);
/* ------------------------------------------------------------------------- */
LOCAL int set_rich_core_dumps(char *folder, testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
LOCAL int parse_logid(char *logid, testrunner_lite_options *opts);
/* ------------------------------------------------------------------------- */
LOCAL int parse_target_address_hwinfo(char* address, 
				      testrunner_lite_options *opts);
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
	printf ("Example: testrunner-lite -f tests.xml -o ~/results.xml -v\n");
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
	printf ("  -M, --disable-measurement-verdict\n\t\t"
		" Do not fail cases based on measurement data\n");
	printf ("  --measure-power\n\t\t"
		"Perform current measurement with hat_ctrl tool during execution\n\t\t"
		"of test cases\n");
	printf ("  -u URL, --vcs-url=URL\n\t\t"
		"Causes testrunner-lite to write the given VCS URL to "
		"results.\n"
		);
	printf ("  -U URL, --package-url=URL\n\t\t"
		"Causes testrunner-lite to write the given package URL to "
		"results.\n"
		);
	printf ("  --logid=ID\n\t\t"
		"User defined identifier for HTTP log messages.\n");
        printf ("  -d PATH, --rich-core-dumps=PATH\n\t\t"
                "Save rich-core dumps. PATH is the location, where rich-core dumps\n\t\t"
                "are produced in the device. Creates UUID mappings between executed\n\t\t"
                "test cases and generated rich-core dumps. This makes possible\n\t\t"
                "to link each rich-cores and test cases in test reporting\n\t\t"
                "NOTE: This feature requires working sp-rich-core package to be\n\t\t"
                "installed in the Device Under Test.\n");
	printf ("  -T SECONDS, --core-upload-timeout=SECONDS\n\t\t"
		"How long testrunner-lite will wait for crash reports generated during\n\t\t"
		"a test case to upload to telemetry server. Default is 0 (no waiting),\n\t\t"
		"-1 means wait never times out.\n");
	printf ("  --utf8-limit=MAXLENGTH\n\t\t"
	        "Maximum allowed length of a UTF-8 byte sequence in output of a test step.\n\t\t"
		"If the limit is exceeded, the whole output will be written into a separate\n\t\t"
		"file as in case of any invalid UTF-8 output. Default value is 4.\n");
	printf ("\nTest commands are executed locally by default.  Alternatively, one\n"
		"of the following executors can be used:\n");
	printf ("\nChroot Execution:\n");
	printf ("  -C PATH, --chroot=PATH\n\t\t"
		"Run tests inside a chroot environment. Note that this\n\t\t"
		"doesn't change the root of the testrunner itself,\n\t\t"
		"only the tests will have the new root folder set.\n");
	printf ("\nHost-based SSH Execution:\n");
	printf ("  -t [USER@]ADDRESS[:PORT], --target=[USER@]ADDRESS[:PORT]\n\t\t"
		"Enable host-based testing. If given, commands are executed\n\t\t"
		"from test control PC (host) side. ADDRESS is the ipv4 address\n\t\t"
		"of the system under test. Behind the scenes, host-based\n\t\t"
		"testing uses the external execution described below with SSH\n\t\t"
		"and SCP.\n");
	printf ("  -R[ACTION], --resume[=ACTION]\n\t\t"
		"Resume testrun when ssh connection failure occurs.\n\t\t"
		"The possible ACTIONs after resume are:\n\t\t"
		"  exit      Exit after current test set\n\t\t"
		"  continue  Continue normally to the next test set\n\t\t"
		"The default action is 'exit'.\n"
		);
	printf ("  -i [USER@]ADDRESS[:PORT], --hwinfo-target=[USER@]ADDRESS[:PORT]\n\t\t"
		"Obtain hwinfo remotely. Hwinfo is usually obtained locally or in\n\t\t"
		"case of host-based testing from target address. This option\n\t\t"
		"overrides target address when hwinfo is obtained.\n\t\t"
		"Usage is similar to -t option.\n");
	printf ("  -k KEY, --ssh-key=KEY\n"
	        "\t\tpath to SSH private key file\n");

#ifdef ENABLE_LIBSSH2
	printf ("\nLibssh2 Execution:\n");
	printf ("  -n [USER@]ADDRESS, --libssh2=[USER@]ADDRESS\n\t\t"
	        "Run host based testing with native ssh (libssh2) "
	        "EXPERIMENTAL\n");
#endif
	printf ("\nExternal Execution:\n");
	printf ("  -E EXECUTOR, --executor=EXECUTOR\n\t\t"
		"Use an external command to execute test commands on the\n\t\t"
		"system under test. The external command must accept a test\n\t\t"
		"command as a single additional argument and exit with the\n\t\t"
		"status of the test command. For example, an external executor\n\t\t"
		"that uses SSH to execute test commands could be\n\t\t"
		"\"/usr/bin/ssh user@target\".\n");
	printf ("  -G GETTER, --getter=GETTER\n\t\t"
		"Use an external command to get files from the system under\n\t\t"
		"test. The external getter should contain <FILE> and <DEST>\n\t\t"
		"(with the brackets) where <FILE> will be replaced with the\n\t\t"
		"path to the file on the system under test and <DEST> will be\n\t\t"
		"replaced with the destination directory on the host. If\n\t\t"
		"<FILE> and <DEST> are not specified, they will be appended\n\t\t"
		"automatically. For example, an external getter that uses SCP\n\t\t"
		"to retrieve files could be \"/usr/bin/scp target:'<FILE>' '<DEST>'\".\n");

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
		len = strlen (pwd) + 2;
		opts.output_folder = (char *)malloc (len);
		strncpy (opts.output_folder, pwd, len);
		opts.output_folder[strlen(pwd)] = '/';
		opts.output_folder[strlen(pwd) + 1] = '\0';
	}
	len = strlen(opts.output_folder) + strlen("mkdir -p ") + 1;
	cmd = (char *)malloc (len);
	snprintf (cmd, len, "mkdir -p %s", opts.output_folder);

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
LOCAL int parse_remote_logger(char *url, testrunner_lite_options *opts) 
{
	if (url) {
		opts->remote_logger = strdup(url);
		return 0;
	} else {
		return 1;
	}

}

#ifdef ENABLE_LIBSSH2
/* ------------------------------------------------------------------------- */
/** Parse target address option argument for libssh2 option
 * @param address SUT address.
 * @param opts Options struct containing field(s) to store url
 * @return 0 in success, 1 on failure
 */
LOCAL int parse_target_address_libssh2(char *address, 
                                       testrunner_lite_options *opts) 
{
	const char *token;
	char *param, *p;
	char *param_ptr;
	char *item;
	char *username;
	char *target_address;
	
	if (address) {
		/* Parse username from address */
		item = NULL;
		param = strdup(address);
		
		/* will be modified by strsep */
		param_ptr = param;
		token = "@";
		item = strsep(&param_ptr, token);
		
		/* No username provided, using the host username by default */
		if (!param_ptr) {
			item = getenv("LOGNAME");
			if (!item) {
				fprintf(stderr, "Error: could not get "
					"env for LOGNAME\n");
				free (param);
				return 1;
			}
			username = item;
			target_address = address;
		} else {
			username = item;
			target_address = param_ptr;
		}
		opts->username = strdup(username);
		opts->target_address = strdup(target_address);
		p = strchr (opts->target_address, ':');
		if (p) {
			*p = '\0';
			p++;
			opts->target_port = atoi (p);
			if (opts->target_port < 0 || 
			    opts->target_port > USHRT_MAX) {
				fprintf (stderr, "Invalid port %d\n", 
					 opts->target_port);
				free(param);
				return 1;
			}
		}
		free(param);
		return 0;
	} else {
		return 1;
	}
}
#endif // ENABLE_LIBSSH2

/* ------------------------------------------------------------------------- */
/** Parse key
 * @param key path to ssh key
 * @param opts Options struct containing field(s) to store key filenames
 * @return 0 in success, 1 on failure
 */
LOCAL int parse_key(char *key, testrunner_lite_options *opts) {

	struct stat ssh_key_file;
	int err;
	int key_size;
	char *home_dir;
	char *ssh_key;

	if (!key || !strlen(key)) {
		return 1;
	}

	/* We can't rely that the shell would translate '~' to users home path
	   in every occasion */
	if (strlen(key) > 1 && key[0] == '~') {
		/* ~/file -notation */
		if (key[1] == '/') {
			home_dir = getenv("HOME");
			if (!home_dir) {
				fprintf(stderr, "Fatal: Could not find users home "
				        "directory\n");
				goto error;
			}
		} else {
			/* ~username/file -notation */
			home_dir = "/home/";
		}

		key_size = strlen(home_dir) + 
			strlen(key) - 1 + 1; /* remove '~', add '\0' */
		ssh_key = malloc(key_size);
		snprintf(ssh_key, key_size, "%s%s", home_dir, &key[1]);
	} else {
		ssh_key = malloc(strlen(key) + 1);
		strncpy(ssh_key, key, strlen(key) + 1);
	}

	/* Check that the parameter is an existing file with read access */
	if (stat(ssh_key, &ssh_key_file) == -1) {
			fprintf(stderr, "%s: ssh key file not found: %s\n",
				PROGNAME, ssh_key);
			goto error;
	}

	if (S_ISDIR(ssh_key_file.st_mode)) {
		fprintf(stderr, "%s: '%s' is a directory, not a file\n",
		        PROGNAME, ssh_key);
		goto error;
	}

	if (access(ssh_key, R_OK) < 0) {
		err = errno;
		switch (err) {
		case EACCES:
			fprintf(stderr, "No read access to ssh key %s\n",
			        ssh_key);
			goto error;
		default:
			fprintf(stderr, "Access error to private key %s: %d\n",
			        ssh_key, err);
			goto error;
		}		
	}

	opts->ssh_key = malloc(strlen(ssh_key) + 1);
	strncpy(opts->ssh_key, ssh_key, strlen(ssh_key) + 1);
	if (ssh_key) free (ssh_key);
	return 0;

 error:
	if (ssh_key) free(ssh_key);
	return 1;
}

/* ------------------------------------------------------------------------- */
/** Parse target address option argument for ssh client option
 * @param address SUT address.
 * @param opts Options struct containing field(s) to store url
 * @return 0 in success, 1 on failure
 */
LOCAL int parse_target_address(char *address, testrunner_lite_options *opts) 
{
	char *p; 

	if (address) {
		opts->target_address = strdup(address);
		p = strchr (opts->target_address, ':');
		if (p) {
			*p = '\0';
			p++;
			opts->target_port = atoi (p);
			if (opts->target_port < 0 || 
			    opts->target_port > USHRT_MAX) {
				fprintf (stderr, "Invalid port %d\n", 
					 opts->target_port);
				return 1;
			}
		}
		return 0;
	} else {
		return 1;
	}

}

/* ------------------------------------------------------------------------- */
/** Parse remote getter argument.
 * @param getter Remote getter.
 * @param opts Options struct
 * @return 0 in success, 1 on failure
 */
LOCAL int parse_remote_getter(char *getter, testrunner_lite_options *opts) 
{
	size_t size;

	size = strlen(getter) + strlen(" <FILE> <DEST>") + 1;
	opts->remote_getter = malloc(size);

	strncpy(opts->remote_getter, getter, size);
	if (strstr(getter, "<FILE>") == NULL)
		strcat(opts->remote_getter, " <FILE>");
	if (strstr(getter, "<DEST>") == NULL)
		strcat(opts->remote_getter, " <DEST>");

	return 0;
}

/* ------------------------------------------------------------------------- */
/** Parse target options to create remote executor string using SSH
 * @param opts Options struct
 * @return 0 in success, 1 on failure
 */
LOCAL int parse_default_ssh_executor(testrunner_lite_options *opts)
{
	char portarg [3 + 5 + 1]; /* "-p " + max port size + '\0' */
	int keyarg_len;
	if (opts->ssh_key) {  
		keyarg_len = strlen(opts->ssh_key) + 3 + 1; /* -i + ssh key 
							     * len + '\0'*/
	} else {
		keyarg_len = 1; /* for null termination */
	}
	char keyarg[keyarg_len];
	size_t len;

	if (opts->target_address == NULL) {
		fprintf (stderr, "Missing target address\n");
		return 1;
	}

	portarg[0] = '\0';
	if (opts->target_port)
		snprintf (portarg, 3 + 5 + 1,
			  "-p %u", opts->target_port);
	
	keyarg[0] = '\0';
	if (opts->ssh_key) {
		snprintf (keyarg, keyarg_len + 1, "-i %s", opts->ssh_key);
	}

	len = strlen(SSH_REMOTE_EXECUTOR) + strlen(portarg) +
		strlen(opts->target_address) + keyarg_len + 1;
	opts->remote_executor = malloc(len);
	if (opts->remote_executor == NULL) {
		fprintf (stderr, "Malloc failed\n");
		return 1;
	}

	snprintf(opts->remote_executor, len, SSH_REMOTE_EXECUTOR, portarg,
	        opts->target_address, keyarg);

	return 0;
}

/* ------------------------------------------------------------------------- */
/** Parse target options to create remote getter string using SCP
 * @param opts Options struct
 * @return 0 in success, 1 on failure
 */
LOCAL int parse_default_scp_getter(testrunner_lite_options *opts)
{
	char portarg [3 + 5 + 1]; /* "-P " + max port size + '\0' */
	int keyarg_len;
	if (opts->ssh_key) {  
		keyarg_len = strlen(opts->ssh_key) + 3 + 1; /* -i + ssh key len + '\0'*/
	} else {
		keyarg_len = 1; /* for null termination */
	}
	char keyarg[keyarg_len];
	size_t len;

	portarg[0] = '\0';
	if (opts->target_port)
		snprintf (portarg, 3 + 5 + 1, "-P %u", opts->target_port);

	keyarg[0] = '\0';
	if (opts->ssh_key) {
		snprintf (keyarg, keyarg_len + 1, "-i %s", opts->ssh_key);
	}

	len = strlen(SCP_REMOTE_GETTER) + strlen(portarg) +
		strlen(opts->target_address) + keyarg_len + 1;
	opts->remote_getter = malloc(len);
	if (opts->remote_getter == NULL) {
		fprintf (stderr, "Malloc failed\n");
		return 1;
	}

	snprintf(opts->remote_getter, len, SCP_REMOTE_GETTER, portarg, keyarg,
		 opts->target_address);

	return 0;
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
		opts->chroot_folder = strdup (folder);

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
/** Save the rich-core dumps folder setting
 * @param folder path to rich-core dumps in the device.
 * @param opts options struct containing field(s)
 * @return 0 on success; otherwise 1
 */
LOCAL int set_rich_core_dumps(char *folder, testrunner_lite_options *opts)
{
	size_t pattern_len;

	if (folder && strlen (folder) > 0) {
		pattern_len = strlen (folder) + 2;
		opts->rich_core_dumps = (char *) malloc (pattern_len);

		strncpy (opts->rich_core_dumps, folder, pattern_len);
		if (folder[strlen (folder) - 1] != '/') {
			strcat (opts->rich_core_dumps, "/");
		}
		return 0;
	} else {
		return 1;
	}
}
/* ------------------------------------------------------------------------- */
/** Parse logid option argument string. Only letters and digits are accepted.
 * @param logid Option argument string
 * @param opts Options struct
 * @return 0 in success, 1 on failure
 */
LOCAL int parse_logid(char *logid, testrunner_lite_options *opts)
{
	char *p;

	for (p = logid; *p != '\0'; p++) {
		if (!isalnum(*p)) {
			fprintf(stderr,
				"%s: invalid char '%c' in logid argument\n",
				PROGNAME,
				*p);
			return 1;
		}
	}

	opts->logid = strdup(logid);
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Parse target address where to ask hwinfo
 * @param address SUT address.
 * @param opts Options struct containing field(s) to store url
 * @return 0 in success, 1 on failure
 */
LOCAL int parse_target_address_hwinfo(char *address, testrunner_lite_options *opts) {
	char *p;

	if (address) {
		opts->hwinfo_target = strdup(address);
		p = strchr (opts->hwinfo_target, ':');
		if (p) {
			*p = '\0';
			p++;
			opts->hwinfo_port = atoi (p);
			if (opts->hwinfo_port < 0 ||
			    opts->hwinfo_port > USHRT_MAX) {
				fprintf (stderr, "Invalid port %d\n",
					 opts->hwinfo_port);
				return 1;
			}
		}
		return 0;
	} else {
		return 1;
	}
}
/* ------------------------------------------------------------------------- */
/** Parse UTF-8 limit option
 * @param limit Limit as a string
 * @param opts Options struct
 * @return 0 in success, 1 on failure
 */
LOCAL int parse_utf8_limit(char *limit, testrunner_lite_options *opts) {
	long int value = 0;
	char *endptr = NULL;

	value = strtol(limit, &endptr, 10);
	if (value > 0 && value < 5 && *endptr == '\0') {
		opts->max_utf8_bytes = value;
		return 0;
	}

	fprintf(stderr, "Invalid value for option utf8-limit\n");
	return 1;
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
	int power_flag = 0;
	int opt_char, option_idx;
	char *address = NULL;
	char *executor = NULL;
#ifdef ENABLE_LIBSSH2
	int libssh2 = 0;
#endif
	in_port_t port = 0;
	opts.remote_executor = NULL;
	opts.remote_getter = NULL;
#ifdef ENABLE_LIBSSH2
	opts.libssh2 = 0;
#endif
	opts.ssh_key = NULL;
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
			{"vcs-url", required_argument, NULL, 'u'},
			{"package-url", required_argument, NULL, 'U'},
			{"ci", no_argument, &opts.disable_schema},
			{"semantic", no_argument, &opts.semantic_schema},
			{"validate-only", no_argument, &A_flag},
			{"no-hwinfo", no_argument, &opts.skip_hwinfo, 1},
			{"target", required_argument, NULL, 't'},
			{"executor", required_argument, NULL, 'E'},
			{"getter", required_argument, NULL, 'G'},
			{"chroot", required_argument, NULL, 'C'},
#ifdef ENABLE_LIBSSH2
			{"libssh2", required_argument, NULL, 'n'},
			{"ssh-key", required_argument, NULL, 'k'},
#endif
			{"print-step-output", no_argument, 
			 &opts.print_step_output, 1},
			{"disable-measurement-verdict", no_argument, 
			 &opts.no_measurement_verdicts, 1},
			{"measure-power", no_argument, &power_flag, 1},
			{"resume", optional_argument, NULL, 'R'},
			{"logid", required_argument, NULL,
			 TRLITE_LONG_OPTION_LOGID},
			{"hwinfo-target", required_argument, NULL, 'i'},
			{"rich-core-dumps", required_argument, NULL, 'd'},
			{"utf8-limit", required_argument, NULL,
			 TRLITE_LONG_OPTION_UTF8_LIMIT},
			{"core-upload-timeout", required_argument, NULL, 'T'},
			{0, 0, 0, 0}
		};


	LIBXML_TEST_VERSION

	memset (&opts, 0x0, sizeof(testrunner_lite_options));
        memset (&hwinfo, 0x0, sizeof(hwinfo));

	opts.output_type = OUTPUT_TYPE_XML;
	opts.run_automatic = opts.run_manual = 1;
	opts.ssh_key = NULL;
	gettimeofday (&created, NULL);
	signal (SIGINT, handle_sigint);
	signal (SIGTERM, handle_sigterm);
	if (argc == 1)
		h_flag = 1;

	while (1) {
		option_idx = 0;
     
		opt_char = getopt_long (argc, argv, 
					":hVaAHSMsmcPd:C:f:o:e:l:r:u:U:L:t:E:G:v::k:T:"
#ifdef ENABLE_LIBSSH2
					"n:"
#endif
					"R::i:", testrunnerlite_options,
					&option_idx);
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
			opts.input_filename = strdup (optarg); 
			break;
		case 'o':
			opts.output_filename = strdup (optarg); 
			break;
		case 'e':
			opts.environment = strdup (optarg); 
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
		case 'E':
			opts.remote_executor = strdup (optarg);
			break;
		case 'G':
			if (parse_remote_getter(optarg, &opts) != 0) {
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
				goto OUT;
			}
			break;
		case 'C':
			if (parse_chroot_folder(optarg, &opts) != 0) {
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
				goto OUT;
			}
			break;
#ifdef ENABLE_LIBSSH2
		case 'n':
			opts.libssh2 = 1;
			if (parse_target_address_libssh2(optarg, &opts) != 0) {
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
				goto OUT;
			}
			break;
#endif
		case 'k':
			if (parse_key(optarg, &opts) != 0) {
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
				goto OUT;
			}
			break;
		case 'P':
			opts.print_step_output = 1;
			break;
		case 'M':
			opts.no_measurement_verdicts = 1;
			break;

		case 'u':
			opts.vcsurl = strdup (optarg);
			break;
		case 'U':
			opts.packageurl = strdup (optarg);
			break;
		case 'R':
			if (optarg) {
				if (strncmp(optarg, "exit", 4) == 0) {
					opts.resume_testrun =
						RESUME_TESTRUN_ACTION_EXIT;
					break;
				}
				else if (strncmp(optarg, "continue", 8) == 0) {
					opts.resume_testrun =
						RESUME_TESTRUN_ACTION_CONTINUE;
					break;
				}
				fprintf (stderr, "invalid argument value: %s\n",
					 optarg);
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
				goto OUT;
			}
			/* use default action if argument is not given */
			opts.resume_testrun = RESUME_TESTRUN_ACTION_EXIT;
			break;
		case 'i':
			if (parse_target_address_hwinfo(optarg, &opts) != 0) {
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
				goto OUT;
			}
			break;
		case 'd':
			if (set_rich_core_dumps (optarg, &opts) != 0) {
                                retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
                                goto OUT;
			}
			break;
		case 'T':
			opts.core_upload_timeout = atoi (optarg);
			break;
		case TRLITE_LONG_OPTION_LOGID:
			if (parse_logid(optarg, &opts) != 0) {
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
				goto OUT;
			}
			break;
		case TRLITE_LONG_OPTION_UTF8_LIMIT:
			if (parse_utf8_limit(optarg, &opts) != 0) {
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
				goto OUT;
			}
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
		default:
			break;
		}
	}

	/*
	 * Convert target address to remote executor/getter using SSH/SCP
	 */
#ifdef ENABLE_LIBSSH2
	if (!opts.libssh2) {
#endif
	if (opts.target_address) {
		if (opts.remote_executor || opts.remote_getter) {
			fprintf (stderr,
				"%s: -t and -E/-G are mutually exclusive\n",
				PROGNAME);
			retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
			goto OUT;
		}

		if ((parse_default_ssh_executor(&opts) != 0) ||
		    (parse_default_scp_getter(&opts) != 0)) {
			fprintf (stderr,
				"%s: Failed to parse SSH/SCP executor/getter\n",
				PROGNAME);
			retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
			goto OUT;
		}
	}
#ifdef ENABLE_LIBSSH2
	}
#endif

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

#ifdef ENABLE_LIBSSH2
	if (opts.libssh2) {
		if (opts.chroot_folder || opts.remote_executor || opts.remote_getter) {
			fprintf (stderr,
				"%s: -n is mutually exclusive with -C/-E/-G\n",
				PROGNAME);
			retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
			goto OUT;
		}
	}
#endif

	if (opts.chroot_folder && (opts.remote_executor || opts.remote_getter)) {
		fprintf (stderr,
			"%s: -C and remote execution (-t or -E/-G) are mutually exclusive\n",
			PROGNAME);
		retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
		goto OUT;
	}

	if ((opts.remote_executor && !opts.remote_getter) ||
	    (!opts.remote_executor && opts.remote_getter)) {
		fprintf (stderr,
			"%s: If either -E or -G is given, both must be given\n",
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

	if (power_flag)
		opts.measure_power = 1;

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
	if (executor_init (&opts) != 0) {
		LOG_MSG(LOG_ERR, "Executor init failed... exiting");
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
		opts.environment = strdup ("hardware");
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
	if (!opts.skip_hwinfo) {
		/* Save old values */
		address = opts.target_address;
		port = opts.target_port;
		executor = opts.remote_executor;
#ifdef ENABLE_LIBSSH2
		libssh2 = opts.libssh2;
#endif

		/* If hwinfo target is given change target address and port */
		if(opts.hwinfo_target) {
			opts.target_address = opts.hwinfo_target;
			opts.target_port = opts.hwinfo_port;
#ifdef ENABLE_LIBSSH2
			opts.libssh2 = 0;
#endif

			if(parse_default_ssh_executor(&opts) != 0) {
				fprintf (stderr,
				"%s: Failed to parse hwinfo target\n",
				PROGNAME);
				retval = TESTRUNNER_LITE_INVALID_ARGUMENTS;
				goto OUT;
			}

			/* If remote_executor was null initialize executor so we can
			* obtain hwinfo from remote device */
			if (executor_init (&opts) != 0) {
				LOG_MSG(LOG_ERR, "Executor init failed... exiting");
				goto OUT;
			}
		}

		LOG_MSG (LOG_DEBUG, "Remote executor %s", opts.remote_executor);
		read_hwinfo (&hwinfo);

		/* Return original values */
		opts.target_address = address;
		opts.target_port = port;
		opts.remote_executor = executor;
#ifdef ENABLE_LIBSSH2
		opts.libssh2 = libssh2;
#endif
	}
	
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
	clean_hwinfo(&hwinfo);
	if (opts.input_filename) free (opts.input_filename);
	if (opts.output_filename) free (opts.output_filename);
	if (opts.output_folder) free (opts.output_folder);
	if (opts.environment) free (opts.environment);
	if (opts.remote_logger) free (opts.remote_logger);
	if (opts.target_address) free (opts.target_address);
	if (opts.remote_executor) free (opts.remote_executor);
	if (opts.remote_getter) free (opts.remote_getter);
	if (opts.packageurl) free (opts.packageurl);
	if (opts.vcsurl) free (opts.vcsurl);
	if (opts.logid) free (opts.logid);
#ifdef ENABLE_LIBSSH2
	if (opts.username) free (opts.username);
#endif
	if (opts.ssh_key) free (opts.ssh_key);
	if (opts.rich_core_dumps) free (opts.rich_core_dumps);
	if (filter_string) free (filter_string);
	if (bail_out == 255+SIGINT) {
		signal (SIGINT, SIG_DFL);
		raise (SIGINT);
	} else if (bail_out == 255+SIGTERM) {
		signal (SIGTERM, SIG_DFL);
		raise (SIGTERM);
	} else if (bail_out) retval = TESTRUNNER_LITE_REMOTE_FAIL;

	return retval; 
}	


/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
