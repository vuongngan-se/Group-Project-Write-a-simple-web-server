/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 3: URI Resolution, Path Traversal Defense & User Dirs
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "uri.h"

/*
 * Check if the resolved path is located within the allowed base directory.
 */
static int
is_subpath(const char *base, const char *path)
{
	size_t blen;

	blen = strlen(base);
	if (strncmp(base, path, blen) != 0) {
		return 0;
	}
	if (path[blen] == '\0' || path[blen] == '/' || base[blen - 1] == '/') {
		return 1;
	}
	return 0;
}

/*
 * Resolve incoming relative URI against document root or user directory.
 */
int
uri_resolve(const char *uri, const char *docroot, const char *cgi_dir,
    struct resource_info *res)
{
	struct passwd *pw;
	char base_dir[PATH_MAX], target[PATH_MAX * 2];
	char uname[64], user_root[PATH_MAX];
	const char *p, *slash;
	size_t ulen;

	if (uri == NULL || docroot == NULL || res == NULL) {
		return 400;
	}

	(void)memset(res, 0, sizeof(*res));
	(void)snprintf(res->request_uri, sizeof(res->request_uri), "%s", uri);

	/* Check for CGI execution path: /cgi-bin */
	if (cgi_dir != NULL && (strncmp(uri, "/cgi-bin/", 9) == 0 ||
	    strcmp(uri, "/cgi-bin") == 0)) {
		p = (uri[8] == '/') ? uri + 9 : "";
		(void)snprintf(target, sizeof(target), "%s/%s", cgi_dir, p);
		if (realpath(target, res->resolved_path) == NULL) {
			return 404;
		}
		if (stat(res->resolved_path, &res->sb) < 0) {
			return (errno == EACCES) ? 403 : 404;
		}
		res->resource_type = SWS_TYPE_CGI;
		return 200;
	}

	/* Check for user directory path: /~<user>/ */
	if (strncmp(uri, "/~", 2) == 0) {
		p = uri + 2;
		slash = strchr(p, '/');
		if (slash != NULL) {
			ulen = (size_t)(slash - p);
			if (ulen >= sizeof(uname)) {
				return 400;
			}
			(void)memcpy(uname, p, ulen);
			uname[ulen] = '\0';
			p = slash;
		} else {
			(void)snprintf(uname, sizeof(uname), "%s", p);
			p = "";
		}

		errno = 0;
		pw = getpwnam(uname);
		if (pw == NULL) {
			return 404;
		}

		(void)snprintf(user_root, sizeof(user_root), "%s/sws",
		    pw->pw_dir);
		if (realpath(user_root, base_dir) == NULL) {
			return 404;
		}
		(void)snprintf(target, sizeof(target), "%s/%s", base_dir,
		    (*p == '/') ? p + 1 : p);
	} else {
		/* Normal docroot mapping */
		(void)snprintf(base_dir, sizeof(base_dir), "%s", docroot);
		(void)snprintf(target, sizeof(target), "%s/%s", docroot,
		    (*uri == '/') ? uri + 1 : uri);
	}

	/* Detect ../ directory traversal patterns */
	if (strstr(uri, "/../") != NULL ||
	    (strlen(uri) >= 3 && strcmp(uri + strlen(uri) - 3, "/..") == 0)) {
		return 403;
	}

	/* Verify target directory existence and security boundaries */
	if (realpath(target, res->resolved_path) == NULL) {
		return (errno == EACCES) ? 403 : 404;
	}

	if (!is_subpath(base_dir, res->resolved_path)) {
		return 403;
	}

	if (stat(res->resolved_path, &res->sb) < 0) {
		return (errno == EACCES) ? 403 : 404;
	}

	if (S_ISDIR(res->sb.st_mode)) {
		res->resource_type = SWS_TYPE_DIR;
	} else if (S_ISREG(res->sb.st_mode)) {
		res->resource_type = SWS_TYPE_FILE;
	} else {
		return 403;
	}

	return 200;
}
