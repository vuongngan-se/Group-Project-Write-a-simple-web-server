/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 4: Logging Subsystem Interface
 */

#ifndef MEMBER4_LOGGER_H
#define MEMBER4_LOGGER_H

#include <sys/types.h>
#include <time.h>

/*
 * Log entry adhering to sws Apache common format:
 * '%a %t "%r" %>s %b'
 */
struct log_entry {
	const char	*client_ip;	/* %a: remote IP address */
	const char	*request_line;	/* %r: quoted first line of request */
	off_t		 bytes_sent;	/* %b: size in bytes or -1 for '-' */
	time_t		 req_time;	/* %t: time received in GMT */
	int		 status_code;	/* %>s: status of request */
};

/*
 * Initialize the logging subsystem with file and debug mode flags.
 * Returns 0 on success, -1 on failure.
 */
int	logger_init(const char *log_filepath, int debug_mode);

/*
 * Write an access log entry to file (or stdout if debug_mode is active).
 */
void	logger_write(const struct log_entry *entry);

/*
 * Close open log file descriptors.
 */
void	logger_cleanup(void);

#endif /* MEMBER4_LOGGER_H */
