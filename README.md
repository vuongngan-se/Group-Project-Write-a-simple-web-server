# Member 4: CGI Engine, Logging Subsystem & DevOps/QA

## Overview
This module manages external execution, access logging, and quality verification:
1. **CGI Engine (`cgi.c`, `cgi.h`)**:
   - RFC 3875 implementation for `/cgi-bin/...` endpoints.
   - Sets environment variables: `REQUEST_METHOD`, `QUERY_STRING`, `SCRIPT_NAME`, `PATH_INFO`, `SERVER_PROTOCOL`, `SERVER_SOFTWARE`, `REMOTE_ADDR`, `SERVER_PORT`.
   - Executes scripts using `fork(2)`, `pipe(2)`, `dup2(2)`, and `execl(2)`.
   - Relays CGI output to client socket.
2. **Logging Subsystem (`logger.c`, `logger.h`)**:
   - Implements Apache common log variation: `%a %t "%r" %>s %b`
   - Formats timestamps in GMT ISO 8601: `YYYY-MM-DDTHH:MM:SSZ`.
   - Appends safely to log file (`-l file`) or prints to `stdout` (`-d`).
3. **Automated Test Suite (`test_sws.sh`)**:
   - End-to-end regression tests verifying all required features (GET, HEAD, 404, 403, directory indexing, CGI, logging).
4. **Homework Checklist (`checklist`)**:
   - Verified checklist answering all grading requirements.

## API Usage Example
```c
/* Logging access */
struct log_entry entry;
entry.client_ip = "192.168.1.10";
entry.req_time = time(NULL);
entry.request_line = "GET /index.html HTTP/1.0";
entry.status_code = 200;
entry.bytes_sent = 1024;
logger_write(&entry);

/* CGI execution */
struct cgi_params params;
params.script_path = "/var/www/cgi-bin/hello.cgi";
params.script_name = "/cgi-bin/hello.cgi";
params.method = "GET";
params.query_string = "name=world";
params.client_ip = "127.0.0.1";
params.server_port = "8080";
cgi_execute(client_fd, &params);
```
