# Member 2: HTTP Protocol Engine & Response Builder

## Overview
This module handles all HTTP/1.0 protocol processing as defined in RFC 1945, including:
1. **Request Line Parsing (`http.c`, `http.h`)**:
   - Parses Method, Request-URI, and HTTP-Version.
   - Restricts to `GET` and `HEAD` (returns 501 Not Implemented for others).
   - Validates relative URIs (RFC 1945 Section 3.2.1).
   - Quietly downgrades `HTTP/1.1` requests to `HTTP/1.0`.
2. **Header Parsing (`header.c`, `header.h`)**:
   - Parses `If-Modified-Since` request header.
   - Supports 3 date formats specified in RFC 1945 Section 3.3 (RFC 1123, RFC 850, asctime).
3. **Response Generation (`response.c`, `response.h`)**:
   - Generates standard RFC 1945 status codes (`200 OK`, `304 Not Modified`, `400 Bad Request`, `403 Forbidden`, `404 Not Found`, `500 Internal Server Error`, `501 Not Implemented`).
   - Automatically builds required HTTP response headers: `Date` (GMT), `Server` (`sws/1.0`), `Last-Modified` (GMT), `Content-Type`, `Content-Length`.
   - Generates clean HTML error pages for errors.
   - Streams file bodies in 4KB chunks for `GET`, and omits the body for `HEAD`.

## API Usage Example
```c
struct http_request req;
int status = http_parse_request_line(raw_line, &req);
if (status == 200) {
    http_parse_headers(client_fd, &req);
    /* Process file and send response */
    http_send_headers(client_fd, 200, "text/html", file_size, file_mtime);
    if (strcmp(req.method, "GET") == 0) {
        http_stream_fd(file_fd, client_fd);
    }
} else {
    http_send_error(client_fd, status, NULL, 0);
}
```
