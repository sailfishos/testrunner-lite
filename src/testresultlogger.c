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
#include "testresultlogger.h"
#include <libxml/xmlwriter.h>
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
    int (*write_post_set_tag) (void);

} out_cbs;
/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
/* None */

/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_pre_suite_tag (td_suite *suite)
{
	int ret = 0;
	
	ret = xmlTextWriterStartElement (writer, BAD_CAST "suite");
	if (ret < 0)
		goto err_out;
	
	ret = xmlTextWriterWriteAttribute (writer, 
					   BAD_CAST "name", 
					   suite->name);
	return 0;
err_out:
	return 1;
}
/* ------------------------------------------------------------------------- */
LOCAL int xml_write_pre_set_tag (td_set *set)
{
	int ret = 0;
	
	ret = xmlTextWriterStartElement (writer, BAD_CAST "set");
	if (ret < 0)
		goto err_out;
	
	ret = xmlTextWriterWriteAttribute (writer, 
					   BAD_CAST "name", 
					   set->name);
	return 0;
err_out:
	return 1;
}
/* ------------------------------------------------------------------------- */
LOCAL int xml_end_element ()
{
	int ret = 0;
	
	ret = xmlTextWriterFullEndElement (writer);
	if (ret < 0)
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
	    /*
	     * Set callbacks
	     */
	    out_cbs.write_pre_suite_tag = xml_write_pre_suite_tag;
	    out_cbs.write_post_suite_tag = xml_end_element;
	    out_cbs.write_pre_set_tag = xml_write_pre_set_tag;
	    out_cbs.write_post_set_tag = xml_end_element;
	    
	    
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
int write_post_set_tag ()
{
	
	return out_cbs.write_post_set_tag ();
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
