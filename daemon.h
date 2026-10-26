/*
 * Copyright (c) 2026 vuongngan-se
 * All rights reserved.
 *
 * CS631 Advanced Programming in the UNIX Environment
 * Member 1: Daemon & Process Concurrency Subsystem Interface
 */

#ifndef DAEMON_H
#define DAEMON_H

#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

struct server_conf;

/* Global termination flag updated by signal handlers */
extern volatile sig_atomic_t g_running;

/*
 * Configure signal handlers for SIGCHLD, SIGTERM, SIGINT, SIGPIPE.
 */
void	daemon_setup_signals(void);

/*
 * Detach from controlling terminal and background if dflag is 0.
 */
void	daemon_init(const struct server_conf *conf);

/*
 * Dispatch incoming client connection:
 * - If dflag == 1: handles synchronously in foreground (one at a time).
 * - If dflag == 0: forks child process to handle connection concurrently.
 */
void	daemon_dispatch_connection(const struct server_conf *conf,
	    int client_fd, const struct sockaddr_storage *client_addr,
	    socklen_t addr_len, const int *listen_fds, int num_fds);

#endif /* DAEMON_H */
