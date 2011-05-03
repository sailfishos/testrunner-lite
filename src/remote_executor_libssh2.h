/*
 * This file is part of testrunner-lite
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Timo Makimattila <timo.makimattila@digia.com>
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

#ifndef REMOTE_EXECUTOR_LIBSSH2_H
#define REMOTE_EXECUTOR_LIBSSH2_H

/* ------------------------------------------------------------------------- */
/* INCLUDES */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <libssh2.h>
#include "testrunnerlite.h"
#include "executor.h"

/* ------------------------------------------------------------------------- */
/* CONSTANTS */
/* None */

/* ------------------------------------------------------------------------- */
/* MACROS */
/* None */

/* ------------------------------------------------------------------------- */
/* DATA TYPES */
/* ------------------------------------------------------------------------- */
/* None */

/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* STRUCTURES */
typedef enum {
	SESSION_OK = 1,
	SESSION_GIVE_UP
} connection_status;

typedef struct libssh2_conn {
	struct libssh2_knownhost *host;
	struct timespec timeout;
	struct sockaddr_in sin;
	const char *hostname;
	const char *username;
	const char *password;
	const char *fingerprint;
	char *priv_key; 
	char *pub_key;
	unsigned long hostaddr;
	in_port_t port;
	int sock;
	fd_set nfd;
	fd_set *writefd;
	fd_set *readfd;
	LIBSSH2_SESSION *ssh2_session;
	connection_status status;
	int signaled;
} libssh2_conn;
/* ------------------------------------------------------------------------- */
/* FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */
libssh2_conn *lssh2_executor_init(const char *username, const char *hostname,
                                  in_port_t port, const char *priv_key, 
                                  const char *pub_key);
/* ------------------------------------------------------------------------- */
int lssh2_execute(libssh2_conn *conn, const char *command, 
		  exec_data *data);
/* ------------------------------------------------------------------------- */
int lssh2_executor_close(libssh2_conn *conn);
/* ------------------------------------------------------------------------- */
int lssh2_signal (int signal);
/* ------------------------------------------------------------------------- */

#endif                          /* REMOTE_EXECUTOR_LIBSSH2_H */
/* End of file */
