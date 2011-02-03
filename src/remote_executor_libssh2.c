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

/* ------------------------------------------------------------------------- */
/* INCLUDE FILES */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <libssh2.h>

#include "testrunnerlite.h"
#include "executor.h"
#include "testdefinitionprocessor.h"
#include "remote_executor.h"
#include "log.h"
#include "remote_executor_libssh2.h"

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

/* How many tries after a session dies */
#define MAX_SSH_RETRIES 5
/* Size we try to read from ssh session */
#define CHANNEL_BUFFER_SIZE 1024
#define KNOWN_HOSTS_FILE "known_hosts"
#define DEFAULT_PUBLIC_KEY "id_eat_dsa.pub"
#define DEFAULT_PRIVATE_KEY "id_eat_dsa"
#define KEY_FMT "%s/.ssh/%s"
/* A command to be run in the remote end while executing a test step */
#define TRLITE_RUN_CMD "sh /var/tmp/testrunner-lite.sh '%s'"
/* Kills the children of the jammed shell session */
#define TRLITE_KILL_SHELL_CMD "kill -%d $(ps --ppid $(cat\
 /var/tmp/testrunner-lite-shell.pid) -o pid=)"
/* Kills stored background process PIDs from remote end */
#define TRLITE_KILL_BG_PIDS_CMD "kill -9\
  $(cat /var/tmp/testrunner-lite-children.pid)"
/* Cleans up helper shell scripts from remote end */
#define TRLITE_CLEAN_CMD "rm /var/tmp/testrunner-lite-children.pid\
 /var/tmp/testrunner-lite.sh /var/tmp/testrunner-lite-shell.pid"

/* A shell script deployed to remote end that executes a test step,
   handles freezing ssh sessions by a horrible brute force hack
   and writes down background jobs */
#define REMOTE_RUN_SCRIPT "echo '#!/bin/bash\n\
echo $$ > /var/tmp/testrunner-lite-shell.pid\n\
bgjobs=0\n\
eval $@\n\
ret=$?\n\
for bgjob in `jobs -p`\n\
do\n\
echo -n $bgjob ' '  >> /var/tmp/testrunner-lite-children.pid\n\
bgjobs=1\n\
done\n\
if [ $bgjobs -eq 1 ]; then\n\
kill -9 $PPID\n\
fi\n\
exit $ret\n' > /var/tmp/testrunner-lite.sh"

typedef enum {
	NO_TIMEOUT = 1,
	SOFT_TIMEOUT,
	SOFT_TIMEOUT_KILLED,
	HARD_TIMEOUT,
	HARD_TIMEOUT_KILLED
} trlite_timeout;

/* ------------------------------------------------------------------------- */
/* LOCAL GLOBAL VARIABLES */
static sigset_t blocked_signals;
volatile trlite_timeout last_timeout = NO_TIMEOUT;
/* ------------------------------------------------------------------------- */
/* LOCAL CONSTANTS AND MACROS */
#define UNIQUE_ID_FMT     "%d"
#define PID_FILE      "/var/tmp/testrunner-lite-children.pid"
#define UNIQUE_ID_MAX_LEN (HOST_NAME_MAX + 10 + 1 + 1)
#define PID_FILE_MAX_LEN  (30 + UNIQUE_ID_MAX_LEN + 10 + 1 + 1)

/* ------------------------------------------------------------------------- */
/* MODULE DATA STRUCTURES */
/* none */
/* ------------------------------------------------------------------------- */
/* LOCAL FUNCTION PROTOTYPES */
/* ------------------------------------------------------------------------- */

static int lssh2_set_timer(unsigned long soft_timeout, 
                           unsigned long hard_timeout);
static void lssh2_stop_timer();
static void lssh2_timeout(int signal, siginfo_t *siginfo, void *data);
static int lssh2_check_timeouts(libssh2_conn *conn);
static int lssh2_setup_socket(libssh2_conn *conn);
static int lssh2_select(libssh2_conn *conn);
static int lssh2_read_output(libssh2_conn *conn, 
                             LIBSSH2_CHANNEL *channel, exec_data *data);
static int lssh2_session_connect(libssh2_conn *conn);
static int lssh2_session_reconnect(libssh2_conn *conn);
static int lssh2_session_free(libssh2_conn *conn);
static int lssh2_channel_close(LIBSSH2_CHANNEL *channel, int *exitcode);
static int lssh2_execute_command(libssh2_conn *conn, char *command, 
                                 exec_data *data);
static int lssh2_create_shell_scripts(libssh2_conn *conn);


/* ------------------------------------------------------------------------- */
/* FORWARD DECLARATIONS */
/* None */

/* ------------------------------------------------------------------------- */
/* ==================== LOCAL FUNCTIONS ==================================== */
/* ------------------------------------------------------------------------- */

/** Initialize timer and set signal action for SIGALRM
 * @param timeout Value in seconds after which global variable timer_value
 * will be set
 * @return 0 in success, -1 in error
 */
static int lssh2_set_timer(unsigned long soft_timeout, 
                           unsigned long hard_timeout) {

	struct sigaction act;
	struct itimerval timer;
	
	last_timeout = NO_TIMEOUT;
	act.sa_sigaction = &lssh2_timeout;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	timer.it_interval.tv_sec = hard_timeout;
	timer.it_interval.tv_usec = 0;
	timer.it_value.tv_sec = soft_timeout;
	timer.it_value.tv_usec = 0;

	/* set signal action. original action is stored in global 
	 * default_alarm_action */
	if (sigaction(SIGALRM, &act, NULL) < 0) {
		perror("sigaction");
		goto erraction;
	}

	if (setitimer(ITIMER_REAL, &timer, NULL) < 0) {
		perror("setitimer");
		goto errtimer;
	}

	LOG_MSG(LOG_DEBUG, "Set timeouts to: soft %u hard %u", 
	        soft_timeout, hard_timeout);

	return 0;

 errtimer:
 erraction:
	return -1;
}


/** Reset timer and restore signal action of SIGALRM
 */
static void lssh2_stop_timer() {
	struct itimerval timer;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 0;

	if (setitimer(ITIMER_REAL, &timer, NULL) < 0) {
		perror("setitimer");
	}
	last_timeout = NO_TIMEOUT;
	LOG_MSG(LOG_DEBUG, "Timer stopped");
}


/* ------------------------------------------------------------------------- */
/** Signal handler that tries to resolve a soft timeout in a polite manner. 
    And a hard timeout with brute force.
 */
static void lssh2_timeout(int signal, siginfo_t *siginfo, void *data) {
	if (signal == SIGALRM) {
		switch(last_timeout) {
		case NO_TIMEOUT:
			last_timeout = SOFT_TIMEOUT;
			break;
		case SOFT_TIMEOUT_KILLED:
			last_timeout = HARD_TIMEOUT;
			break;
		default:
			break;
		}
	}
}

/* ------------------------------------------------------------------------- */
/** Checks during reading if the timeout timer has expired. Kills processes
 *  accordingly.
 */
static int lssh2_check_timeouts(libssh2_conn *conn) {

	if (!conn) {
		LOG_MSG(LOG_ERR, "No connection");
		return -1;
	}
	
	switch(last_timeout) {
	case NO_TIMEOUT:
		//LOG_MSG(LOG_DEBUG, "No timeout");
		break;
	case SOFT_TIMEOUT:
		LOG_MSG(LOG_DEBUG, "Soft timeout, sending SIGTERM");
		lssh2_kill(conn, SIGTERM);
		last_timeout = SOFT_TIMEOUT_KILLED;
		break;
	case SOFT_TIMEOUT_KILLED:
		//LOG_MSG(LOG_DEBUG, "Soft timeout, already killed");
		break;
	case HARD_TIMEOUT:
		LOG_MSG(LOG_DEBUG, "Hard timeout, sending SIGKILL");
		lssh2_kill(conn, SIGKILL);
		last_timeout = HARD_TIMEOUT_KILLED;
		break;
	case HARD_TIMEOUT_KILLED:
		//LOG_MSG(LOG_DEBUG, "Hard timeout, already killed");
		break;
	default:
		break;
	}
	return 0;
}



/* ------------------------------------------------------------------------- */
/** Creates helper shell script to remote end
 * @param session SSH session
 * @return 0 on succes, -1 if fails
 */
static int lssh2_create_shell_scripts(libssh2_conn *conn) {
	/* Create shell script to target */
  exec_data data;
  data.stdout_data.buffer = NULL;
  data.stderr_data.buffer = NULL;
  data.result = -1;

  if (lssh2_execute_command(conn, REMOTE_RUN_SCRIPT, &data) < 0) {
	  LOG_MSG(LOG_ERR, "Creating remote run script failed");
	  return -1;
	}
	return 0;
}


/* ------------------------------------------------------------------------- */
/** Connects to remote end
 * @param conn SSH session
 * @return 0 on succes, -1 if fails
 */
static int lssh2_session_connect(libssh2_conn *conn) {

	LIBSSH2_KNOWNHOSTS *hosts;
	int check;
	int n;
	int type;  
	size_t len;

	LOG_MSG(LOG_DEBUG, "connecting to %s", conn->hostname);

	/* Create socket */
	if (lssh2_setup_socket(conn) < 0) {
		LOG_MSG(LOG_ERR, "Opening socket failed");
		return -1;
	}

	/* Create libssh2 session */
	conn->ssh2_session = libssh2_session_init();
	if (!conn->ssh2_session) {
		LOG_MSG(LOG_ERR, "libssh2 session init failed");
		return -1;
	}


	/* Set non-blocking mode */
	libssh2_session_set_blocking(conn->ssh2_session, 0);

	/* Set signals we wan't to block for pselect() */
	sigemptyset(&blocked_signals);
	//sigaddset(&blocked_signals, SIGALRM);
	sigprocmask(SIG_BLOCK, &blocked_signals, NULL);

	/* Start session */
	while ((n = libssh2_session_startup(conn->ssh2_session, conn->sock)) ==
	       LIBSSH2_ERROR_EAGAIN);

	if (n) {
		LOG_MSG(LOG_ERR, "libssh2 session startup failed");
		return -1;
	}
    
	hosts  = libssh2_knownhost_init(conn->ssh2_session);

	if(!hosts) {
		LOG_MSG(LOG_ERR, "libssh2_knownhost_init failed");
		libssh2_knownhost_free(hosts);
		return -1;
	}

	/* Read known hosts */
	libssh2_knownhost_readfile(hosts, "known_hosts",
	                           LIBSSH2_KNOWNHOST_FILE_OPENSSH);

	conn->fingerprint = libssh2_session_hostkey(conn->ssh2_session, &len, &type);
	if(conn->fingerprint) {
		check = libssh2_knownhost_check(hosts, (char *)conn->hostname,
		                                (char *)conn->fingerprint, len,
		                                LIBSSH2_KNOWNHOST_TYPE_PLAIN|
		                                LIBSSH2_KNOWNHOST_KEYENC_RAW,
		                                &conn->host);
		switch(check) {
			/* Ignoring errors currently */
		case LIBSSH2_KNOWNHOST_CHECK_FAILURE:
			LOG_MSG(LOG_ERR, "libssh2 knownhost check failure");
			break;
		case LIBSSH2_KNOWNHOST_CHECK_NOTFOUND:
			LOG_MSG(LOG_ERR, "libssh2 knownhost check not found");
			break;
		case LIBSSH2_KNOWNHOST_CHECK_MATCH:
			LOG_MSG(LOG_DEBUG, "libssh2 knownhost check match");
			break;
		case LIBSSH2_KNOWNHOST_CHECK_MISMATCH:
			LOG_MSG(LOG_ERR, "libssh2 knownhost check mitchmatch");
			break;
		default:
			break;
		}

	}	else {
		LOG_MSG(LOG_ERR, "libssh2_session_hostkey failed");
		libssh2_knownhost_free(hosts);
		return -1;
	}

	/* Authenticate with public key */

	LOG_MSG(LOG_DEBUG, "Authenticating with private key: %s, "
	        "public key: %s", conn->priv_key, conn->pub_key);
	while ((n = libssh2_userauth_publickey_fromfile(conn->ssh2_session, 
	                                                conn->username,
	                                                conn->pub_key,
	                                                conn->priv_key,
	                                                conn->password)) ==
	       LIBSSH2_ERROR_EAGAIN);
	if (n) {
		/* Won't be fixed via connection retries, so giving up */
		LOG_MSG(LOG_ERR, "Authentication by public key failed, giving up\n");
		conn->status = SESSION_GIVE_UP;
		libssh2_knownhost_free(hosts);
		return -1;
	}

	libssh2_knownhost_free(hosts);
	return 0;
}


/* ------------------------------------------------------------------------- */
/** Cleans up an SSH channel
 * @param channel SSH channel
 * @param exitcode pointer to store return value from command
 * @return 0 on succes, -1 if fails
 */
static int lssh2_channel_close(LIBSSH2_CHANNEL *channel, int *exitcode) {
	int n;
	*exitcode = -127;

	if (channel) {
		do {
			n = libssh2_channel_close(channel);
		} while (n == LIBSSH2_ERROR_EAGAIN);
		if (n < 0) {
			LOG_MSG(LOG_ERR, "Closing SSH channel failed, error %d", n);
			/* Continue to channel freeing, though */
		}
		/* success */
		if (!n) {
			*exitcode = libssh2_channel_get_exit_status(channel);
			LOG_MSG(LOG_DEBUG, "Got exit code %d", *exitcode);
		} else {
			LOG_MSG(LOG_ERR, "Failed to get exit code, error %d", n);
		}
		

		do {
			n = libssh2_channel_free(channel);
		} while (n == LIBSSH2_ERROR_EAGAIN);
		if (n < 0) {
			LOG_MSG(LOG_ERR, "Freeing SSH channel failed");
			return -1;
		} else if (n == 0) {
			LOG_MSG(LOG_DEBUG, "SSH channel freed");
			channel = NULL;
		}
	}     
	return 0;
}

/* ------------------------------------------------------------------------- */
/** Cleans up an SSH session
 * @param session SSH session
 * @return 0 on succes, -1 if fails
 */
static int lssh2_session_free(libssh2_conn *conn) {
  	LOG_MSG(LOG_DEBUG, "");
	if (!conn) {
		LOG_MSG(LOG_DEBUG, "No SSH session");
		return 0;
	}

	exec_data data;
	data.stdout_data.buffer = NULL;
	data.stderr_data.buffer = NULL;
	data.result = -1;
	
	if (conn->ssh2_session) {
		LOG_MSG(LOG_DEBUG, "%s", TRLITE_KILL_BG_PIDS_CMD);
		if (lssh2_execute_command(conn, TRLITE_KILL_BG_PIDS_CMD, &data) < 0) {
			LOG_MSG(LOG_ERR, "Killing remote PID:s failed");
		}
	}
	
	data.result = -1;

	LOG_MSG(LOG_DEBUG, "%s", TRLITE_CLEAN_CMD);
	if (lssh2_execute_command(conn, TRLITE_CLEAN_CMD, &data) < 0) {
		LOG_MSG(LOG_ERR, "Cleaning shell scripts failed");
	}

	if (conn->ssh2_session) {
		if (libssh2_session_disconnect(conn->ssh2_session, NULL) < 0) {
			LOG_MSG(LOG_ERR, "Disconnecting SSH session failed");
		}
	}

	if (conn->ssh2_session) {
		if (libssh2_session_free(conn->ssh2_session) < 0) {
			LOG_MSG(LOG_ERR, "Freeing SSH session failed");
		} else {
			conn->ssh2_session = NULL;			
		}
	}

	close(conn->sock);

	if (conn->priv_key) free(conn->priv_key);
	if (conn->pub_key) free(conn->pub_key);

	if (conn) {
		free(conn);
		conn = NULL;
	}
	return 0;
}

/* ------------------------------------------------------------------------- */
/** Reconnects SSH session
 * @param conn SSH session
 * @return 0 on succes, -1 if fails
 */
static int lssh2_session_reconnect(libssh2_conn *conn) {

	if (conn->ssh2_session) {
		if (libssh2_session_disconnect(conn->ssh2_session, NULL) < 0) {
			/* Ignore */
			LOG_MSG(LOG_ERR, "Reconnect: disconnecting old session failed");			
		}
		if (libssh2_session_free(conn->ssh2_session) < 0) {
			/* Ignore */
			LOG_MSG(LOG_ERR, "Reconnect: disconnecting old session failed");			
		}
	}

	close(conn->sock);
	
	if (lssh2_setup_socket(conn) < 0) {
		LOG_MSG(LOG_ERR, "Reconnect: setup socket failed");			
		return -1;
	}

	if (lssh2_session_connect(conn) < 0) {
		LOG_MSG(LOG_ERR, "Reconnect: session connect failed");			
		return -1;
	}
	return 0;
}

/* ------------------------------------------------------------------------- */
/** Creates socket and connects to remote end
 * @param conn SSH session
 * @return 0 on succes, -1 if fails
 */
static int lssh2_setup_socket(libssh2_conn *conn) {

	struct hostent *host;
	conn->hostaddr = inet_addr(conn->hostname);
	conn->sock = socket(AF_INET, SOCK_STREAM, 0);
	if (conn->sock < 0) {
		LOG_MSG(LOG_ERR, "Opening socket failed %s", strerror(errno));
		return -1;
	}

	conn->sin.sin_family = AF_INET;
	conn->sin.sin_port = htons(22);
	conn->sin.sin_addr.s_addr = conn->hostaddr;

	host = gethostbyname(conn->hostname);
	if (!host) {
		LOG_MSG(LOG_ERR, "Target addess '%s' not found", conn->hostname); 
		return -1;
	}

	memcpy (&(conn->sin.sin_addr.s_addr), host->h_addr, host->h_length);

	if (connect(conn->sock, (struct sockaddr*)(&conn->sin),
	            sizeof(struct sockaddr_in)) != 0) {
		LOG_MSG(LOG_ERR, "Connecting to remote end failed %s", strerror(errno));
		close(conn->sock);
		return -1;
	}

	return 0;
}

/* ------------------------------------------------------------------------- */
/** Waits for response with select
 * @param conn SSH session
 * @return 0 on succes, -1 if fails
 */
static int lssh2_select(libssh2_conn *conn) 
{
	int n;
	int dir;

	FD_ZERO(&conn->nfd);
	FD_SET(conn->sock, &conn->nfd);

	/* Get direction */
	dir = libssh2_session_block_directions(conn->ssh2_session);

	if (dir & LIBSSH2_SESSION_BLOCK_INBOUND) {
		conn->readfd = &conn->nfd;
	}

	if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND) {
		conn->writefd = &conn->nfd;
	}

	n = pselect(conn->sock + 1, conn->readfd, conn->writefd, NULL, 
	           &conn->timeout, &blocked_signals);

	if (n < 0) {
		LOG_MSG(LOG_DEBUG, "pselect() failed: %s", strerror(errno));
	}
	LOG_MSG(LOG_DEBUG, "select returned %d", n);

	return n;
}

/* ------------------------------------------------------------------------- */
/** Reads output after a command
 * @param conn SSH session
 * @param channel SSH channel
 * @param data exec_data to store output
 * @return 0 on succes, -1 if fails
 */
static int lssh2_read_output(libssh2_conn *conn, 
                           LIBSSH2_CHANNEL *channel, exec_data *data) {
	LOG_MSG(LOG_DEBUG, "");

	int alloc_size;

	int n_stdout = 0;
	char *stdout_buffer = (char*)data->stdout_data.buffer;
	int *stdout_length = &data->stdout_data.length;
	int *stdout_size = &data->stdout_data.size;
	
	int n_stderr = 0;
	char *stderr_buffer = (char*)data->stderr_data.buffer;
	int *stderr_length = &data->stderr_data.length;
	int *stderr_size = &data->stderr_data.size;

	while (1) {	
		do {
			/* Allocate more memory if needed */
			alloc_size = *stdout_length + CHANNEL_BUFFER_SIZE + 1 
				- *stdout_size;
			if (alloc_size > 0) {
				*stdout_size += alloc_size;
				stdout_buffer = realloc(stdout_buffer, *stdout_size);
				if (!stdout_buffer) {
					LOG_MSG(LOG_ERR, "realloc() for stdout buffer failed");
					return -1;
				}
				data->stdout_data.buffer = (unsigned char*)stdout_buffer;
			}
			n_stdout = libssh2_channel_read(channel, 
			                                &stdout_buffer[*stdout_length], 
			                                CHANNEL_BUFFER_SIZE );
			if (n_stdout > 0) {
				*stdout_length += n_stdout;
				stdout_buffer[*stdout_length] = '\0'; 
				LOG_MSG(LOG_DEBUG, "got %d bytes from stdout\n", n_stdout);
			}
			alloc_size = *stderr_length + CHANNEL_BUFFER_SIZE + 1 
				- *stderr_size;
			if (alloc_size > 0) {
				*stderr_size += alloc_size;
				stderr_buffer = realloc(stderr_buffer, *stderr_size);
				if (!stderr_buffer) {
					LOG_MSG(LOG_ERR, "realloc() for stderr buffer failed");
					return -1;
				}
				data->stderr_data.buffer = (unsigned char*)stderr_buffer;
			}
			n_stderr = libssh2_channel_read_stderr(channel, 
			                                       &stderr_buffer[*stderr_length], 
			                                       CHANNEL_BUFFER_SIZE );
			if (n_stderr > 0) {
				*stderr_length += n_stderr;
				stderr_buffer[*stderr_length] = '\0'; 
				LOG_MSG(LOG_DEBUG, "got %d bytes from stderr\n", n_stderr);
			}
		} while (n_stdout > 0 || n_stderr > 0);
		if (n_stdout == LIBSSH2_ERROR_EAGAIN ||
		    n_stderr == LIBSSH2_ERROR_EAGAIN) {
			lssh2_select(conn);	
			lssh2_check_timeouts(conn);
		} else {
			break;
		}	
	}
	return 0;
}


/* ------------------------------------------------------------------------- */
/** Creates an SSH channel and executes a command at remote end
 * @param conn SSH session
 * @param data exec_data to store output
 * @return 0 on succes, -1 if fails
 */
static int lssh2_execute_command(libssh2_conn *conn, char *command, 
                               exec_data *data) {

	LIBSSH2_CHANNEL *channel = NULL;
	int n;
	int retries = 0;

	if (!conn || !conn->ssh2_session || conn->status == SESSION_GIVE_UP) {
		LOG_MSG(LOG_DEBUG, "Cannot create SSH channel: no SSH session");
		return -1;
	}

	/* Open session channel */
	do {
		while( (channel = 
		        libssh2_channel_open_session(conn->ssh2_session)) == NULL &&
		       libssh2_session_last_error(conn->ssh2_session, NULL, NULL, 0) 
		       == LIBSSH2_ERROR_EAGAIN ) {
			lssh2_select(conn);
		} 
		if (channel) {
			break;
		}
		if (retries > MAX_SSH_RETRIES) {
			LOG_MSG(LOG_ERR, "Exceeding max number of retries " 
			        "for SSH connection. Giving up.\n");			
			return -1;
		}

		/* Session stuck */
		LOG_MSG(LOG_DEBUG, "SSH session stuck\n");
		lssh2_session_reconnect(conn);
		sleep(1);
		retries++;
	} while (!channel);
  
	while ((n = libssh2_channel_exec(channel, command)) == LIBSSH2_ERROR_EAGAIN) {
		lssh2_select(conn);
	}

	/* Read streams if the receiving buffers exist 
	   (Not all commands expect output) */
	if (data->stdout_data.buffer && data->stderr_data.buffer) {
		lssh2_read_output(conn, channel, data);
	} else {
		libssh2_channel_flush_ex(channel, LIBSSH2_CHANNEL_FLUSH_ALL);
	}

	lssh2_channel_close(channel, &data->result);	
	return 0;

}


/* ------------------------------------------------------------------------- */
/* ======================== FUNCTIONS ====================================== */
/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */
/** Creates an instance of SSH session
 * @param username User name 
 * @param hostname Host name
 * @return session instance on success, NULL if fails
 */
 libssh2_conn *lssh2_executor_init(const char *username, const char *hostname,
                                   const char *priv_key, const char *pub_key) {

	LOG_MSG(LOG_DEBUG, "");
	char *home_dir;
	char *private_key_file;
	char *public_key_file;
	int key_size;
	libssh2_conn *conn;	
	conn = malloc(sizeof(libssh2_conn));
	conn->hostname = hostname;
	conn->username = username;
	conn->password = "";
	conn->writefd = NULL;
	conn->readfd = NULL;
	conn->timeout.tv_sec = 10;
	conn->timeout.tv_nsec = 0;
	
	/* No ssh key files given, using default */
	if (!priv_key || !pub_key || !strlen(priv_key) || !strlen(pub_key)) { 
		priv_key = DEFAULT_PRIVATE_KEY;
		pub_key = DEFAULT_PUBLIC_KEY;
	}

	/* Get the ssh keys from the user running testrunner-lite */
	home_dir = getenv("HOME");
	if (!home_dir) {
		LOG_MSG(LOG_ERR, "Fatal: Could not get env for "
		        "HOME");
		conn->status = SESSION_GIVE_UP;
		return NULL;
	}

	/* Create absolute paths to keys for libssh2
	   ( ~ notation doesn't work in 1.2.6 yet) */
	key_size = strlen(home_dir) + 
		strlen(KEY_FMT) + 
		strlen(pub_key) - 1;
	public_key_file = malloc(key_size);
	snprintf(public_key_file, key_size, KEY_FMT, home_dir, pub_key);

	key_size = strlen(home_dir) + 
		strlen(KEY_FMT) + 
		strlen(priv_key) - 1;
	private_key_file = malloc(key_size);
	snprintf(private_key_file, key_size, KEY_FMT, home_dir, priv_key);

	LOG_MSG(LOG_DEBUG, "public key file: %s\n", public_key_file);
	LOG_MSG(LOG_DEBUG, "private key file: %s\n", private_key_file);

	conn->priv_key = private_key_file;
	conn->pub_key = public_key_file;
	conn->status = SESSION_OK;
	
	if (lssh2_session_connect(conn) < 0) {
		LOG_MSG(LOG_ERR, "Connection error");
		lssh2_session_free(conn);
		return NULL;
	}

	if (lssh2_create_shell_scripts(conn) < 0) {
		LOG_MSG(LOG_ERR, "Error creating remote shell scripts");
		lssh2_session_free(conn);
		return NULL;
	}

	return conn;
}


/* ------------------------------------------------------------------------- */
/** Builds a test command out of test step and executes it in remote end
 * @param conn SSH session
 * @param data exec_data to store output
 * @return 0 on succes, -1 if fails
 */
int lssh2_execute(libssh2_conn *conn, const char *command, 
                                    exec_data *data) {
	int test_cmd_size;
	char *test_cmd;
	char *log_cmd;
	char *casename;
	char *setname;
	int stepnum;
	int log_cmd_size;
	int ret = -1;

	if (conn->status == SESSION_GIVE_UP) {
		LOG_MSG(LOG_ERR, "Fatal error, can't (re)connect");
		return -1;
	}

 	/*
	 * Query the current set name, case name and step number 
	 * from testdefinition processor so we can put some debug 
	 * to target syslog
	 */
	casename = (char*)current_case_name();
	stepnum  = current_step_num();
	setname  = (char*)current_set_name();

	log_cmd_size = strlen(command) + strlen(casename) +
		strlen(setname) + 130;

	/* Remote end logger command */
	log_cmd = malloc(log_cmd_size);
	if (!log_cmd) {
		return ret;
	}
	snprintf (log_cmd, log_cmd_size, "logger set:%s-case:%s-step:%d",
	          setname, casename, stepnum);
		
	exec_data logger_data;
	logger_data.stdout_data.buffer = NULL;
	logger_data.stderr_data.buffer = NULL;
	logger_data.result = -1;

	if (lssh2_execute_command(conn, log_cmd, &logger_data) < 0) {
		LOG_MSG(LOG_ERR, "Logger command failed");
	}	

	test_cmd_size = strlen(TRLITE_RUN_CMD) + strlen(command) + 1;
	test_cmd = malloc(test_cmd_size);
	snprintf(test_cmd, test_cmd_size, TRLITE_RUN_CMD, command);
	LOG_MSG(LOG_DEBUG, "Executing test command: %s\n", test_cmd);
	lssh2_set_timer(data->soft_timeout, data->hard_timeout);
	if (lssh2_execute_command(conn, test_cmd, data) < 0) {
		LOG_MSG(LOG_ERR, "Executing test command failed");
		ret = -1;
	} else {
		ret = 0;
	}
	lssh2_stop_timer();

	LOG_MSG(LOG_DEBUG, "Test case return value %d", data->result);

	free(log_cmd);
	free(test_cmd);
	return ret;
}

/* ------------------------------------------------------------------------- */
/** Clean up
 * returns 0 on success, -1 if fails
 */
int lssh2_executor_close (libssh2_conn *conn)
{
	LOG_MSG(LOG_DEBUG, "");
	return lssh2_session_free(conn);
}

/* ------------------------------------------------------------------------- */
/** Kills remote shell
 * returns 0 on success, -1 if fails
 */
int lssh2_kill (libssh2_conn *conn, int signal)
{
	LOG_MSG(LOG_DEBUG, "");
	
	char *kill_cmd;
	int kill_cmd_size;
	exec_data data;

	if (!conn) {
		LOG_MSG(LOG_DEBUG, "No session");
		return -1;
	}

	kill_cmd_size = strlen(TRLITE_KILL_SHELL_CMD) + 5; /* Max PID size */
	kill_cmd = malloc(kill_cmd_size);
	snprintf (kill_cmd, kill_cmd_size, TRLITE_KILL_SHELL_CMD,
	          signal);

	data.stdout_data.buffer = NULL;
	data.stderr_data.buffer = NULL;
	data.result = -1;

	LOG_MSG(LOG_DEBUG, "%s", kill_cmd);
	if (lssh2_execute_command(conn, kill_cmd, &data) < 0) {
		LOG_MSG(LOG_ERR, "Killing remote shell failed");
	}

	free(kill_cmd);
	return 0;
}

/* ================= OTHER EXPORTED FUNCTIONS ============================== */
/* None */

/* ------------------------------------------------------------------------- */
/* End of file */
