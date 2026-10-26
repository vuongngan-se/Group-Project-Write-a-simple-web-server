/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 2: HTTP Header Parser Interface
 */

#ifndef MEMBER2_HEADER_H
#define MEMBER2_HEADER_H

#include <time.h>
#include "http.h"

/*
 * Parse an HTTP date string into time_t (GMT).
 * Supports RFC 1123, RFC 850, and asctime() formats.
 * Returns (time_t)-1 on failure.
 */
time_t	http_parse_date(const char *str);

/*
 * Parse incoming HTTP headers from client socket until empty line.
 * Updates fields in struct http_request (e.g. if_modified_since).
 */
void	http_parse_headers(int client_fd, struct http_request *req);

#endif /* MEMBER2_HEADER_H */
