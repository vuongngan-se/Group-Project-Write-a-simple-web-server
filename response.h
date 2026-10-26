/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 2: HTTP Response Subsystem Header
 */

#ifndef MEMBER2_RESPONSE_H
#define MEMBER2_RESPONSE_H

#include <sys/types.h>
#include <time.h>

/*
 * Retrieve the canonical reason phrase for an HTTP status code.
 */
const char	*http_status_reason(int status);

/*
 * Format a time_t value into RFC 1123 GMT string.
 */
void		 http_format_gmt_time(time_t t, char *buf, size_t buflen);

/*
 * Send a standard HTTP response header (and body if not HEAD).
 */
void		 http_send_headers(int fd, int status, const char *mime,
		    off_t size, time_t mtime);

/*
 * Send an HTML error response page.
 */
void		 http_send_error(int fd, int status, const char *msg,
		    int is_head);

/*
 * Stream contents of a file descriptor to a socket.
 */
void		 http_stream_fd(int src_fd, int dst_fd);

#endif /* MEMBER2_RESPONSE_H */
