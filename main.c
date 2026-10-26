/*
 * Copyright (c) 2026 vuongngan-se
 * All rights reserved.
 *
 * CS631 Advanced Programming in the UNIX Environment
 * Member 1: Main entry point & CLI parser for sws (Simple Web Server)
 */

#include "sws.h"

static void	usage(void);

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
 * Baseline connection handler for Member 1 testing.
 * Can be plugged into by Members 2, 3, and 4.
 */
void
handle_client(int client_fd, const struct sockaddr_storage *client_addr,
    socklen_t addr_len, const struct server_conf *conf)
{
	char host[NI_MAXHOST], serv[NI_MAXSERV];
	char buf[REQ_BUF_SIZE], resp[REQ_BUF_SIZE];
	ssize_t nread, nwritten;
	int error;

	(void)conf;

	error = getnameinfo((const struct sockaddr *)client_addr, addr_len,
	    host, sizeof(host), serv, sizeof(serv),
	    NI_NUMERICHOST | NI_NUMERICSERV);
	if (error != 0) {
		(void)snprintf(host, sizeof(host), "unknown");
		(void)snprintf(serv, sizeof(serv), "0");
	}

	nread = read(client_fd, buf, sizeof(buf) - 1);
	if (nread <= 0) {
		return;
	}
	buf[nread] = '\0';

	if (conf->dflag != 0) {
		(void)printf("[debug] client %s:%s connected\n", host, serv);
		(void)fflush(stdout);
	}

	/* Initial HTTP/1.0 baseline response */
	(void)snprintf(resp, sizeof(resp),
	    "HTTP/1.0 200 OK\r\n"
	    "Server: %s\r\n"
	    "Content-Type: text/plain\r\n"
	    "Connection: close\r\n"
	    "\r\n"
	    "sws daemon running.\r\n"
	    "Docroot: %s\r\n",
	    SWS_VERSION, conf->docroot);

	nwritten = write(client_fd, resp, strlen(resp));
	if (nwritten < 0) {
		warn("write to client failed");
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
			/* NOTREACHED */
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
			/* NOTREACHED */
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

	daemon_setup_signals();

	num_fds = net_init(&conf, listen_fds, MAX_LISTEN_FDS);

	daemon_init(&conf);

	net_serve(&conf, listen_fds, num_fds);

	net_close_all(listen_fds, num_fds);

	return EXIT_SUCCESS;
}
