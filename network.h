/*
 * Copyright (c) 2026 vuongngan-se
 * All rights reserved.
 *
 * CS631 Advanced Programming in the UNIX Environment
 * Member 1: Network Subsystem Interface
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <sys/types.h>
#include <sys/socket.h>

struct server_conf;

/*
 * Initialize listening sockets for IPv4 and/or IPv6.
 * Returns number of listening sockets opened, or exits on fatal error.
 */
int	net_init(const struct server_conf *conf, int *listen_fds, int max_fds);

/*
 * Main event loop monitoring listening sockets using poll(2).
 */
void	net_serve(const struct server_conf *conf, const int *listen_fds,
	    int num_fds);

/*
 * Close all listening sockets cleanly upon shutdown.
 */
void	net_close_all(const int *listen_fds, int num_fds);

#endif /* NETWORK_H */
