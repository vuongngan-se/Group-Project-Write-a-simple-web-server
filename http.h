/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 2: HTTP Protocol Subsystem Header
 */

#ifndef MEMBER2_HTTP_H
#define MEMBER2_HTTP_H

#include <sys/types.h>
#include <time.h>

#define HTTP_METHOD_LEN		16
#define HTTP_URI_LEN		2048
#define HTTP_VER_LEN		16
#define HTTP_LINE_MAX		4096

/*
 * Parsed HTTP request structure.
 */
struct http_request {
	char	method[HTTP_METHOD_LEN];
	char	uri[HTTP_URI_LEN];
	char	version[HTTP_VER_LEN];
	char	raw_line[HTTP_LINE_MAX];
	time_t	if_modified_since;	/* -1 if header not present */
};

/*
 * Parse the first line of an HTTP request.
 * Returns 200 on success, or HTTP error status (400, 501).
 */
int	http_parse_request_line(const char *line, struct http_request *req);

/*
 * Check if the method is supported (GET or HEAD).
 */
int	http_is_supported_method(const char *method);

#endif /* MEMBER2_HTTP_H */
