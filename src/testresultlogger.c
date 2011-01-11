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
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <libxml/xmlwriter.h>
#include "testresultlogger.h"
#include "log.h"

/* ------------------------------------------------------------------------- */
/* EXTERNAL DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* EXTERNAL GLOBAL VARIABLES */
/* None */

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
LOCAL xmlTextWriterPtr writer;
LOCAL FILE *ofile;
/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
struct  
{
    int (*write_td_start)  (td_td *);
    int (*write_td_end)  (td_td *);
    int (*write_pre_suite) (td_suite *);
    int (*write_post_suite) (td_suite *);
    int (*write_pre_set) (td_set *);
    int (*write_post_set) (td_set *);
} out_cbs;
/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_td_start (td_td *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_td_end (td_td *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_pre_suite (td_suite *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_post_suite (td_suite *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_pre_set (td_set *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_general_attributes (td_gen_attribs *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_step (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_pre_post_step (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_case (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_file_data (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_post_set (td_set *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_measurement (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int txt_write_td_start (td_td *);
/* ------------------------------------------------------------------------- */
LOCAL int txt_write_td_end (td_td *);
/* ------------------------------------------------------------------------- */
LOCAL int txt_write_pre_suite (td_suite *);
/* ------------------------------------------------------------------------- */
LOCAL int txt_write_post_suite (td_suite *);
/* ------------------------------------------------------------------------- */
LOCAL int txt_write_pre_set (td_set *);
/* ------------------------------------------------------------------------- */
LOCAL int txt_write_post_set (td_set *);
/* ------------------------------------------------------------------------- */
LOCAL int txt_write_case (const void *, const void *);
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/************************** xml output ***************************************/
/* ------------------------------------------------------------------------- */
/** Write the test definition attributes to xml results 
 * @param td test definition data
 * @return 0 on success, 1 on error
 */
LOCAL int xml_write_td_start (td_td *td)
{
	
	if (!td->version || xmlTextWriterWriteAttribute (writer, 
							 BAD_CAST "version", 
							 td->version) < 0)
		goto err_out;
	
	return 0; 
 err_out:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Write the test definition top level elements
 * @param td test definition data
 * @return 0 on success, 1 on error
 */
LOCAL int xml_write_td_end (td_td *td)
{
	if (td->hw_detector)
		if (xmlTextWriterWriteElement (writer, 
					       BAD_CAST "hwiddetect", 
					       td->hw_detector) < 0)
			goto err_out;
	
	if (td->description)
		if (xmlTextWriterWriteElement	(writer, 
						 BAD_CAST "description", 
						 td->description) < 0)
			goto err_out;

	xmlTextWriterFlush (writer);
	
	while (!xml_end_element());
	return  0;
 err_out:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Write suite start xml tag
 * @param suite suite data
 * @return 0 on success, 1 on error
 */
LOCAL int xml_write_pre_suite (td_suite *suite)
{
	
	if (xmlTextWriterStartElement (writer, BAD_CAST "suite") < 0)
		goto err_out;
	
	if (xml_write_general_attributes (&suite->gen))
		goto err_out;


	return 0;
err_out:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Write suite end
 * @param suite suite data
 * @return 0 on success, 1 on error
 */
LOCAL int xml_write_post_suite (td_suite *suite)
{
	if (suite->description)
		if (xmlTextWriterWriteElement	(writer, 
						 BAD_CAST "description", 
						 suite->description) < 0)
			goto err_out;

	return xml_end_element();
err_out:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Write pre-set xml tag
 * @param set set data
 * @return 0 on success, 1 on error
 */
LOCAL int xml_write_pre_set (td_set *set)
{
	if (xmlTextWriterStartElement (writer, BAD_CAST "set") < 0)
		goto err_out;
	
	if (xml_write_general_attributes (&set->gen))
		goto err_out;


	if (set->environment)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "environment", 
						 set->environment) < 0)
			goto err_out;
	return 0;
 err_out:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Write the values of general attributes
 * @param gen 
 * @return 0 on success, 1 on error
 */
LOCAL int xml_write_general_attributes (td_gen_attribs *gen)
{
	if (gen->name)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "name", 
						 gen->name) < 0)
			goto err_out;

	if (gen->description)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "description", 
						 gen->description) < 0)
			goto err_out;

	if (gen->requirement)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "requirement", 
						 gen->requirement) < 0)
			goto err_out;
	
	if (xmlTextWriterWriteFormatAttribute (writer,
					       BAD_CAST "timeout",
					       "%lu",
					       gen->timeout) < 0)
		goto err_out;

	if (gen->type)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "type", 
						 gen->type) < 0)
			goto err_out;
	

	if (gen->level)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "level", 
						 gen->level) < 0)
			goto err_out;
		
	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "manual", 
					 BAD_CAST (gen->manual ? "true" :
						   "false")) < 0)
		goto err_out;

	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "insignificant", 
					 BAD_CAST (gen->insignificant ? 
						   "true" :
						   "false")) < 0)
		goto err_out;


	if (gen->domain) 
	    if (xmlTextWriterWriteAttribute (writer,  
					     BAD_CAST 
					     "domain", 
					     gen->domain) < 0)
		goto err_out;

	if (gen->feature)
	    if (xmlTextWriterWriteAttribute (writer, 
					     BAD_CAST "feature", 
					     gen->feature) < 0)
		goto err_out;

	if (gen->component)
	    if (xmlTextWriterWriteAttribute (writer, 
					     BAD_CAST "component", 
					     gen->component) < 0)
		goto err_out;

	if (gen->hwid)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "hwid", 
						 gen->hwid) < 0)
			goto err_out;
	
	return 0;
 err_out:
	return 1;
}

/* ------------------------------------------------------------------------- */
/** Write step result xml
 * @param data step data 
 * @param user not used
 * @return 1 on success, 0 on error if the step is failed
 */
LOCAL int xml_write_step (const void *data, const void *user)
{
	td_step *step = (td_step *)data;
	struct tm *tm;

	if (xmlTextWriterStartElement (writer, BAD_CAST "step") < 0)
		goto err_out;

	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "manual", 
					 BAD_CAST (step->manual ? "true" :
						   "false")) < 0)
		goto err_out;

	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "command", 
					 step->step) < 0)
		goto err_out;
	
	if (step->has_result == 0) {
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "result", 
						 BAD_CAST "N/A") < 0)
			goto err_out;


	} else if (xmlTextWriterWriteAttribute (writer, 
						BAD_CAST "result", 
						step->expected_result == 
						step->return_code ? 
						BAD_CAST "PASS" :
						BAD_CAST "FAIL") < 0)
		goto err_out;
	
	if (step->failure_info) {
		if (strlen ((char *)step->failure_info) >= FAILURE_INFO_MAX)
			step->failure_info[FAILURE_INFO_MAX - 1] = '\0';
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "failure_info", 
						 step->failure_info) < 0)
			goto err_out;
	}

	if (xmlTextWriterWriteFormatElement (writer,
					     BAD_CAST "expected_result",
					     "%d", step->expected_result) < 0)
		goto err_out;
	
	if (xmlTextWriterWriteFormatElement (writer,
					     BAD_CAST "return_code",
					     "%d", step->return_code) < 0)
		goto err_out;
	
	tm =  localtime (&step->start);
	if (xmlTextWriterWriteFormatElement (writer,
					     BAD_CAST "start",
					     "%02d-%02d-%02d %02d:%02d:%02d", 
					     tm->tm_year+1900,
					     tm->tm_mon+1,
					     tm->tm_mday,
					     tm->tm_hour,
					     tm->tm_min,
					     tm->tm_sec) < 0)
		goto err_out;

	tm =  localtime (&step->end);
	if (xmlTextWriterWriteFormatElement (writer,
					     BAD_CAST "end",
					     "%02d-%02d-%02d %02d:%02d:%02d", 
					     tm->tm_year+1900,
					     tm->tm_mon+1,
					     tm->tm_mday,
					     tm->tm_hour,
					     tm->tm_min,
					     tm->tm_sec) < 0)
		goto err_out;

	if (xmlTextWriterWriteFormatElement (writer,
					     BAD_CAST "stdout",
					     "%s", 
					     step->stdout_ ? step->stdout_ :
					     BAD_CAST "") < 0)
		goto err_out;

	if (xmlTextWriterWriteFormatElement (writer,
					     BAD_CAST "stderr",
					     "%s", 
					     step->stderr_ ? step->stderr_ :
					     BAD_CAST "") < 0)
		goto err_out;


	return !xml_end_element();
	
err_out:
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Write pre or post step result xml
 * @param data step data 
 * @param user not used
 * @return 1 on success, 0 on error if the step is failed
 */
LOCAL int xml_write_pre_post_step (const void *data, const void *user)
{
	td_step *step = (td_step *)data;
	struct tm *tm;

	if (xmlTextWriterStartElement (writer, BAD_CAST "step") < 0)
		goto err_out;

	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "command", 
					 step->step) < 0)
		goto err_out;
	
	if (step->has_result == 0) {
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "result", 
						 BAD_CAST "N/A") < 0)
			goto err_out;


	} else if (xmlTextWriterWriteAttribute (writer, 
						BAD_CAST "result", 
						step->expected_result == 
						step->return_code ? 
						BAD_CAST "PASS" :
						BAD_CAST "FAIL") < 0)
		goto err_out;
	
	if (step->failure_info) {
		if (strlen ((char *)step->failure_info) >= FAILURE_INFO_MAX)
			step->failure_info[FAILURE_INFO_MAX - 1] = '\0';
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "failure_info", 
						 step->failure_info) < 0)
			goto err_out;
	}

	if (xmlTextWriterWriteFormatElement (writer,
					     BAD_CAST "expected_result",
					     "%d", step->expected_result) < 0)
		goto err_out;
	
	if (xmlTextWriterWriteFormatElement (writer,
					     BAD_CAST "return_code",
					     "%d", step->return_code) < 0)
		goto err_out;
	
	tm =  localtime (&step->start);
	if (xmlTextWriterWriteFormatElement (writer,
					     BAD_CAST "start",
					     "%02d-%02d-%02d %02d:%02d:%02d", 
					     tm->tm_year+1900,
					     tm->tm_mon+1,
					     tm->tm_mday,
					     tm->tm_hour,
					     tm->tm_min,
					     tm->tm_sec) < 0)
		goto err_out;

	tm =  localtime (&step->end);
	if (xmlTextWriterWriteFormatElement (writer,
					     BAD_CAST "end",
					     "%02d-%02d-%02d %02d:%02d:%02d", 
					     tm->tm_year+1900,
					     tm->tm_mon+1,
					     tm->tm_mday,
					     tm->tm_hour,
					     tm->tm_min,
					     tm->tm_sec) < 0)
		goto err_out;

	if (xmlTextWriterWriteFormatElement (writer,
					     BAD_CAST "stdout",
					     "%s", 
					     step->stdout_ ?
					     step->stdout_ : BAD_CAST "") < 0)
		goto err_out;

	if (xmlTextWriterWriteFormatElement (writer,
					     BAD_CAST "stderr",
					     "%s", 
					     step->stderr_ ?
					     step->stderr_ : BAD_CAST "") < 0)
		goto err_out;


	
	return !xml_end_element();
err_out:
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Write case result xml
 * @param data case data 
 * @param user not used
 * @return 1 on success, 0 on error
 */
LOCAL int xml_write_case (const void *data, const void *user)
{
	td_case *c = (td_case *)data;

	if (c->filtered)
		return 1;

	if (xmlTextWriterStartElement (writer, BAD_CAST "case") < 0)
		goto err_out;

	if (xml_write_general_attributes (&c->gen))
		goto err_out;

	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "result", 
					 BAD_CAST (case_result_str
						   (c->case_res))) < 0)
		
		goto err_out;

	if (c->failure_info) {
		if (strlen ((char *)c->failure_info) >= FAILURE_INFO_MAX)
			c->failure_info[FAILURE_INFO_MAX - 1] = '\0';
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "failure_info", 
						 c->failure_info) < 0)
			goto err_out;
	}

	if (c->subfeature)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "subfeature", 
						 c->subfeature) < 0)
			goto err_out;
	if (c->bugzilla_id)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "bugzilla_id", 
						 c->bugzilla_id) < 0)
			goto err_out;
	if (c->state)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "state", 
						 c->state) < 0)
			goto err_out;
	
	if (c->description)
		if (xmlTextWriterWriteElement	(writer, 
						 BAD_CAST "description", 
						 c->description) < 0)
			goto err_out;

	if (c->gen.manual && c->comment)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "comment", 
						 c->comment) < 0)
			goto err_out;

	xmlListWalk (c->steps, xml_write_step, NULL);
	xmlListWalk (c->measurements, xml_write_measurement, NULL);

	return !xml_end_element ();

err_out:
	LOG_MSG (LOG_ERR, "%s:%s: error\n", PROGNAME, __FUNCTION__);
	
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Write get/file data to results
 * @param data td_dile
 * @param user not used
 * @return 1 on success, 0 on error
 */
LOCAL int xml_write_file_data (const void *data, const void *user)
{
	td_file *f = (td_file *)data;

	if (xmlTextWriterStartElement (writer, BAD_CAST "file") < 0)
		goto err_out;
	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "delete_after", 
					 f->delete_after ? BAD_CAST "true"
					 : BAD_CAST "false") < 0)
		goto err_out;
	if (xmlTextWriterWriteString (writer, f->filename) < 0)
		goto err_out;
	return !xml_end_element();
 err_out:
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Write set start tag and cases result xml
 * @param set set data
 * @return 0 on success, 1 on error
 */
LOCAL int xml_write_post_set (td_set *set)
{
	td_steps *steps;

	if (set->description)
		if (xmlTextWriterWriteElement	(writer, 
						 BAD_CAST "description", 
						 set->description) < 0)
			goto err_out;

	if (xmlListSize (set->pre_steps) > 0) {
		steps = xmlLinkGetData (xmlListFront (set->pre_steps));

		if (xmlTextWriterStartElement (writer, 
					       BAD_CAST "pre_steps") < 0)
			goto err_out;

		if (xmlTextWriterWriteFormatAttribute (writer,
						       BAD_CAST "timeout",
						       "%lu",
						       steps->timeout) < 0)
			goto err_out;

		xmlListWalk (steps->steps, xml_write_pre_post_step, 
			     NULL);
		xml_end_element ();
	}

	xmlListWalk (set->cases, xml_write_case, NULL);

	if (xmlListSize (set->post_steps) > 0) {
		steps = xmlLinkGetData (xmlListFront (set->post_steps));

		if (xmlTextWriterStartElement (writer, 
					       BAD_CAST "post_steps") < 0)
			goto err_out;
		if (xmlTextWriterWriteFormatAttribute (writer,
						       BAD_CAST "timeout",
						       "%lu",
						       steps->timeout) < 0)
			goto err_out;

		xmlListWalk (steps->steps, xml_write_pre_post_step, NULL);
		xml_end_element ();
	}

	if (xmlListSize (set->gets) > 0) {
		if (xmlTextWriterStartElement (writer, 
					       BAD_CAST "get") < 0)
			goto err_out;

		xmlListWalk (set->gets, xml_write_file_data, NULL);
		xml_end_element();
	}
	
	return 0;

 err_out:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Write measurement data
 * @param data measurement data 
 * @param user not used
 * @return 1 on success, 0 on error
 */
LOCAL int xml_write_measurement (const void *data, const void *user)
{
	td_measurement *meas = (td_measurement *)data;

	if (xmlTextWriterStartElement (writer, BAD_CAST "measurement") < 0)
		goto err_out;
	if (xmlTextWriterWriteAttribute	(writer, BAD_CAST "name", 
					 meas->name) < 0)
		goto err_out;
	if (xmlTextWriterWriteFormatAttribute	(writer, BAD_CAST "value", 
						 "%f", meas->value) <0)
		goto err_out;
	if (xmlTextWriterWriteAttribute	(writer, BAD_CAST "unit", 
					 meas->unit) < 0)
		goto err_out;

	if (!meas->target_specified)
		goto ok_out;

	if (xmlTextWriterWriteFormatAttribute	(writer, BAD_CAST "target", 
						 "%f", meas->target) <0)
		goto err_out;
	if (xmlTextWriterWriteFormatAttribute	(writer, BAD_CAST "failure", 
						 "%f", meas->failure) <0)
		goto err_out;
 ok_out:
	if (xmlTextWriterEndElement (writer) < 0)
		goto err_out;
	return 1;
 err_out:
	LOG_MSG (LOG_ERR, "%s:%s: error\n", PROGNAME, __FUNCTION__);
	return 0;
}
/* ------------------------------------------------------------------------- */
/************************* text output ***************************************/
/* ------------------------------------------------------------------------- */
/** Write step result to text file
 * @param data step data 
 * @param user not used
 * @return 1 on success, 0 on error
 */
LOCAL int txt_write_step (const void *data, const void *user)
{
	td_step *step = (td_step *)data;
	struct tm *tm;
	fprintf (ofile, "----------------------------------"
		 "----------------------------------\n");
	fprintf (ofile, "      Test step       : %s\n", step->step);
	tm =  localtime (&step->start);

	fprintf (ofile, "        start         : %02d-%02d-%02d"
		 " %02d:%02d:%02d\n",
		 tm->tm_year+1900,
		 tm->tm_mon+1,
		 tm->tm_mday,
		 tm->tm_hour,
		 tm->tm_min,
		 tm->tm_sec);
	tm =  localtime (&step->end);
	fprintf (ofile, "        end           : %02d-%02d-%02d"
		 " %02d:%02d:%02d\n",
		 tm->tm_year+1900,
		 tm->tm_mon+1,
		 tm->tm_mday,
		 tm->tm_hour,
		 tm->tm_min,
		 tm->tm_sec);
	fprintf (ofile, "        expected code : %d\n", step->expected_result);
	fprintf (ofile, "        return code   : %d\n", step->return_code);
	fprintf (ofile, "        result        : %s %s\n",
		 (step->return_code == step->expected_result ? "PASS" : "FAIL"),
		 (step->failure_info ? (char *)step->failure_info : " "));
	fprintf (ofile, "        stdout        : %s\n",
		 step->stdout_ ? (char *)step->stdout_ : " ");
	fprintf (ofile, "        stderr        : %s\n",
		 step->stderr_ ? (char *)step->stderr_ : " ");
	fflush (ofile);

	return 1;
}
/* ------------------------------------------------------------------------- */
/** Write case result to text file
 * @param data case data 
 * @param user not used
 * @return 1 on success, 0 on error
 */
LOCAL int txt_write_case (const void *data, const void *user)
{
	td_case *c = (td_case *)data;
	
	if (c->filtered)
		return 1;

	fprintf (ofile, "----------------------------------"
		 "----------------------------------\n");
        fprintf (ofile, "    Test case name  : %s\n", c->gen.name);
        fprintf (ofile, "      description   : %s\n", c->gen.description ? 
		 (char *)c->gen.description : "");
	fprintf (ofile, "      manual        : %s\n", c->gen.manual ?
		 "true" : "false");
	fprintf (ofile, "      result        : %s", 
		 case_result_str(c->case_res));
	if (c->failure_info) {
		fprintf (ofile, " (%s)", c->failure_info);
	}
	fprintf (ofile, "\n");

	if (c->gen.requirement)
	    fprintf (ofile, "      requirement   : %s\n", c->gen.requirement);
	if (c->subfeature)
	    fprintf (ofile, "      subfeature    : %s\n", c->subfeature);
	if (c->bugzilla_id)
	    fprintf (ofile, "      bugzilla_id   : %s\n", c->bugzilla_id);
	if (c->gen.type)
	    fprintf (ofile, "      type          : %s\n", c->gen.type);
	if (c->gen.level)
	    fprintf (ofile, "      level         : %s\n", c->gen.level);
	
        fprintf (ofile, "      insignificant : %s\n", c->gen.insignificant ?
		 "true" : "false");

	if (c->gen.manual && c->comment)
		fprintf (ofile, "      comment       : %s\n", c->comment);
		
	fflush (ofile);
	
	xmlListWalk (c->steps, txt_write_step, NULL);


	return 1;
}
/* ------------------------------------------------------------------------- */
/**  Write the test definition attributes to xml results 
 * @param td test definition data
 * @return 0 on success, 1 on error
 */
LOCAL int txt_write_td_start (td_td *td)
{

	return 0;
}
/* ------------------------------------------------------------------------- */
/**  Write the test definition attributes to xml results 
 * @param td test definition data
 * @return 0 on success, 1 on error
 */
LOCAL int txt_write_td_end (td_td *td)
{
	fprintf (ofile, "----------------------------------"
		 "----------------------------------\n");
	fprintf (ofile, "End of test results.\n");

	return 0;
}
/* ------------------------------------------------------------------------- */
/** Write suite start txt tag
 * @param suite suite data
 * @return 0 on success, 1 on error
 */
LOCAL int txt_write_pre_suite (td_suite *suite)
{
	fprintf (ofile, "----------------------------------"
		 "----------------------------------\n");

        fprintf (ofile, "Test suite name : %s\n", suite->gen.name);
	fprintf (ofile, "  description   : %s\n", suite->gen.description ? 
		 (char *)suite->gen.description : " ");
	if (suite->gen.domain)
		fprintf (ofile, "  domain        : %s\n", suite->gen.domain);
	
	fflush (ofile);
	
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Write post suite to text file - does not do anything at the moment
 * @return 0 on always
 */
LOCAL int txt_write_post_suite (td_suite *suite)
{
	return 0;

}
/* ------------------------------------------------------------------------- */
/** Write pre set information to text file
 * @param set set data
 * @return 0 on always
 */
LOCAL int txt_write_pre_set (td_set *set)
{
	fprintf (ofile, "----------------------------------"
		 "----------------------------------\n");

	fprintf (ofile, "  Test set name   : %s\n", set->gen.name);
	fprintf (ofile, "    description   : %s\n", set->gen.description ? 
		 (char *)set->gen.description : "");
	if (set->gen.feature)
		fprintf (ofile, "    feature       : %s\n", set->gen.feature);
	
	fprintf (ofile, "    environment   : %s\n", set->environment ? 
		 (char *)set->environment : "");

	fflush (ofile);
	return 0;

}
/* ------------------------------------------------------------------------- */
/** Write post set information to text file - loop through test cases
 * @param set set data
 * @return 0 on always
 */
LOCAL int txt_write_post_set (td_set *set)
{
	
	xmlListWalk (set->cases, txt_write_case, NULL);
	fflush (ofile);
	
	return 0;
}
/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** Initialize result logger according to user options.
 *  @param opts commandline options 
 *  @param hwinfo hardware information
 *  @return 0 on success
 */
int init_result_logger (testrunner_lite_options *opts, hw_info *hwinfo)
{

    switch (opts->output_type) {
    case OUTPUT_TYPE_XML:
	    /*
	     * Instantiate writer 
	     */
	    writer = xmlNewTextWriterFilename(opts->output_filename, 0);
	    if (!writer)  {
		    LOG_MSG (LOG_ERR, "%s:%s:failed to create writer for %s\n",
			     PROGNAME, __FUNCTION__, opts->output_filename);
		    return 1;
	    }
	    xmlTextWriterSetIndent (writer, 1);
	    if (xmlTextWriterStartDocument(writer, 
					   "1.0", 
					   "UTF-8", 
					   NULL) < 0) {
		    LOG_MSG (LOG_ERR, "%s:%s:failed to write document start\n",
			     PROGNAME, __FUNCTION__);
		    return 1;
	    }
		    
	    if (xmlTextWriterStartElement (writer, BAD_CAST "testresults") 
		< 0) {
		    LOG_MSG (LOG_ERR, "%s:%s:failed to write "
			     "testsresults tag\n",
			     PROGNAME, __FUNCTION__);
		    return 1;
	    }

	    if (xmlTextWriterWriteAttribute (writer, 
					     BAD_CAST "environment", 
					     BAD_CAST (opts->environment ?
						       opts->environment :
						       "unknown")) < 0)
		    return 1;

	    
	    if (xmlTextWriterWriteAttribute (writer, 
					     BAD_CAST "hwproduct", 
					     (hwinfo->product ?
					      hwinfo->product :
					      BAD_CAST "unknown")) < 0)
		    return 1;


	    
	    if (xmlTextWriterWriteAttribute (writer, 
					     BAD_CAST "hwbuild", 
					     (hwinfo->hw_build ?
					      hwinfo->hw_build :
					      BAD_CAST "unknown")) < 0)
		    return 1;


	    /*
	     * Set callbacks
	     */
	    out_cbs.write_td_start = xml_write_td_start;
	    out_cbs.write_td_end = xml_write_td_end;
	    out_cbs.write_pre_suite = xml_write_pre_suite;
	    out_cbs.write_post_suite = xml_write_post_suite;
	    out_cbs.write_pre_set = xml_write_pre_set;
	    out_cbs.write_post_set = xml_write_post_set;
	     
	    
	    break;
	    
    case OUTPUT_TYPE_TXT:
	    /*
	     * Open results file
	     */
	    ofile = fopen (opts->output_filename, "w+");
	    if (!ofile)  {
		    LOG_MSG (LOG_ERR, "%s:%s:failed to open file %s %s\n",
			     PROGNAME, __FUNCTION__, opts->output_filename,
			     strerror(errno));
		    return 1;
	    }
	    fprintf (ofile,"Test results:\n");
	    fprintf (ofile, "  environment : %s\n", opts->environment);

	    fprintf (ofile, "  hwproduct   : %s\n", 
		     (char *)(hwinfo->product ? hwinfo->product : 
			      (unsigned char *)"unknown"));
	    
	    fprintf (ofile, "  hwbuild     : %s\n", 
		     (char *)(hwinfo->hw_build ? hwinfo->hw_build :
			      (unsigned char *)"unknown"));
	    

	    /*
	     * Set callbacks
	     */
	    out_cbs.write_td_start = txt_write_td_start;
	    out_cbs.write_td_end = txt_write_td_end;
	    out_cbs.write_pre_suite = txt_write_pre_suite;
	    out_cbs.write_post_suite = txt_write_post_suite;
	    out_cbs.write_pre_set = txt_write_pre_set;
	    out_cbs.write_post_set = txt_write_post_set;
	     
	    break;

    default:
	    LOG_MSG (LOG_ERR, "%s:%s:invalid output type %d\n",
		     PROGNAME, __FUNCTION__, opts->output_type);
	    return 1;
    }
    
    return 0;
}
/* ------------------------------------------------------------------------- */
/** Call td_start callback
 *  @param td td data
 *  @return 0 on success
 */
int write_td_start (td_td *td)
{
	return out_cbs.write_td_start (td);
}
/* ------------------------------------------------------------------------- */
/** Call td_end callback
 *  @param td td data
 *  @return 0 on success
 */
int write_td_end (td_td *td)
{
	return out_cbs.write_td_end (td);
}
/* ------------------------------------------------------------------------- */
/** Call pre_suite callback
 *  @param suite suite data
 *  @return 0 on success
 */
int write_pre_suite (td_suite *suite)
{
	return out_cbs.write_pre_suite (suite);
}
/* ------------------------------------------------------------------------- */
/** Call post_suite callback
 *  @param suite suite data
 *  @return 0 on success
 */
int write_post_suite (td_suite *suite)
{
	
	return out_cbs.write_post_suite (suite);
}
/* ------------------------------------------------------------------------- */
/** Call pre_set callback
 *  @param set set data
 *  @return 0 on success
 */
int write_pre_set (td_set *set)
{
	return out_cbs.write_pre_set (set);
}
/* ------------------------------------------------------------------------- */
/** Call post_set callback
 *  @param set set data
 *  @return 0 on success
 */
int write_post_set (td_set *set)
{
	
	return out_cbs.write_post_set (set);
}
/* ------------------------------------------------------------------------- */
/** Write end element tag
 * @return 0 on success, 1 on error.
 */
int xml_end_element ()
{

	if (xmlTextWriterFullEndElement (writer) < 0)
		goto err_out;
	return 0;
err_out:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Close the result logger */

void close_result_logger (void)
{
	if (writer) {
		xmlTextWriterFlush (writer);
		xmlFreeTextWriter (writer);
		writer = NULL;
	} else if (ofile) {
		fflush (ofile);
		fclose (ofile);
		ofile = NULL;
	} else {
		LOG_MSG (LOG_ERR, "%s:%s: Result logger not open?\n",
			 PROGNAME, __FUNCTION__);
	}

	return;
}
/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
