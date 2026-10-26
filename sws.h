/*
 * Copyright (c) 2026 vuongngan-se
 * All rights reserved.
 *
 * CS631 Advanced Programming in the UNIX Environment
 * sws - Simple Web Server (RFC 1945 HTTP/1.0)
 */

#ifndef SWS_H
#define SWS_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>

#define DEFAULT_PORT	8080
#define BACKLOG		128
#define MAX_LISTEN_FDS	8
#define SWS_VERSION	"sws/1.0"
#define REQ_BUF_SIZE	8192

/*
 * Server configuration parsed from command line arguments.
 */
struct server_conf {
	char	*cflag;			/* -c dir: CGI directory */
	char	*iflag;			/* -i address: bind address */
	char	*lflag;			/* -l file: log file */
	char	 docroot[PATH_MAX];	/* dir: absolute path to docroot */
	int	 dflag;			/* -d: debugging mode */
	int	 hflag;			/* -h: usage */
	int	 port;			/* -p port: listen port */
};

/*
 * Portability shims for Linux / NetBSD / OmniOS
 */
#ifndef __NetBSD__
# ifdef __linux__
#  ifndef getprogname
#   define getprogname() (program_invocation_short_name)
#  endif
#  ifndef setprogname
#   define setprogname(name) ((void)0)
#  endif
# endif
#endif

/*
 * Portable safe string copy
 */
size_t	portable_strlcpy(char *dst, const char *src, size_t siz);
#if !defined(__NetBSD__) && !defined(__FreeBSD__) && !defined(__OpenBSD__)
# ifndef strlcpy
#  define strlcpy portable_strlcpy
# endif
#endif

/*
 * Subsystem interfaces
 */
#include "network.h"
#include "daemon.h"
#include "http.h"
#include "header.h"
#include "response.h"
#include "uri.h"
#include "dir_index.h"
#include "mime.h"
#include "cgi.h"
#include "logger.h"

/*
 * Client connection handler
 */
void	handle_client(int client_fd, const struct sockaddr_storage *client_addr,
	    socklen_t addr_len, const struct server_conf *conf);

#endif /* SWS_H */
