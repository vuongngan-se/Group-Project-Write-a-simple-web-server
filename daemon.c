/*
 * Copyright (c) 2026 vuongngan-se
 * All rights reserved.
 *
 * CS631 Advanced Programming in the UNIX Environment
 * Member 1: Daemonization & Process Concurrency Subsystem
 */

#include "sws.h"

volatile sig_atomic_t g_running = 1;

static void	sigchld_handler(int sig);
static void	sigterm_handler(int sig);

/*
 * Signal handler to reap terminated child processes.
 */
static void
sigchld_handler(int sig)
{
	pid_t pid;
	int saved_errno, status;

	(void)sig;
	saved_errno = errno;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		continue;
	}
	errno = saved_errno;
}

/*
 * Signal handler to request graceful server termination.
 */
static void
sigterm_handler(int sig)
{
	(void)sig;
	g_running = 0;
}

/*
 * Set up signal handlers according to APUE conventions.
 */
void
daemon_setup_signals(void)
{
	struct sigaction sa;

	(void)memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sigchld_handler;
	(void)sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		err(EXIT_FAILURE, "sigaction SIGCHLD");
	}

	sa.sa_handler = sigterm_handler;
	(void)sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGTERM, &sa, NULL) < 0) {
		err(EXIT_FAILURE, "sigaction SIGTERM");
	}
	if (sigaction(SIGINT, &sa, NULL) < 0) {
		err(EXIT_FAILURE, "sigaction SIGINT");
	}

	sa.sa_handler = SIG_IGN;
	(void)sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGPIPE, &sa, NULL) < 0) {
		err(EXIT_FAILURE, "sigaction SIGPIPE");
	}
}

/*
 * Detach process from terminal and run as background daemon.
 */
void
daemon_init(const struct server_conf *conf)
{
	pid_t pid;
	int devnull;

	if (conf->dflag != 0) {
		(void)printf("sws: running in debug mode (single-connection, "
		    "foreground)\n");
		(void)fflush(stdout);
		return;
	}

	/* First fork */
	pid = fork();
	if (pid < 0) {
		err(EXIT_FAILURE, "fork");
	}
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* Create new session */
	if (setsid() < 0) {
		err(EXIT_FAILURE, "setsid");
	}

	/* Second fork to ensure daemon cannot acquire controlling terminal */
	pid = fork();
	if (pid < 0) {
		err(EXIT_FAILURE, "fork");
	}
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	(void)umask(022);

	/* Redirect standard descriptors to /dev/null */
	devnull = open("/dev/null", O_RDWR);
	if (devnull >= 0) {
		if (dup2(devnull, STDIN_FILENO) < 0 ||
		    dup2(devnull, STDOUT_FILENO) < 0 ||
		    dup2(devnull, STDERR_FILENO) < 0) {
			warn("dup2 /dev/null");
		}
		if (devnull > STDERR_FILENO) {
			(void)close(devnull);
		}
	}
}

/*
 * Dispatch incoming connection based on debugging or daemon mode.
 */
void
daemon_dispatch_connection(const struct server_conf *conf, int client_fd,
    const struct sockaddr_storage *client_addr, socklen_t addr_len,
    const int *listen_fds, int num_fds)
{
	pid_t pid;

	if (conf->dflag != 0) {
		/* Debug mode: handle connection sequentially in foreground */
		handle_client(client_fd, client_addr, addr_len, conf);
		(void)close(client_fd);
		return;
	}

	/* Normal daemon mode: handle connection concurrently in child */
	pid = fork();
	if (pid < 0) {
		warn("fork client connection");
		(void)close(client_fd);
		return;
	}

	if (pid == 0) {
		/* Child process */
		net_close_all(listen_fds, num_fds);
		handle_client(client_fd, client_addr, addr_len, conf);
		(void)close(client_fd);
		exit(EXIT_SUCCESS);
	}

	/* Parent process */
	(void)close(client_fd);
}
