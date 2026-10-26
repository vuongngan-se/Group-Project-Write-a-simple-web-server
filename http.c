/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 2: HTTP Request Line Parser (RFC 1945)
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "http.h"

/*
 * Check if the HTTP method is supported by sws.
 * Only GET and HEAD are supported per RFC 1945 specification.
 */
int
http_is_supported_method(const char *method)
{
	if (strcmp(method, "GET") == 0 || strcmp(method, "HEAD") == 0) {
		return 1;
	}
	return 0;
}

/*
 * Parse HTTP Request-Line: Method SP Request-URI SP HTTP-Version CRLF
 * Automatically downgrades HTTP/1.1 requests to HTTP/1.0.
 */
int
http_parse_request_line(const char *line, struct http_request *req)
{
	char copy[HTTP_LINE_MAX];
	char *method, *uri, *ver, *saveptr;
	size_t len;

	if (line == NULL || req == NULL) {
		return 400;
	}

	(void)memset(req, 0, sizeof(*req));
	req->if_modified_since = (time_t)-1;

	/* Save raw line (without trailing CR/LF) for logging */
	(void)snprintf(req->raw_line, sizeof(req->raw_line), "%s", line);
	len = strlen(req->raw_line);
	while (len > 0 && (req->raw_line[len - 1] == '\r' ||
	    req->raw_line[len - 1] == '\n')) {
		req->raw_line[--len] = '\0';
	}

	(void)snprintf(copy, sizeof(copy), "%s", req->raw_line);

	method = strtok_r(copy, " \t", &saveptr);
	uri = strtok_r(NULL, " \t", &saveptr);
	ver = strtok_r(NULL, " \t", &saveptr);

	if (method == NULL || uri == NULL) {
		return 400;
	}

	(void)snprintf(req->method, sizeof(req->method), "%s", method);
	(void)snprintf(req->uri, sizeof(req->uri), "%s", uri);

	/* Check HTTP version */
	if (ver != NULL) {
		if (strncmp(ver, "HTTP/1.0", 8) == 0 ||
		    strncmp(ver, "HTTP/1.1", 8) == 0) {
			/* Quietly downgrade HTTP/1.1 to HTTP/1.0 */
			(void)snprintf(req->version, sizeof(req->version),
			    "HTTP/1.0");
		} else {
			return 400;
		}
	} else {
		/* Simple HTTP/0.9 request treated as HTTP/1.0 response */
		(void)snprintf(req->version, sizeof(req->version), "HTTP/1.0");
	}

	/* Validate method */
	if (!http_is_supported_method(req->method)) {
		return 501;
	}

	/* Relative URI check (RFC 1945 section 3.2.1) */
	if (req->uri[0] != '/') {
		return 400;
	}

	return 200;
}
