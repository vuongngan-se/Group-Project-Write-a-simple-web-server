/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 4: CGI Engine Implementation (RFC 3875)
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cgi.h"

#define CGI_BUF_SIZE	4096

/*
 * Set up RFC 3875 environment variables for CGI execution.
 */
static void
setup_cgi_env(const struct cgi_params *p)
{
	(void)setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	(void)setenv("SERVER_SOFTWARE", "sws/1.0", 1);
	(void)setenv("SERVER_PROTOCOL", "HTTP/1.0", 1);
	(void)setenv("REQUEST_METHOD", (p->method != NULL) ? p->method : "GET",
	    1);
	(void)setenv("QUERY_STRING", (p->query_string != NULL) ?
	    p->query_string : "", 1);
	(void)setenv("SCRIPT_NAME", (p->script_name != NULL) ?
	    p->script_name : "", 1);
	(void)setenv("PATH_INFO", "", 1);
	(void)setenv("REMOTE_ADDR", (p->client_ip != NULL) ?
	    p->client_ip : "127.0.0.1", 1);
	(void)setenv("SERVER_PORT", (p->server_port != NULL) ?
	    p->server_port : "8080", 1);
}

/*
 * Execute a CGI script using fork/pipe/execve per RFC 3875.
 */
int
cgi_execute(int client_fd, const struct cgi_params *params)
{
	char buf[CGI_BUF_SIZE];
	pid_t pid;
	ssize_t nread, nwritten, total;
	int pipe_out[2], status;

	if (params == NULL || params->script_path == NULL) {
		return -1;
	}

	if (pipe(pipe_out) < 0) {
		return -1;
	}

	pid = fork();
	if (pid < 0) {
		(void)close(pipe_out[0]);
		(void)close(pipe_out[1]);
		return -1;
	}

	if (pid == 0) {
		/* Child process: configure CGI environment and execute */
		(void)close(pipe_out[0]);
		if (dup2(pipe_out[1], STDOUT_FILENO) < 0) {
			_exit(EXIT_FAILURE);
		}
		(void)close(pipe_out[1]);

		setup_cgi_env(params);

		(void)execl(params->script_path, params->script_path,
		    (char *)NULL);
		_exit(127);
	}

	/* Parent process: relay CGI stdout to client socket */
	(void)close(pipe_out[1]);

	/* Initial HTTP/1.0 header wrapper for CGI script output */
	(void)snprintf(buf, sizeof(buf),
	    "HTTP/1.0 200 OK\r\n"
	    "Server: sws/1.0\r\n"
	    "Connection: close\r\n");
	(void)write(client_fd, buf, strlen(buf));

	while ((nread = read(pipe_out[0], buf, sizeof(buf))) > 0) {
		total = 0;
		while (total < nread) {
			nwritten = write(client_fd, buf + total,
			    (size_t)(nread - total));
			if (nwritten <= 0) {
				break;
			}
			total += nwritten;
		}
	}
	(void)close(pipe_out[0]);

	(void)waitpid(pid, &status, 0);

	return 0;
}
