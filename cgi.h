/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 4: CGI Subsystem Interface (RFC 3875)
 */

#ifndef MEMBER4_CGI_H
#define MEMBER4_CGI_H

#include <sys/types.h>

struct cgi_params {
	const char	*script_path;
	const char	*script_name;
	const char	*method;
	const char	*query_string;
	const char	*client_ip;
	const char	*server_port;
};

/*
 * Execute a CGI script per RFC 3875 and stream the response to client_fd.
 * Returns 0 on success, -1 on failure (e.g. fork/exec failed).
 */
int	cgi_execute(int client_fd, const struct cgi_params *params);

#endif /* MEMBER4_CGI_H */
