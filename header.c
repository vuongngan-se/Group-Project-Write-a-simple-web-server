/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 2: HTTP Header Parsing Implementation
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include "header.h"

/*
 * Parse HTTP date according to RFC 1945 section 3.3.
 */
time_t
http_parse_date(const char *str)
{
	struct tm tm;
	char *res;

	if (str == NULL) {
		return (time_t)-1;
	}

	while (*str == ' ' || *str == '\t') {
		str++;
	}

	(void)memset(&tm, 0, sizeof(tm));

	/* Format 1: RFC 1123 / RFC 822: Sun, 06 Nov 1994 08:49:37 GMT */
	res = strptime(str, "%a, %d %b %Y %H:%M:%S GMT", &tm);
	if (res != NULL) {
		return timegm(&tm);
	}

	/* Format 2: RFC 850: Sunday, 06-Nov-94 08:49:37 GMT */
	(void)memset(&tm, 0, sizeof(tm));
	res = strptime(str, "%a, %d-%b-%y %H:%M:%S GMT", &tm);
	if (res != NULL) {
		return timegm(&tm);
	}

	/* Format 3: ANSI C asctime(): Sun Nov  6 08:49:37 1994 */
	(void)memset(&tm, 0, sizeof(tm));
	res = strptime(str, "%a %b %d %H:%M:%S %Y", &tm);
	if (res != NULL) {
		return timegm(&tm);
	}

	return (time_t)-1;
}

/*
 * Read HTTP headers line by line from the client socket.
 */
void
http_parse_headers(int client_fd, struct http_request *req)
{
	char line[HTTP_LINE_MAX];
	char ch;
	size_t idx;
	ssize_t n;

	idx = 0;
	while (1) {
		n = read(client_fd, &ch, 1);
		if (n <= 0) {
			break;
		}

		if (ch == '\r') {
			continue;
		}

		if (ch == '\n') {
			line[idx] = '\0';
			if (idx == 0) {
				/* Empty line marks the end of headers */
				break;
			}

			/* Check for If-Modified-Since header */
			if (strncasecmp(line, "If-Modified-Since:", 18) == 0) {
				req->if_modified_since =
				    http_parse_date(line + 18);
			}

			idx = 0;
			continue;
		}

		if (idx < sizeof(line) - 1) {
			line[idx++] = ch;
		}
	}
}
