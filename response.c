/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 2: HTTP Response Subsystem Implementation
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <err.h>

#include "response.h"

#define SWS_SERVER_STR	"sws/1.0"
#define CHUNK_SIZE	4096

/*
 * Map HTTP status code to RFC 1945 reason phrases.
 */
const char *
http_status_reason(int status)
{
	switch (status) {
	case 200:
		return "OK";
	case 304:
		return "Not Modified";
	case 400:
		return "Bad Request";
	case 403:
		return "Forbidden";
	case 404:
		return "Not Found";
	case 500:
		return "Internal Server Error";
	case 501:
		return "Not Implemented";
	default:
		return "Unknown";
	}
}

/*
 * Format GMT time according to RFC 1123.
 */
void
http_format_gmt_time(time_t t, char *buf, size_t buflen)
{
	struct tm *tm;

	tm = gmtime(&t);
	if (tm == NULL) {
		(void)snprintf(buf, buflen, "Thu, 01 Jan 1970 00:00:00 GMT");
		return;
	}
	(void)strftime(buf, buflen, "%a, %d %b %Y %H:%M:%S GMT", tm);
}

/*
 * Send standard HTTP/1.0 response headers to client.
 */
void
http_send_headers(int fd, int status, const char *mime, off_t size,
    time_t mtime)
{
	char date_str[64], mtime_str[64], header_buf[1024];
	time_t now;
	size_t len;

	now = time(NULL);
	http_format_gmt_time(now, date_str, sizeof(date_str));

	len = (size_t)snprintf(header_buf, sizeof(header_buf),
	    "HTTP/1.0 %d %s\r\n"
	    "Date: %s\r\n"
	    "Server: %s\r\n"
	    "Connection: close\r\n",
	    status, http_status_reason(status), date_str, SWS_SERVER_STR);

	if (status != 304) {
		if (mtime > 0) {
			http_format_gmt_time(mtime, mtime_str,
			    sizeof(mtime_str));
			len += (size_t)snprintf(header_buf + len,
			    sizeof(header_buf) - len,
			    "Last-Modified: %s\r\n", mtime_str);
		}
		if (mime != NULL) {
			len += (size_t)snprintf(header_buf + len,
			    sizeof(header_buf) - len,
			    "Content-Type: %s\r\n", mime);
		}
		if (size >= 0) {
			len += (size_t)snprintf(header_buf + len,
			    sizeof(header_buf) - len,
			    "Content-Length: %jd\r\n", (intmax_t)size);
		}
	}

	(void)snprintf(header_buf + len, sizeof(header_buf) - len, "\r\n");
	if (write(fd, header_buf, strlen(header_buf)) < 0) {
		warn("write response headers");
	}
}

/*
 * Send an HTML error response.
 */
void
http_send_error(int fd, int status, const char *msg, int is_head)
{
	char body[1024];
	size_t body_len;

	body_len = (size_t)snprintf(body, sizeof(body),
	    "<!DOCTYPE html>\n"
	    "<html>\n"
	    "<head><title>%d %s</title></head>\n"
	    "<body>\n"
	    "<h1>%d %s</h1>\n"
	    "<p>%s</p>\n"
	    "<hr>\n"
	    "<i>%s</i>\n"
	    "</body>\n"
	    "</html>\n",
	    status, http_status_reason(status),
	    status, http_status_reason(status),
	    (msg != NULL) ? msg : http_status_reason(status),
	    SWS_SERVER_STR);

	http_send_headers(fd, status, "text/html", (off_t)body_len, 0);

	if (!is_head) {
		if (write(fd, body, body_len) < 0) {
			warn("write error body");
		}
	}
}

/*
 * Stream source file content to client socket in chunks.
 */
void
http_stream_fd(int src_fd, int dst_fd)
{
	char buf[CHUNK_SIZE];
	ssize_t nread, nwritten, total;

	while ((nread = read(src_fd, buf, sizeof(buf))) > 0) {
		total = 0;
		while (total < nread) {
			nwritten = write(dst_fd, buf + total,
			    (size_t)(nread - total));
			if (nwritten <= 0) {
				return;
			}
			total += nwritten;
		}
	}
}
