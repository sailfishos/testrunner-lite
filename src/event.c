/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Sami Lahtinen <ext-sami.t.lahtinen@nokia.com>
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
#include <libcqpid/cqpid.h>
#include <json.h>
#include <libxml/list.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "event.h"
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
static Connection *connection = NULL;
static Session *session = NULL;

/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
/* None */

/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */
static int types_match(json_object *obj, xmlChar *type);
static int add_to_json_object(const void *data, const void *user);
static void get_event_params(td_event *event, json_object *object);
static Session *broker_session();
/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */

/** 
 * Compares json_type of an object to an event param type string
 * 
 * @param obj Pointer to JSON object
 * @param type Type string
 * 
 * @return 1 if types match, 0 if not
 */
static int types_match(json_object *obj, xmlChar *type)
{
	/* json_object_get_type() cannot be called for a null object  */
	if (!obj) {
		return xmlStrEqual(type, BAD_CAST "null");
	}

	switch(json_object_get_type(obj)) {
	case json_type_boolean:
		if (!xmlStrEqual(type, BAD_CAST "boolean"))
			return 0;
		break;
	case json_type_double:
	case json_type_int:
		if (!xmlStrEqual(type, BAD_CAST "number"))
			return 0;
		break;
	case json_type_object:
		if (!xmlStrEqual(type, BAD_CAST "object"))
			return 0;
		break;
	case json_type_array:
		if (!xmlStrEqual(type, BAD_CAST "array"))
			return 0;
		break;
	case json_type_string:
		if (!xmlStrEqual(type, BAD_CAST "string"))
			return 0;
		break;
	default:
		return 0;
		break;
	}

	return 1;
}

/** 
 * Adds event parameter to JSON object.
 * 
 * @param data Pointer to td_event_param
 * @param user Pointer to json_object
 * 
 * @return 1 always
 */
static int add_to_json_object(const void *data, const void *user)
{
	const td_event_param *param = (const td_event_param *) data;
	json_object *obj = (json_object *) user;
	json_object *new_obj = NULL;
	int type_error = 1;

	if (param->type == NULL || param->name == NULL || 
	    param->value == NULL) {
		return 1;
	}

	if (xmlStrEqual(param->type, BAD_CAST "string")) {
		/* We don't expect quotes for a string as json_tokener does */
		new_obj = json_object_new_string(param->value);
	} else {
		new_obj = json_tokener_parse((const char*)param->value);
	}

	if (is_error(new_obj)) {
		/* error occured in parsing */
		LOG_MSG (LOG_WARNING, "Cannot parse event param '%s'",
			 (const char*)param->name);
		return 1;
	}

	if (!types_match(new_obj, param->type)) {
		/* actual type is other than specified in  param->type */
		LOG_MSG (LOG_WARNING, "Type conflict for event param '%s'",
			 (const char*)param->name);
		/* free json object */
		json_object_put(new_obj);
		return 1;
	}

	json_object_object_add(obj, (const char*)param->name, new_obj);

	return 1;
}

/** 
 * Parses JSON object into event parameters.
 * 
 * @param event Pointer to td_event
 * @param object Pointer to json_object
 */
static void get_event_params(td_event *event, json_object *object)
{
	td_event_param *param = NULL;
	char *key;
	struct json_object *val;
	struct lh_entry *entry;

	for(entry = json_object_get_object(object)->head;
	    (entry ? (key = (char*)entry->k,
		      val = (struct json_object*)entry->v, entry) : 0);
	    entry = entry->next) {
		param = td_event_param_create();

		if (!val) {
			/* null object needs special processing */
			param->type = xmlStrdup(BAD_CAST "null");
			param->name = xmlStrdup(BAD_CAST key);
			param->value = xmlStrdup(BAD_CAST "null");
		} else {
			switch (json_object_get_type(val)) {
			case json_type_boolean:
				param->type = xmlStrdup(BAD_CAST "boolean");
				break;
			case json_type_double:
			case json_type_int:
				param->type = xmlStrdup(BAD_CAST "number");
				break;
			case json_type_object:
				param->type = xmlStrdup(BAD_CAST "object");
				break;
			case json_type_array:
				param->type = xmlStrdup(BAD_CAST "array");
				break;
			case json_type_string:
				param->type = xmlStrdup(BAD_CAST "string");
				break;
			default:
				break;
			}

			param->name = xmlStrdup(BAD_CAST key);
			param->value = xmlStrdup(BAD_CAST 
						 json_object_get_string(val));
		}

		xmlListAppend (event->params, param);
	}
}

/** 
 * Creates connection and session to AMQP broker if not yet connected. Returns
 * pointer to a session handle.
 * 
 * @return Pointer to a session, 0 in failure
 */
static Session *broker_session()
{
	const char *url = "amqp:tcp:127.0.0.1:5672";
	const char *options = "";

	while (!connection) {
		connection = connection_new(url, options);

		if (!connection) {
			LOG_MSG (LOG_WARNING, "Creating AMQP "
				 "connection failed");
			break;
		}

		connection_open(connection);
		if (connection_is_open(connection) > 0) {
			LOG_MSG (LOG_INFO, "Opened connection to AMQP broker");
		} else {
			LOG_MSG (LOG_WARNING, "Cannot connect to AMQP broker");
		}

		session = connection_create_session(connection, "");
		if (!session) {
			LOG_MSG (LOG_WARNING, "Creating AMQP session failed");
		}

		break;
	}

	if (!connection || !session || connection_is_open(connection) <= 0) {
		cleanup_event_system();
	}

	return session;
}

/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */

/** 
 * Initializes event system.
 * 
 * 
 * @return 1 in success, 0 in failure
 */
int init_event_system()
{
	/* nothing to do */
	return 1;
}

/** 
 * Closes connection and session to broker.
 * 
 */
void cleanup_event_system()
{
	if (session) {
		session_close(session);
		session_destroy(session);
	}

	if (connection) {
		if (connection_is_open(connection) > 0) {
			LOG_MSG (LOG_INFO, "Closing connection to AMQP broker");
			connection_close(connection);
		}
		connection_destroy(connection);
	}

	connection = NULL;
	session = NULL;
}

/** 
 * Waits until an event is received or timeout occurs. 
 * 
 * @param event Event description
 * 
 * @return 1 on success, 0 in failure
 */
int wait_for_event(td_event *event)
{
	char *buf = NULL;
	Session *session = NULL;
	Receiver *receiver = NULL;
	Message *message = NULL;
	size_t len = 0;
	json_object *obj = NULL;
	int ret = 1;

	session = broker_session();
	if (!session) {
		ret = 0;
		goto out;
	}

	LOG_MSG (LOG_INFO, "Waiting for event: %s", event->resource);

	receiver = session_create_receiver_str(session, event->resource);
	if (!receiver) {
		ret = 0;
		goto out;
	}

	message = receiver_fetch(receiver, event->timeout * 1000UL);
	if (!message) {
		ret = 0;
		goto out;
	}

	LOG_MSG (LOG_INFO, "Received event");

	len = message_get_content_size(message);

	if (len > 0) {
		buf = (char *)malloc(len + 1);
		memcpy(buf, message_get_content_ptr(message), len);
		buf[len] = '\0';

		/* parse json content */
		obj = json_tokener_parse(buf);
		if (!is_error(obj)) {
			if (json_object_get_type(obj) == json_type_object) {
				get_event_params(event, obj);
			}
			json_object_put(obj);
		} else {
			LOG_MSG (LOG_WARNING, "Cannot parse received "
				 "event content");
		}

		free(buf);
	}

	session_acknowledge(session);

 out:
	if (message) {
		message_destroy(message);
	}

	if (receiver) {
		receiver_destroy(receiver);
	}

	return ret;
}

/** 
 * Sends an event.
 * 
 * @param event Event description
 * 
 * @return 1 on success, 0 in failure
 */
int send_event(td_event *event)
{
	Session *session = NULL;
	Sender *sender = NULL;
	Message *message = NULL;
	json_object *obj = NULL;
	const char* token = NULL;
	int ret = 1;

	session = broker_session();
	if (!session) {
		ret = 0;
		goto out;
	}

	obj = json_object_new_object();

	/* add event params to json object */
	xmlListWalk(event->params, add_to_json_object, obj);

	token = json_object_to_json_string(obj);

	sender = session_create_sender_str(session, event->resource);
	if (!sender) {
		ret = 0;
		goto out;
	}

	message = message_new();
	if (!message) {
		ret = 0;
		goto out;
	}

	message_set_content(message, token, strlen(token));
	sender_send(sender, message);

	LOG_MSG (LOG_INFO, "Sending event: %s", event->resource);

 out:
	if (message) {
		message_destroy(message);
	}
	if (sender) {
		sender_destroy(sender);
	}
	if (obj) {
		json_object_put(obj);
	}

	return ret;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
