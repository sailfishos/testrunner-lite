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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <signal.h>
#include <ctype.h>
#include <uuid/uuid.h>

#include "testrunnerlite.h"
#include "testdefinitionparser.h"
#include "testresultlogger.h"
#include "testfilters.h"
#include "testmeasurement.h"
#include "executor.h"
#include "remote_executor.h"
#include "manual_executor.h"
#include "utils.h"
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
LOCAL td_td    *current_td = NULL;    /* Test definition currently executed */
LOCAL td_suite *current_suite = NULL; /* Suite currently executed */
LOCAL td_set   *current_set = NULL;   /* Set currently executed */
LOCAL xmlChar  *cur_case_name = BAD_CAST"";   /* Name of the current case */ 
LOCAL int       cur_step_num;         /* Number of current step within case */

LOCAL int passcount = 0;
LOCAL int failcount = 0;
LOCAL int casecount = 0;
/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */
#define CORE_PATTERN_COMMAND "sysctl -q -w kernel.core_pattern=\'|/usr/sbin/rich-core-dumper "\
	"--pid=%p --signal=%s --name=%e --tc-uuid=<UUID>\'"
/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL void process_td(td_td *);
/* ------------------------------------------------------------------------- */
LOCAL void end_td();
/* ------------------------------------------------------------------------- */
LOCAL void process_hwiddetect();
/* ------------------------------------------------------------------------- */
LOCAL void process_suite(td_suite *);
/* ------------------------------------------------------------------------- */
LOCAL void end_suite ();
/* ------------------------------------------------------------------------- */
LOCAL void process_set(td_set *);
/* ------------------------------------------------------------------------- */
LOCAL int process_case (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int case_result_fail (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int process_get (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int process_get_case (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int step_execute (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int prepost_steps_execute (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int step_result_fail (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int step_post_process (const void *, const void *);
/* ------------------------------------------------------------------------- */
#ifdef ENABLE_EVENTS
LOCAL int event_execute (const void *data, const void *user);
/* ------------------------------------------------------------------------- */
#endif
LOCAL int set_device_core_pattern (const char *);
/* ------------------------------------------------------------------------- */
LOCAL int fetch_rich_core_dumps (const char *, xmlListPtr);
/* ------------------------------------------------------------------------- */

/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/** Puts crashids from string to list
 * @param ids crash identifiers separetated by newlined
 * @param crashids list to store found crash identifier into
 */
LOCAL void parse_crashids(char *ids, xmlListPtr crashids)
{
	char *p;
	char *id;

	if (!ids || strlen (ids) == 0)
	      return;
	p = strtok (ids, "\n");

	while (p) {
		id = strdup(p);
                LOG_MSG (LOG_DEBUG, "Adding crash id: %s \n", id);
		xmlListInsert (crashids, id);
		p = strtok (NULL, "\n");
	}
	
}
/* ------------------------------------------------------------------------- */
/** Fetch rich-core dumps from the device
 * @param uuid String identifier
 * @param crashids list to store found crash identifier into
 * @return 1, if found; otherwise 0
 */
LOCAL int fetch_rich_core_dumps (const char *uuid, xmlListPtr crashids)
{
        exec_data edata;
        char *command;
	char *get_pattern;
        size_t pattern_len;
	td_file *file;
	int ret = 0;
#define RICHCORE_SEARCH_COMMAND "rich-core-find-uuid"
#define RICH_CORE_SEARCH_PATTERN "%s*%s*"

        memset (&edata, 0x0, sizeof (exec_data));
        init_exec_data(&edata);
        edata.soft_timeout = COMMON_SOFT_TIMEOUT;
        edata.hard_timeout = COMMON_HARD_TIMEOUT;
	edata.redirect_output = REDIRECT_OUTPUT;

        pattern_len = strlen (opts.rich_core_dumps) + strlen (uuid) + 3;
        get_pattern = (char *) malloc (pattern_len);
	snprintf (get_pattern, pattern_len, RICH_CORE_SEARCH_PATTERN,
		 opts.rich_core_dumps, uuid);

        command = (char *) malloc (strlen(RICHCORE_SEARCH_COMMAND) +
				   strlen (opts.rich_core_dumps) + 
				   strlen (uuid) + 3);
        snprintf (command, strlen(RICHCORE_SEARCH_COMMAND) +
		  strlen (opts.rich_core_dumps) + 
		  strlen (uuid) + 3,
		  "%s %s %s", 
		  RICHCORE_SEARCH_COMMAND, opts.rich_core_dumps, uuid);

        LOG_MSG (LOG_DEBUG, "%s:  Executing command: %s", PROGNAME, command);
        execute(command, &edata);

        free (command);

        if (edata.result != 0) {
                LOG_MSG (LOG_DEBUG, "%s: Rich core dumps not found with UUID: %s \n", 
			PROGNAME, uuid);
		goto out;
        } else {
		parse_crashids ((char *)edata.stdout_data.buffer, crashids);
	}

	file = (td_file *)malloc (sizeof (td_file));
		file->filename = (xmlChar *)get_pattern;
                file->delete_after = 1;
                file->measurement = 0;
                file->series = 0;

	process_get (file, 0);
	ret = 1;
out:
        if (edata.stdout_data.buffer) free (edata.stdout_data.buffer);
        if (edata.stderr_data.buffer) free (edata.stderr_data.buffer);
        if (edata.failure_info.buffer) free (edata.failure_info.buffer);

	if (get_pattern) free (get_pattern);
	if (file) free (file);

        return ret;
}

/** Set device core pattern
 * @param uuid String identifier
 * @return always 1 
 */
LOCAL int set_device_core_pattern (const char *uuid)
{
	exec_data edata;
	char *command, *p;
	size_t command_len;

	memset (&edata, 0x0, sizeof (exec_data));
        init_exec_data(&edata);
        edata.soft_timeout = COMMON_SOFT_TIMEOUT;
        edata.hard_timeout = COMMON_HARD_TIMEOUT;

	command_len = strlen(CORE_PATTERN_COMMAND) + strlen(uuid) + 1;
	command = strdup (CORE_PATTERN_COMMAND);

	p = command;
	command = replace_string (command, "<UUID>", uuid);
	free (p);

        LOG_MSG (LOG_DEBUG, "%s:  Executing command: %s", PROGNAME, command);
        execute(command, &edata);

        if (edata.result) {
                LOG_MSG (LOG_WARNING, "%s: %s failed: %s\n", PROGNAME, command,
                         (char *)(edata.stderr_data.buffer ?
                                  edata.stderr_data.buffer : 
                                  BAD_CAST "no info available"));
        }

	if (edata.stdout_data.buffer) free (edata.stdout_data.buffer);
        if (edata.stderr_data.buffer) free (edata.stderr_data.buffer);
        if (edata.failure_info.buffer) free (edata.failure_info.buffer);

	free (command);

	return 1;
}
#ifdef ENABLE_EVENTS
/** Process event
 *  @param data event data
 *  @param user step data
 *  @return 1 if event is successfully processed, 0 if not
 */
LOCAL int event_execute (const void *data, const void *user)
{
	td_event *event = (td_event *)data;
	int ret = 0;

	if (event->type == EVENT_TYPE_SEND) {
		ret = send_event(event);
	}

	if (event->type == EVENT_TYPE_WAIT) {
		ret = wait_for_event(event);
	}
	
	return ret;
}
#endif	/* ENABLE_EVENTS */
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

	cur_step_num++;

	memset (&edata, 0x0, sizeof (exec_data));
	if (bail_out) {
		step->has_result = 1;
		step->return_code = bail_out;
		if (global_failure) {
			step->failure_info = xmlCharStrdup (global_failure);
			if (!c->failure_info) {
				c->failure_info = xmlCharStrdup(global_failure);
			}
		}
		c->case_res = CASE_FAIL;

		return 1;
	}

#ifdef ENABLE_EVENTS
	if (step->event) {
		/* just process the event */
		if (!event_execute(step->event, step)) {
			step->return_code = 1;
			res = CASE_FAIL;
			LOG_MSG (LOG_INFO, "EVENT: '%s' failed\n",
					 step->event->resource);
		}
		step->has_result = 1;
		goto out;
	}
#endif	/* ENABLE_EVENTS */
	
	if (step->manual) {
		if (c->dummy) {
			LOG_MSG (LOG_WARNING, 
				 "manual pre/post steps not supported");
			goto out;
		}
		if (!c->gen.manual)
			LOG_MSG (LOG_WARNING, "Executing manual step from "
				 "automatic case %s "
				 "(generally not a good idea)",
				 c->gen.name);
		res = execute_manual (step);
		goto out;
	}
	
	init_exec_data(&edata);
	
	edata.redirect_output = REDIRECT_OUTPUT;
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
			c->failure_info = xmlCharStrdup ((char *)
							 step->failure_info);
			
			LOG_MSG (LOG_INFO, "FAILURE INFO: %s",
				 step->failure_info);
		}
		
		step->pgid = edata.pgid; 
		step->pid = edata.pid;
		step->has_result = 1;
		step->return_code = edata.result;
		step->start = edata.start_time;
		step->end = edata.end_time;

		/*
		** Post and pre steps fail only if the expected result is 
		*  specified
		*/
		if (c->dummy) 
			if (!step->has_expected_result)
				goto out;
		/*
		** Testrunner-lite may have killed the step command on timeout
		** in this case we do not trust the return value.
		*/
		if (edata.signaled) { 
			step->fail = 1;
			LOG_MSG (LOG_INFO, "STEP: %s terminated by signal %d",
				 step->step, edata.signaled);
			res = CASE_FAIL;
		} else if (step->return_code != step->expected_result) {
			LOG_MSG (LOG_INFO, "STEP: %s return %d expected %d",
				 step->step, step->return_code, 
				 step->expected_result);
			res = CASE_FAIL;
		}
	}
 out:
	if (res != CASE_PASS)
		c->case_res = res;
	
	
	return (res == CASE_PASS);
}
/* ------------------------------------------------------------------------- */
/** Process pre/post steps.
 *  @param data steps data
 *  @param user dummy case data
 *  @return 1 always
 */
LOCAL int prepost_steps_execute (const void *data, const void *user)
{
	td_steps *steps = (td_steps *)data;
	td_case *dummy = (td_case *)user;
	
	dummy->gen.timeout = steps->timeout;
	
	if (xmlListSize(steps->steps) > 0) {
		xmlListWalk (steps->steps, step_execute, dummy);
	}
	
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Set fail result for test step
 *  @param data step data
 *  @param user failure info string
 *  @return 1 always
 */
LOCAL int step_result_fail (const void *data, const void *user) 
{
	td_step *step = (td_step *)data;
	char *failure_info = (char *)user;

	step->has_result = 1;
	step->fail = 1;
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
	if (step->manual) 
		goto out;
	/* ... or filtered ones ... */
	if (c->filtered)
		goto out;

	/* ... or ones that are not run ... */
	if (!step->start)
		goto out;

	/* ... or ones that do not have process group ... */
	if (!step->pgid)
		goto out;

	/* The required PID file from remote has been cleaned already.
	   Thus commented this useless remote_kill call */
	/* if (opts.remote_executor && !bail_out) { */
	/* 	remote_kill (opts.remote_executor, step->pid, SIGKILL); */
	/* } */

	if (step->pgid > 0) {
		kill_pgroup(step->pgid, SIGKILL);
	}
	
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
	char uuid_buf[36];
	uuid_t uuid_gen;
	char *pos = NULL;

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
	if (filter_case (c)) {
		LOG_MSG (LOG_INFO, "Test case %s is filtered", c->gen.name);
		return 1;
	}
	if (c->state && !xmlStrcmp (c->state, BAD_CAST "Design")) {
		LOG_MSG (LOG_INFO, "Skipping case in Design state (%s)",
			 c->gen.name);
		c->case_res = CASE_NA;
		return 1;
	}

	cur_case_name = c->gen.name;
	LOG_MSG (LOG_INFO, "Starting test case %s", c->gen.name);
	casecount++;

	if (opts.rich_core_dumps != NULL) {
		/* Create UUID to map test case and rich-core dump. */
		uuid_generate (uuid_gen);
		
		if (uuid_is_null (uuid_gen)) {
			LOG_MSG (LOG_WARNING, "Failed to generate UUID.");
		}
		else {
			uuid_unparse (uuid_gen, uuid_buf);
			/* UUID format is xxxx-xxxx-xxxx-xxxx. Replace dashes (-) */
			/* with zeros (0). */
			while ((pos = strchr(uuid_buf, '-')) != NULL) {
				*pos = '0';
			}
			set_device_core_pattern (uuid_buf);
		}
	}

	if (opts.measure_power) {
		if (system("hat_ctrl -stream:5:s1-2:f" MEASUREMENT_FILE
			   ":0 > /dev/null 2>&1") != 0) {
			LOG_MSG (LOG_WARNING, "Failure in power measurement initialization");
		}
	}
	
	c->case_res = CASE_PASS;
	if (c->gen.timeout == 0)
		c->gen.timeout = COMMON_SOFT_TIMEOUT; /* the default one */
	
	if (c->gen.manual && opts.run_manual)
		pre_manual (c);
	if (xmlListSize (c->steps) == 0) {
		LOG_MSG (LOG_WARNING, "Case with no steps (%s).",
			 c->gen.name);
		c->case_res = CASE_NA;
	}
	cur_step_num = 0;
	
	xmlListWalk (c->steps, step_execute, data);
	xmlListWalk (c->steps, step_post_process, data);
	
	if (c->gen.manual && opts.run_manual)
		post_manual (c);

	if (opts.measure_power) {
		if (system("hat_ctrl -stream:0 > /dev/null 2>&1")) {
			LOG_MSG (LOG_WARNING, "Failure in stopping power measurement");
		}
		process_current_measurement(MEASUREMENT_FILE, c);
	}

	if (opts.rich_core_dumps != NULL && 
	    fetch_rich_core_dumps (uuid_buf, c->crashids)) {
		c->rich_core_uuid = xmlCharStrdup (uuid_buf);
	}
	xmlListWalk (c->gets, process_get_case, c);
	
	LOG_MSG (LOG_INFO, "Finished test case %s Result: %s",
		 c->gen.name, case_result_str(c->case_res));
	passcount += (c->case_res == CASE_PASS);
	failcount += (c->case_res == CASE_FAIL);
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Set case result to fail
 *  @param data case data
 *  @param user failure info
 *  @return 1 always
 */
LOCAL int case_result_fail (const void *data, const void *user)
{

	td_case *c = (td_case *)data;
	char *failure_info = (char *)user;

	LOG_MSG (LOG_DEBUG, "Setting FAIL result for case %s", c->gen.name);

	c->case_res = CASE_FAIL;
	c->failure_info = xmlCharStrdup (failure_info);

	xmlListWalk (c->steps, step_result_fail, user);
	
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Process set get data. 
 *  @param data get file data
 *  @param user not used
 *  @return 1 always
 */
LOCAL int process_get (const void *data, const void *user)
{

	td_file *file = (td_file *)data;
	char *command;
	char *fname;
	char *tmpname;
	exec_data edata;
	char *p;
	char *executor = opts.remote_executor;
	int command_len;
#ifdef ENABLE_LIBSSH2
	int key_param_len = 0;
	char *remote = opts.target_address;
#endif
	if (bail_out) {
		return 1;
	}

	memset (&edata, 0x0, sizeof (exec_data));
	init_exec_data(&edata);
	edata.soft_timeout = COMMON_SOFT_TIMEOUT;
	edata.hard_timeout = COMMON_HARD_TIMEOUT;

	if (opts.chroot_folder) {
		/* Tell executor not to execute cp command in chroot
		   but in "normal" environment */
		edata.disobey_chroot = 1;

		/* source file must be prefixed with chroot dir */
		fname = (char *)malloc(strlen(opts.chroot_folder) +
				       strlen((char *)file->filename) + 2);
		strcpy(fname, opts.chroot_folder);
		/* strcat(fname, "/"); */

		tmpname = malloc (strlen((char *)file->filename) + 1);
		trim_string ((char *)file->filename, tmpname);

		strcat(fname, tmpname);
		free(tmpname);
	} else {
		fname = malloc (strlen((char *)file->filename) + 1);
		trim_string ((char *)file->filename, fname);
	}
	command_len = strlen ("rm -f ") + strlen (fname) + 1;
	/*
	** Compose command 
	*/
#ifdef ENABLE_LIBSSH2
	if (opts.libssh2) {
		if (opts.ssh_key) {
			/* length of "-i keyfile + \'0'" */
			key_param_len = strlen(opts.ssh_key) + 3 + 1;			
		} else {
			key_param_len = 1;
		}

		char key_param[key_param_len];

		if (opts.ssh_key) {
			snprintf(key_param, key_param_len, "-i %s", opts.ssh_key);
		} else {
			key_param[0] = '\0';
		}
		
		opts.target_address = NULL; /* execute locally */
		command_len = strlen ("scp ") +
			strlen (opts.username) + 1 +
			strlen (fname) +
			strlen (opts.output_folder) +
			strlen (remote) + 30 + 
			key_param_len;
		command = (char *)malloc (command_len);
		if (opts.target_port)
			snprintf (command, command_len,
				  "scp -P %u ", opts.target_port);
		else
			snprintf (command, command_len, "scp ");
		p = (char *)(command + (strlen (command)));
		snprintf (p, command_len - strlen(command),
			  "%s %s@%s:\'%s\' %s", key_param, opts.username,
			  remote, fname, opts.output_folder);
	} else
#endif
	if (executor) {
		opts.remote_executor = NULL; /* execute locally */
		command = replace_string (opts.remote_getter, "<FILE>", fname);
		p = command;
		command = replace_string (command, "<DEST>", opts.output_folder);
		free(p);
	} else {
		command_len = strlen ("cp ") + strlen (fname) +
			strlen (opts.output_folder) + 2;
		command = (char *)malloc (command_len);
		snprintf (command, command_len, "cp %s %s", fname,
			 opts.output_folder);
	}

	LOG_MSG (LOG_DEBUG, "%s:  Executing command: %s", PROGNAME, command);
	/*
	** Execute it
	*/
	execute(command, &edata);

	if (edata.result) {
		LOG_MSG (LOG_INFO, "%s: %s failed: %s\n", PROGNAME, command,
			 (char *)(edata.stderr_data.buffer ?
				  edata.stderr_data.buffer : 
				  BAD_CAST "no info available"));
	}
#ifdef ENABLE_LIBSSH2
	opts.target_address = remote;
#endif
	opts.remote_executor = executor;
	if (edata.stdout_data.buffer) free (edata.stdout_data.buffer);
	if (edata.stderr_data.buffer) free (edata.stderr_data.buffer);
	if (edata.failure_info.buffer) free (edata.failure_info.buffer);

	if (!file->delete_after)
		goto out;

	memset (&edata, 0x0, sizeof (exec_data));
	init_exec_data(&edata);
	edata.soft_timeout = COMMON_SOFT_TIMEOUT;
	edata.hard_timeout = COMMON_HARD_TIMEOUT;
	snprintf (command, command_len, "rm -f %s", fname);
	LOG_MSG (LOG_DEBUG, "%s:  Executing command: %s", PROGNAME, command);
	execute(command, &edata);
	if (edata.result) {
		LOG_MSG (LOG_WARNING, "%s: %s failed: %s\n", PROGNAME, command,
			 (char *)(edata.stderr_data.buffer ?
				  edata.stderr_data.buffer : 
				  BAD_CAST "no info available"));
	}
	if (edata.stdout_data.buffer) free (edata.stdout_data.buffer);
	if (edata.stderr_data.buffer) free (edata.stderr_data.buffer);
	if (edata.failure_info.buffer) free (edata.failure_info.buffer);

 out:
	free (command);
	free (fname);
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Process case get data. 
 *  @param data get file data
 *  @param user case data
 *  @return 1 always
 */
LOCAL int process_get_case (const void *data, const void *user)
{
	int ret = 0;
	td_file *file = (td_file *)data;
	td_case *c = (td_case *)user;
	char *trimmed_name, *fname, *filename, *failure_str = NULL;
	int measurement_verdict = CASE_PASS;
	size_t len;

	ret = process_get (data, NULL);
	if (!ret)
		LOG_MSG (LOG_WARNING, "get file processing failed");

	if (bail_out)
		return 1;

	/* return if file not specified to contain measurement data */
	if (!file->measurement)
		return 1;
	
	trimmed_name = malloc (strlen ((char *)file->filename) + 1);
	trim_string ((char *)file->filename, trimmed_name);
	fname = strrchr (trimmed_name, '/');
	if (!fname)
		fname = trimmed_name;
	else
		fname ++;
	len = strlen((char *)fname) + 2 + strlen (opts.output_folder);
	filename = malloc (len);
	snprintf (filename, len, "%s%s", opts.output_folder, fname);

	ret = get_measurements (filename, c, file->series);
	free (trimmed_name);
	free (filename);
	
	/* Evaluate measurements only if case is otherwise passed ... */
	if (c->case_res != CASE_PASS)
		return 1;
	/* ... and -M flag is not set */ 
	if (opts.no_measurement_verdicts)
		return 1;
	
	/* ... and measurement getting was succesfull */
	if (ret) {
		c->case_res = CASE_FAIL;
		c->failure_info = xmlCharStrdup ("Failed to process "
						 "measurement file");
		return 1;
	}
	
	ret = eval_measurements (c, &measurement_verdict,
				 &failure_str, file->series);
	if (ret)
		return 1;
	if (measurement_verdict == CASE_FAIL) {
		LOG_MSG (LOG_INFO, "Failing test case %s (%s)", 
			 c->gen.name, failure_str ? failure_str : 
			 "no info");
		c->case_res = CASE_FAIL;
		if (failure_str) 
			c->failure_info = BAD_CAST failure_str;
	}
			
	return 1;
}

/* ------------------------------------------------------------------------- */
/** Process test definition
 *  @param *td Test definition data
 */
LOCAL void process_td (td_td *td)
{
	current_td = td;
	write_td_start (td);
}
/* ------------------------------------------------------------------------- */
/** Do test definition cleaning
 */
LOCAL void end_td ()
{
	write_td_end (current_td);
	td_td_delete (current_td);
	current_td = NULL;
}
/* ------------------------------------------------------------------------- */
/** Process hwiddetect: Run detector command and store result in current td
 */
LOCAL void process_hwiddetect ()
{
	exec_data edata;
	char* trimmed = NULL;
	size_t length = 0;

	if (current_td && current_td->hw_detector) {
		init_exec_data(&edata);
		edata.redirect_output = REDIRECT_OUTPUT;
		edata.soft_timeout = COMMON_SOFT_TIMEOUT;
		edata.hard_timeout = COMMON_HARD_TIMEOUT;

		execute((char*)current_td->hw_detector, &edata);

		if (edata.result != EXIT_SUCCESS) {
			LOG_MSG (LOG_WARNING, "Running HW ID detector "
				 "failed with return value %d",
				 edata.result);
		} else if (edata.stdout_data.buffer) {
			/* remove possible whitespace, linefeeds, etc. */
			length = strlen((char*)edata.stdout_data.buffer);
			trimmed = (char*)malloc(length + 1);
			trim_string((char*)edata.stdout_data.buffer, trimmed);

			current_td->detected_hw = xmlCharStrdup(trimmed);
			LOG_MSG (LOG_INFO, "Detected HW ID '%s'",
				 current_td->detected_hw);
		}

		clean_exec_data(&edata);
	}

	free(trimmed);
}
/* ------------------------------------------------------------------------- */
/** Do processing on suite, currently just writes the pre suite tag to results
 *  @param s suite data
 */
LOCAL void process_suite (td_suite *s)
{
	LOG_MSG (LOG_INFO, "Test suite: %s", s->gen.name);

	write_pre_suite (s);
	current_suite = s;
	
}
/* ------------------------------------------------------------------------- */
/** Suite end function, write suite and delete the current_suite
 */
LOCAL void end_suite ()
{
	write_post_suite (current_suite);
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
	td_steps *steps;
	/*
	** Check that the set is not filtered
	*/
	if (filter_set (s)) {
		LOG_MSG (LOG_INFO, "Test set %s is filtered", s->gen.name);
		goto skip_all;
	}

	/*
	** User defined HW ID based filtering
	*/
	if (s->gen.hwid && current_td->detected_hw &&
	    list_contains((const char *)s->gen.hwid, 
			 (const char *) current_td->detected_hw, ",") == 0) {
		LOG_MSG (LOG_INFO, "Test set %s is filtered based on HW ID",
			 s->gen.name);
		goto skip_all;
	}

	/*
	** Check that the set is supposed to be executed in the current env
	*/
	s->environment = xmlCharStrdup (opts.environment);
	if (!xmlListSearch (s->environments, opts.environment)) {
		LOG_MSG (LOG_INFO, "Test set %s not run on "
			 "environment: %s", 
			 s->gen.name, opts.environment);
		goto skip_all;
	}
	current_set = s;
	LOG_MSG (LOG_INFO, "Test set: %s", s->gen.name);
	write_pre_set (s);

	if (xmlListSize (s->pre_steps) > 0) {
		cur_case_name = (xmlChar *)"pre_steps";
		cur_step_num = 0;
		memset (&dummy, 0x0, sizeof (td_case));
		dummy.case_res = CASE_PASS;
		dummy.dummy = 1;
		LOG_MSG (LOG_INFO, "Executing pre steps");
		xmlListWalk (s->pre_steps, prepost_steps_execute, &dummy);
		if (dummy.case_res != CASE_PASS) {
			LOG_MSG (LOG_INFO, "Pre steps failed. "
				 "Test set %s aborted.", s->gen.name); 
			xmlListWalk (s->cases, case_result_fail, 
				     global_failure ? global_failure :
				     "pre_steps failed");
			goto short_circuit;
		}
	}
	
	xmlListWalk (s->cases, process_case, s);

	if (opts.resume_testrun != RESUME_TESTRUN_ACTION_NONE) {
		wait_for_resume_execution();
	}

	if (xmlListSize (s->post_steps) > 0) {
		LOG_MSG (LOG_INFO, "Executing post steps");
		cur_case_name = (xmlChar *)"post_steps";
		cur_step_num = 0;
		memset (&dummy, 0x0, sizeof (td_case));
		dummy.case_res = CASE_PASS;
		dummy.dummy = 1;
		xmlListWalk (s->post_steps, prepost_steps_execute, &dummy);
		if (dummy.case_res == CASE_FAIL)
			LOG_MSG (LOG_INFO, 
				 "Post steps failed for %s.", s->gen.name);
	}
	xmlListWalk (s->gets, process_get, NULL);

	if (opts.resume_testrun == RESUME_TESTRUN_ACTION_EXIT) {
		restore_bail_out_after_resume_execution();
	}

 short_circuit:
	write_post_set (s);
	if (xmlListSize (s->pre_steps) > 0) {
		steps = xmlLinkGetData(xmlListFront(s->pre_steps));
		xmlListWalk (steps->steps, step_post_process, &dummy);
	}
	if (xmlListSize (s->post_steps) > 0) {
		steps = xmlLinkGetData(xmlListFront(s->post_steps));
		xmlListWalk (steps->steps, step_post_process, &dummy);
	}
	xml_end_element();
 skip_all:
	td_set_delete (s);
	return;
}
/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** Walks through the whole test definition and executes all suites, 
 *  sets, cases and steps.
 */
void td_process () {
	int retval;
	td_parser_callbacks cbs;

        memset (&cbs, 0x0, sizeof(td_parser_callbacks));
	/*
	** Set callbacks for parser
	*/
	cbs.test_td = process_td;
	cbs.test_td_end = end_td;
	cbs.test_hwiddetect = process_hwiddetect;
	cbs.test_suite = process_suite;
	cbs.test_suite_end = end_suite;
	cbs.test_set = process_set;

	retval = td_register_callbacks (&cbs);
	
	/*
	** Call td_next_node untill error occurs or the end of data is reached
	*/
	LOG_MSG (LOG_INFO, "Starting to run tests...");

	while (td_next_node() == 0);

	LOG_MSG (LOG_INFO, "Finished running tests.");
	LOG_MSG (LOG_INFO, "Executed %d cases. Passed %d Failed %d",
		 casecount, passcount, failcount);
	return; 
}	
/* ------------------------------------------------------------------------- */
/** Name of the currently executed set
 * @return name of the set or NULL
 */
const char *current_set_name ()
{
	if (current_set)
		return (char *)current_set->gen.name;
	return "";
}
/* ------------------------------------------------------------------------- */
/** Name of the currently executed case (can be also "pre/post_steps"
 * @return case name
 */
const char *current_case_name ()
{
	return (char *)cur_case_name;
}
/* ------------------------------------------------------------------------- */
/** Number of the step currently executed
 *  return 0 if no step is executed, > 0 otherwise
 */
int current_step_num ()
{
	return cur_step_num;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
