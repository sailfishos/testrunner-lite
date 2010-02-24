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
xmlTextWriterPtr writer;


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
LOCAL int xml_end_element ();


/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_pre_suite_tag (td_suite *suite)
{
	
	if (xmlTextWriterStartElement (writer, BAD_CAST "suite") < 0)
		goto err_out;
	
	if (xmlTextWriterWriteAttribute (writer,  BAD_CAST "name", 
					 suite->name) < 0)
		goto err_out;
	return 0;
err_out:
	return 1;
}
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_pre_set_tag (td_set *set)
{
	
	if (xmlTextWriterStartElement (writer, BAD_CAST "set") < 0)
		goto err_out;
	
	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "name", 
					 set->name) < 0)
		goto err_out;
	
	return 0;
err_out:
	return 1;
}
/* ------------------------------------------------------------------------- */
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
LOCAL int xml_write_case (const void *data, const void *user)
{
	td_case *c = (td_case *)data;

	if (xmlTextWriterStartElement (writer, BAD_CAST "case") < 0)
		goto err_out;

	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "name", 
					 c->name) < 0)
		goto err_out;

	if (c->description)
		if (xmlTextWriterWriteAttribute (writer, 
						 BAD_CAST "description", 
						 c->description) < 0)
			goto err_out;

	if (xmlTextWriterWriteAttribute (writer, 
					 BAD_CAST "result", 
					 c->passed ?
					 BAD_CAST "FAIL" : BAD_CAST "PASS") < 0)
	    
		goto err_out;

	xmlListWalk (c->steps, xml_write_step, NULL);

	xml_end_element ();

	return 1;

err_out:
	fprintf (stderr, "%s:%s: error\n", PROGNAME, __FUNCTION__);
	
	return 0;
}
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_post_set_tag (td_set *set)
{
	
	xmlListWalk (set->cases, xml_write_case, NULL);
	
	return xml_end_element();
}
/* ------------------------------------------------------------------------- */
LOCAL int xml_end_element ()
{
	
	if (xmlTextWriterFullEndElement (writer) < 0)
		goto err_out;
	return 0;
err_out:
	return 1;
}
/* ------------------------------------------------------------------------- */


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
	    /*
	     * Set callbacks
	     */
	    out_cbs.write_pre_suite_tag = xml_write_pre_suite_tag;
	    out_cbs.write_post_suite_tag = xml_end_element;
	    out_cbs.write_pre_set_tag = xml_write_pre_set_tag;
	    out_cbs.write_post_set_tag = xml_write_post_set_tag;
	     
	    
	    break;
	    
    case OUTPUT_TYPE_TXT:
	break;

    default:
	fprintf (stderr, "%s:%s:invalid output type %d\n",
		 PROGNAME, __FUNCTION__, opts->output_type);
	return 1;
    }
    
    return 0;
}
/* ------------------------------------------------------------------------- */
int write_pre_suite_tag (td_suite *suite)
{
	return out_cbs.write_pre_suite_tag (suite);
}
/* ------------------------------------------------------------------------- */
int write_post_suite_tag (td_suite *suite)
{
	
	return out_cbs.write_post_suite_tag ();
}
/* ------------------------------------------------------------------------- */
int write_pre_set_tag (td_set *set)
{
	return out_cbs.write_pre_set_tag (set);
}
/* ------------------------------------------------------------------------- */
int write_post_set_tag (td_set *set)
{
	
	return out_cbs.write_post_set_tag (set);
}
/* ------------------------------------------------------------------------- */
void close_result_logger (void)
{
	xmlTextWriterFlush (writer);
	xmlFreeTextWriter (writer);
	
	return;
}
/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
