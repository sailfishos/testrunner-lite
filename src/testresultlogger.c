/* * This file is part of testrunner-lite *
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
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <libxml/xmlwriter.h>
#include "testresultlogger.h"

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
    int (*write_pre_suite_tag) (td_suite *);
    int (*write_post_suite_tag) (void);
    int (*write_pre_set_tag) (td_set *);
    int (*write_post_set_tag) (td_set *);
} out_cbs;
/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_pre_suite_tag (td_suite *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_pre_set_tag (td_set *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_step (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_case (const void *, const void *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_post_set_tag (td_set *);
/* ------------------------------------------------------------------------- */
LOCAL int xml_end_element ();
/* ------------------------------------------------------------------------- */
LOCAL int txt_write_pre_suite_tag (td_suite *suite);
/* ------------------------------------------------------------------------- */
LOCAL int txt_write_post_suite_tag ();
/* ------------------------------------------------------------------------- */
LOCAL int txt_write_pre_set_tag (td_set *set);
/* ------------------------------------------------------------------------- */
LOCAL int txt_write_post_set_tag (td_set *set);
/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/************************** xml output ***************************************/
/* ------------------------------------------------------------------------- */
/** Write suite start xml tag
 * @param suite suite data
 * @return 0 on success, 1 on error
 */
LOCAL int xml_write_pre_suite_tag (td_suite *suite)
{
	
	if (xmlTextWriterStartElement (writer, BAD_CAST "suite") < 0)
		goto err_out;
	
	if (xmlTextWriterWriteAttribute (writer,  BAD_CAST "name", 
					 suite->gen.name) < 0)
		goto err_out;
	return 0;
err_out:
	return 1;
}
/* ------------------------------------------------------------------------- */
/** Write pre-set xml tag
 * @param set set data
 * @return 0 on success, 1 on error
 */
LOCAL int xml_write_pre_set_tag (td_set *set)
{
	if (xmlTextWriterStartElement (writer, BAD_CAST "set") < 0)
		goto err_out;
	
	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "name", 
					 set->gen.name) < 0)
		goto err_out;
	
	if (set->gen.description)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "description", 
						 set->gen.description) < 0)
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
/** Write step result xml
 * @param data step data 
 * @param user not used
 * @return 1 on success, 0 on error
 */
LOCAL int xml_write_step (const void *data, const void *user)
{
	td_step *step = (td_step *)data;
	struct tm *tm;

	if (xmlTextWriterStartElement (writer, BAD_CAST "step") < 0)
		goto err_out;

	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "command", 
					 step->step) < 0)
		goto err_out;
	
	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "result", 
					 step->expected_result == 
					 step->return_code ? 
					 BAD_CAST "PASS" :
					 BAD_CAST "FAIL") < 0)
		goto err_out;

	if (step->failure_info) {
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "result", 
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

	if (step->stdout_)
		if (xmlTextWriterWriteFormatElement (writer,
						     BAD_CAST "stdout",
						     "%s", 
						     step->stdout_) < 0)
			goto err_out;

	if (step->stderr_)
		if (xmlTextWriterWriteFormatElement (writer,
						     BAD_CAST "stderr",
						     "%s", 
						     step->stderr_) < 0)
			goto err_out;


	xml_end_element();
	

	return 1;
	
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

	if (xmlTextWriterStartElement (writer, BAD_CAST "case") < 0)
		goto err_out;

	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "name", 
					 c->gen.name) < 0)
		goto err_out;
	
	if (c->gen.description)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "description", 
						 c->gen.description) < 0)
		goto err_out;

	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "manual", 
					 BAD_CAST (c->gen.manual ? "true" :
						   "false")) < 0)
		goto err_out;

	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "insignificant", 
					 BAD_CAST (c->gen.insignificant ? 
						   "true" :
						   "false")) < 0)
		goto err_out;


	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "result", 
					 c->passed ?
					 BAD_CAST "FAIL" : BAD_CAST "PASS") < 0)
	    
		goto err_out;

	if (c->subfeature)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "subfeature", 
						 c->subfeature) < 0)
		goto err_out;

	if (c->gen.requirement)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "requirement", 
						 c->gen.requirement) < 0)
		goto err_out;

	if (c->gen.level)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "level", 
						 c->gen.level) < 0)
		goto err_out;

	xmlListWalk (c->steps, xml_write_step, NULL);

	xml_end_element ();

	return 1;

err_out:
	fprintf (stderr, "%s:%s: error\n", PROGNAME, __FUNCTION__);
	
	return 0;
}
/* ------------------------------------------------------------------------- */
/** Write set start tag and cases result xml
 * @param set set data
 * @return 0 on success, 1 on error
 */
LOCAL int xml_write_post_set_tag (td_set *set)
{
	
	
	xmlListWalk (set->cases, xml_write_case, NULL);
	
	return xml_end_element();
 }
/* ------------------------------------------------------------------------- */
/** Write end element tag
 * @return 0 on success, 1 on error.
 */
LOCAL int xml_end_element ()
{
	
	if (xmlTextWriterFullEndElement (writer) < 0)
		goto err_out;
	return 0;
err_out:
	return 1;
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
	
	return 0;
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
	
	fprintf (ofile, "----------------------------------"
		 "----------------------------------\n");
        fprintf (ofile, "    Test case name  : %s\n", c->gen.name);
        fprintf (ofile, "      description   : %s\n", c->gen.description ? 
		 (char *)c->gen.description : "");
	/*
	self._writeline("      manual        : %s\n",)
        self._writeline("      requirement   : %s\n")
        self._writeline("      subfeature    : %s\n")
        self._writeline("      type          : %s\n")
        self._writeline("      level         : %s\n")
        self._writeline("      insignificant : %s\n")
	*/
	xmlListWalk (c->steps, txt_write_step, NULL);

	return 0;
}
/** Write suite start txt tag
 * @param suite suite data
 * @return 0 on success, 1 on error
 */
LOCAL int txt_write_pre_suite_tag (td_suite *suite)
{
	fprintf (ofile, "----------------------------------"
		 "----------------------------------\n");

        fprintf (ofile, "Test suite name : %s\n", suite->gen.name);
	fprintf (ofile, "  description   : %s\n", suite->gen.description ? 
		 (char *)suite->gen.description : " ");
	return 0;
}

LOCAL int txt_write_post_suite_tag ()
{
	return 0;

}

LOCAL int txt_write_pre_set_tag (td_set *set)
{
	fprintf (ofile, "----------------------------------"
		 "----------------------------------\n");

	fprintf (ofile, "  Test set name   : %s\n", set->gen.name);
	fprintf (ofile,"    description   : %s\n", set->gen.description ? 
		 (char *)set->gen.description : "");

	return 0;

}

LOCAL int txt_write_post_set_tag (td_set *set)
{
	
	xmlListWalk (set->cases, txt_write_case, NULL);
	
	return 0;
}
/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */
/** Initialize result logger according to user options.
 *  @param opts commandline options 
 *  @return 0 on success
 */
int init_result_logger (testrunner_lite_options *opts)
{

    switch (opts->output_type) {
    case OUTPUT_TYPE_XML:
	    /*
	     * Instantiate writer 
	     */
	    writer = xmlNewTextWriterFilename(opts->output_filename, 0);
	    if (!writer)  {
		    fprintf (stderr, "%s:%s:failed to create writer for %s\n",
			     PROGNAME, __FUNCTION__, opts->output_filename);
		    return 1;
	    }
	    xmlTextWriterSetIndent (writer, 1);
	    if (xmlTextWriterStartDocument(writer, 
					   "1.0", 
					   "UTF-8", 
					   NULL) < 0) {
		    fprintf (stderr, "%s:%s:failed to write document start\n",
			     PROGNAME, __FUNCTION__);
		    return 1;
	    }
		    
	    if (xmlTextWriterStartElement (writer, BAD_CAST "testresults") 
		< 0) {
		    fprintf (stderr, "%s:%s:failed to write testsresults tag\n",
			     PROGNAME, __FUNCTION__);
		    return 1;
	    }
	    if (xmlTextWriterWriteAttribute (writer, 
					     BAD_CAST "version", 
					     BAD_CAST "1.0") < 0)
		    return 1;

	    /*
	     * Set callbacks
	     */
	    out_cbs.write_pre_suite_tag = xml_write_pre_suite_tag;
	    out_cbs.write_post_suite_tag = xml_end_element;
	    out_cbs.write_pre_set_tag = xml_write_pre_set_tag;
	    out_cbs.write_post_set_tag = xml_write_post_set_tag;
	     
	    
	    break;
	    
    case OUTPUT_TYPE_TXT:
	    /*
	     * Open results file
	     */
	    ofile = fopen (opts->output_filename, "w");
	    if (!ofile)  {
		    fprintf (stderr, "%s:%s:failed to open file %s %s\n",
			     PROGNAME, __FUNCTION__, opts->output_filename,
			     strerror(errno));
		    return 1;
	    }
	    fprintf (ofile,"Test results:\n");
	    fprintf (ofile, "  environment : %s\n", opts->environment);

	    /*
	     * Set callbacks
	     */
	    out_cbs.write_pre_suite_tag = txt_write_pre_suite_tag;
	    out_cbs.write_post_suite_tag = txt_write_post_suite_tag;
	    out_cbs.write_pre_set_tag = txt_write_pre_set_tag;
	    out_cbs.write_post_set_tag = txt_write_post_set_tag;
	     
	    break;

    default:
	    fprintf (stderr, "%s:%s:invalid output type %d\n",
		     PROGNAME, __FUNCTION__, opts->output_type);
	    return 1;
    }
    
    return 0;
}
/* ------------------------------------------------------------------------- */
/** Call pre_suite_tag callback
 *  @param suite suite data
 *  @return 0 on success
 */
int write_pre_suite_tag (td_suite *suite)
{
	return out_cbs.write_pre_suite_tag (suite);
}
/* ------------------------------------------------------------------------- */
/** Call post_suite_tag callback
 *  @param suite suite data
 *  @return 0 on success
 */
int write_post_suite_tag (td_suite *suite)
{
	
	return out_cbs.write_post_suite_tag ();
}
/* ------------------------------------------------------------------------- */
/** Call pre_set_tag callback
 *  @param set set data
 *  @return 0 on success
 */
int write_pre_set_tag (td_set *set)
{
	return out_cbs.write_pre_set_tag (set);
}
/* ------------------------------------------------------------------------- */
/** Call post_set_tag callback
 *  @param set set data
 *  @return 0 on success
 */
int write_post_set_tag (td_set *set)
{
	
	return out_cbs.write_post_set_tag (set);
}
/* ------------------------------------------------------------------------- */
/** Close the result logger */
void close_result_logger (void)
{
	if (writer) {
		xml_end_element(); /* </testresults> */	
		xmlTextWriterFlush (writer);
		xmlFreeTextWriter (writer);
		writer = NULL;
	} else if (ofile) {
		fprintf (ofile, "----------------------------------"
			 "----------------------------------\n");
		fprintf (ofile, "End of test results.\n");
		fflush (ofile);
		fclose (ofile);
		ofile = NULL;
	} else {
		fprintf (stderr, "%s:%s: Result logger not open?\n",
			 PROGNAME, __FUNCTION__);
	}

	return;
}
/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
