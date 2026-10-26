/*
 * Copyright (c) 2026 vuongngan-se
 * All rights reserved.
 *
 * CS631 Advanced Programming in the UNIX Environment
 * Member 1: Network Subsystem (Dual-stack IPv4/IPv6 socket handling)
 */

#include "sws.h"

/*
 * Initialize listening sockets for IPv4 and IPv6.
 */
int
net_init(const struct server_conf *conf, int *listen_fds, int max_fds)
{
	struct addrinfo hints, *res, *res0;
	char port_str[16];
	int count, error, fd, on, v6only;

	count = 0;
	on = 1;
	v6only = 1;

	(void)snprintf(port_str, sizeof(port_str), "%d", conf->port);

	(void)memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (conf->iflag == NULL) {
		hints.ai_flags = AI_PASSIVE;
	}

	error = getaddrinfo(conf->iflag, port_str, &hints, &res0);
	if (error != 0) {
		errx(EXIT_FAILURE, "getaddrinfo: %s", gai_strerror(error));
	}

	for (res = res0; res != NULL && count < max_fds; res = res->ai_next) {
		fd = socket(res->ai_family, res->ai_socktype,
		    res->ai_protocol);
		if (fd < 0) {
			continue;
		}

		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on,
		    (socklen_t)sizeof(on)) < 0) {
			warn("setsockopt SO_REUSEADDR");
			(void)close(fd);
			continue;
		}

		if (res->ai_family == AF_INET6) {
			if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &v6only,
			    (socklen_t)sizeof(v6only)) < 0) {
				warn("setsockopt IPV6_V6ONLY");
			}
		}

		if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
			(void)close(fd);
			continue;
		}

		if (listen(fd, BACKLOG) < 0) {
			warn("listen");
			(void)close(fd);
			continue;
		}

		listen_fds[count++] = fd;
	}

	freeaddrinfo(res0);

	if (count == 0) {
		errx(EXIT_FAILURE, "failed to bind to any listening address");
	}

	return count;
}

/*
 * Main event loop monitoring listening sockets using poll(2).
 */
void
net_serve(const struct server_conf *conf, const int *listen_fds, int num_fds)
{
	struct sockaddr_storage client_addr;
	struct pollfd pfd[MAX_LISTEN_FDS];
	socklen_t addr_len;
	int client_fd, i, ready;

	for (i = 0; i < num_fds; i++) {
		pfd[i].fd = listen_fds[i];
		pfd[i].events = POLLIN;
		pfd[i].revents = 0;
	}

	while (g_running) {
		ready = poll(pfd, (nfds_t)num_fds, 1000);
		if (ready < 0) {
			if (errno == EINTR) {
				continue;
			}
			warn("poll error");
			break;
		}
		if (ready == 0) {
			continue;
		}

		for (i = 0; i < num_fds; i++) {
			if ((pfd[i].revents & POLLIN) == 0) {
				continue;
			}

			addr_len = (socklen_t)sizeof(client_addr);
			client_fd = accept(pfd[i].fd,
			    (struct sockaddr *)&client_addr, &addr_len);
			if (client_fd < 0) {
				if (errno == EINTR || errno == ECONNABORTED) {
					continue;
				}
				warn("accept failed");
				continue;
			}

			daemon_dispatch_connection(conf, client_fd,
			    &client_addr, addr_len, listen_fds, num_fds);
		}
	}
}

/*
 * Close all active listening sockets.
 */
void
net_close_all(const int *listen_fds, int num_fds)
{
	int i;

	for (i = 0; i < num_fds; i++) {
		if (listen_fds[i] >= 0) {
			(void)close(listen_fds[i]);
		}
	}
}
