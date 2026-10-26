/*
 * Copyright (c) 2026 vuongngan-se
 * All rights reserved.
 *
 * CS631 Advanced Programming in the UNIX Environment
 * Member 1: Main entry point & CLI parser for sws (Simple Web Server)
 */

#include "sws.h"

#if defined(__GNUC__)
static void	usage(void) __attribute__((__noreturn__));
#else
static void	usage(void);
#endif

/*
 * Portable implementation of strlcpy for systems lacking it.
 */
size_t
portable_strlcpy(char *dst, const char *src, size_t siz)
{
	const char *s;
	size_t n;

	s = src;
	n = siz;

	if (n != 0) {
		while (--n != 0) {
			if ((*dst++ = *s++) == '\0') {
				break;
			}
		}
	}

	if (n == 0) {
		if (siz != 0) {
			*dst = '\0';
		}
		while (*s++) {
			continue;
		}
	}

	return (size_t)(s - src - 1);
}

/*
 * Print usage message strictly adhering to NetBSD style guidelines.
 */
static void
usage(void)
{
	(void)fprintf(stderr,
	    "usage: %s [-dh] [-c dir] [-i address] [-l file] [-p port] dir\n",
	    getprogname());
	exit(EXIT_FAILURE);
}

/*
 * Read a single CRLF or LF terminated line from file descriptor.
 */
static int
read_line(int fd, char *buf, size_t maxlen)
{
	size_t idx;
	ssize_t n;
	char ch;

	idx = 0;
	while (idx + 1 < maxlen) {
		n = read(fd, &ch, 1);
		if (n <= 0) {
			break;
		}
		if (ch == '\r') {
			continue;
		}
		if (ch == '\n') {
			break;
		}
		buf[idx++] = ch;
	}
	buf[idx] = '\0';
	return (idx > 0 || n > 0) ? (int)idx : -1;
}

/*
 * Full HTTP request processing pipeline integrating all member modules.
 */
void
handle_client(int client_fd, const struct sockaddr_storage *client_addr,
    socklen_t addr_len, const struct server_conf *conf)
{
	struct http_request req;
	struct resource_info res;
	struct log_entry entry;
	char host[NI_MAXHOST], serv[NI_MAXSERV], req_line[HTTP_LINE_MAX];
	char idx_path[PATH_MAX], *html_buf;
	const char *mime;
	size_t html_len;
	off_t bytes_sent;
	time_t now;
	int error, file_fd, is_head, status;

	now = time(NULL);
	html_buf = NULL;
	bytes_sent = -1;

	error = getnameinfo((const struct sockaddr *)client_addr, addr_len,
	    host, sizeof(host), serv, sizeof(serv),
	    NI_NUMERICHOST | NI_NUMERICSERV);
	if (error != 0) {
		(void)snprintf(host, sizeof(host), "unknown");
	}

	if (read_line(client_fd, req_line, sizeof(req_line)) <= 0) {
		return;
	}

	status = http_parse_request_line(req_line, &req);
	if (status != 200) {
		http_send_error(client_fd, status, NULL, 0);
		entry.client_ip = host;
		entry.request_line = req_line;
		entry.status_code = status;
		entry.bytes_sent = -1;
		entry.req_time = now;
		logger_write(&entry);
		return;
	}

	is_head = (strcmp(req.method, "HEAD") == 0);

	http_parse_headers(client_fd, &req);

	status = uri_resolve(req.uri, conf->docroot, conf->cflag, &res);
	if (status != 200) {
		http_send_error(client_fd, status, NULL, is_head);
		entry.client_ip = host;
		entry.request_line = req.raw_line;
		entry.status_code = status;
		entry.bytes_sent = -1;
		entry.req_time = now;
		logger_write(&entry);
		return;
	}

	/* CGI execution */
	if (res.resource_type == SWS_TYPE_CGI) {
		struct cgi_params cgi;
		char port_str[16];
		char *q;

		(void)snprintf(port_str, sizeof(port_str), "%d", conf->port);
		q = strchr(req.uri, '?');
		cgi.script_path = res.resolved_path;
		cgi.script_name = req.uri;
		cgi.method = req.method;
		cgi.query_string = (q != NULL) ? q + 1 : "";
		cgi.client_ip = host;
		cgi.server_port = port_str;

		if (cgi_execute(client_fd, &cgi) < 0) {
			http_send_error(client_fd, 500, "CGI failed", is_head);
			status = 500;
		}
		entry.client_ip = host;
		entry.request_line = req.raw_line;
		entry.status_code = status;
		entry.bytes_sent = -1;
		entry.req_time = now;
		logger_write(&entry);
		return;
	}

	/* Directory listing or index.html */
	if (res.resource_type == SWS_TYPE_DIR) {
		if (dir_has_index_file(res.resolved_path, idx_path,
		    sizeof(idx_path))) {
			(void)snprintf(res.resolved_path,
			    sizeof(res.resolved_path), "%s", idx_path);
			if (stat(res.resolved_path, &res.sb) < 0) {
				http_send_error(client_fd, 404, NULL, is_head);
				return;
			}
			res.resource_type = SWS_TYPE_FILE;
		} else {
			if (dir_generate_index_html(res.resolved_path, req.uri,
			    &html_buf, &html_len) < 0) {
				http_send_error(client_fd, 500, NULL, is_head);
				return;
			}
			http_send_headers(client_fd, 200, "text/html",
			    (off_t)html_len, now);
			if (!is_head) {
				if (write(client_fd, html_buf, html_len) < 0) {
					warn("write html_buf");
				}
			}
			bytes_sent = (off_t)html_len;
			free(html_buf);

			entry.client_ip = host;
			entry.request_line = req.raw_line;
			entry.status_code = 200;
			entry.bytes_sent = bytes_sent;
			entry.req_time = now;
			logger_write(&entry);
			return;
		}
	}

	/* Regular file serving */
	if (res.resource_type == SWS_TYPE_FILE) {
		if (req.if_modified_since != (time_t)-1 &&
		    res.sb.st_mtime <= req.if_modified_since) {
			http_send_headers(client_fd, 304, NULL, 0,
			    res.sb.st_mtime);
			entry.client_ip = host;
			entry.request_line = req.raw_line;
			entry.status_code = 304;
			entry.bytes_sent = 0;
			entry.req_time = now;
			logger_write(&entry);
			return;
		}

		file_fd = open(res.resolved_path, O_RDONLY);
		if (file_fd < 0) {
			http_send_error(client_fd,
			    (errno == EACCES) ? 403 : 404, NULL, is_head);
			return;
		}

		mime = mime_get_type(res.resolved_path);
		http_send_headers(client_fd, 200, mime, res.sb.st_size,
		    res.sb.st_mtime);
		if (!is_head) {
			http_stream_fd(file_fd, client_fd);
		}
		(void)close(file_fd);
		bytes_sent = res.sb.st_size;

		entry.client_ip = host;
		entry.request_line = req.raw_line;
		entry.status_code = 200;
		entry.bytes_sent = bytes_sent;
		entry.req_time = now;
		logger_write(&entry);
		return;
	}
}

/*
 * Program entry point.
 */
int
main(int argc, char *argv[])
{
	struct server_conf conf;
	struct stat sb;
	char *ep;
	long port_num;
	int ch, listen_fds[MAX_LISTEN_FDS], num_fds;

	setprogname(argv[0]);

	(void)memset(&conf, 0, sizeof(conf));
	conf.port = DEFAULT_PORT;

	while ((ch = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
		switch (ch) {
		case 'c':
			conf.cflag = optarg;
			break;
		case 'd':
			conf.dflag = 1;
			break;
		case 'h':
			usage();
			break;
		case 'i':
			conf.iflag = optarg;
			break;
		case 'l':
			conf.lflag = optarg;
			break;
		case 'p':
			errno = 0;
			port_num = strtol(optarg, &ep, 10);
			if (port_num <= 0 || port_num > 65535 || *ep != '\0' ||
			    (errno == ERANGE && (port_num == LONG_MAX ||
			    port_num == LONG_MIN))) {
				errx(EXIT_FAILURE,
				    "illegal port number -- %s", optarg);
			}
			conf.port = (int)port_num;
			break;
		case '?':
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1) {
		usage();
	}

	/* Validate docroot directory */
	if (realpath(argv[0], conf.docroot) == NULL) {
		err(EXIT_FAILURE, "docroot '%s'", argv[0]);
	}
	if (stat(conf.docroot, &sb) < 0) {
		err(EXIT_FAILURE, "stat '%s'", conf.docroot);
	}
	if (!S_ISDIR(sb.st_mode)) {
		errx(EXIT_FAILURE, "'%s' is not a directory", conf.docroot);
	}

	/* Validate CGI directory if specified */
	if (conf.cflag != NULL) {
		if (stat(conf.cflag, &sb) < 0) {
			err(EXIT_FAILURE, "cgi directory '%s'", conf.cflag);
		}
		if (!S_ISDIR(sb.st_mode)) {
			errx(EXIT_FAILURE, "'%s' is not a directory",
			    conf.cflag);
		}
	}

	if (logger_init(conf.lflag, conf.dflag) < 0) {
		warn("logger_init failed");
	}

	(void)mime_init();

	daemon_setup_signals();

	num_fds = net_init(&conf, listen_fds, MAX_LISTEN_FDS);

	daemon_init(&conf);

	net_serve(&conf, listen_fds, num_fds);

	net_close_all(listen_fds, num_fds);

	mime_cleanup();
	logger_cleanup();

	return EXIT_SUCCESS;
}
