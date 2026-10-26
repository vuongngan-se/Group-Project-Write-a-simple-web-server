/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 3: URI Resolution, Security & User Directory Header
 */

#ifndef MEMBER3_URI_H
#define MEMBER3_URI_H

#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#define SWS_TYPE_FILE	1
#define SWS_TYPE_DIR	2
#define SWS_TYPE_CGI	3

struct resource_info {
	char		resolved_path[PATH_MAX];
	char		request_uri[PATH_MAX];
	struct stat	sb;
	int		resource_type;	/* FILE, DIR, CGI */
};

/*
 * Resolve incoming relative URI against docroot (or user directory /~user/).
 * Prevents directory traversal attacks (returns 403).
 * Returns HTTP status: 200 (OK), 403 (Forbidden), 404 (Not Found).
 */
int	uri_resolve(const char *uri, const char *docroot, const char *cgi_dir,
	    struct resource_info *res);

#endif /* MEMBER3_URI_H */
