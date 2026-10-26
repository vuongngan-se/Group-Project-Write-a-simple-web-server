/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 4: Logging Subsystem Implementation
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "logger.h"

static int g_log_fd = -1;
static int g_debug_mode = 0;

/*
 * Initialize logging subsystem.
 */
int
logger_init(const char *log_filepath, int debug_mode)
{
	g_debug_mode = debug_mode;

	if (log_filepath != NULL) {
		g_log_fd = open(log_filepath,
		    O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (g_log_fd < 0) {
			return -1;
		}
	}

	return 0;
}

/*
 * Write log entry: '%a %t "%r" %>s %b'
 */
void
logger_write(const struct log_entry *entry)
{
	struct tm *tm;
	char time_buf[64], line_buf[2048];
	size_t len;

	if (entry == NULL) {
		return;
	}

	/* Format time as ISO 8601 GMT: YYYY-MM-DDTHH:MM:SSZ */
	tm = gmtime(&entry->req_time);
	if (tm != NULL) {
		(void)strftime(time_buf, sizeof(time_buf),
		    "%Y-%m-%dT%H:%M:%SZ", tm);
	} else {
		(void)snprintf(time_buf, sizeof(time_buf),
		    "1970-01-01T00:00:00Z");
	}

	if (entry->bytes_sent >= 0) {
		len = (size_t)snprintf(line_buf, sizeof(line_buf),
		    "%s %s \"%s\" %d %jd\n",
		    (entry->client_ip != NULL) ? entry->client_ip : "-",
		    time_buf,
		    (entry->request_line != NULL) ? entry->request_line : "-",
		    entry->status_code,
		    (intmax_t)entry->bytes_sent);
	} else {
		len = (size_t)snprintf(line_buf, sizeof(line_buf),
		    "%s %s \"%s\" %d -\n",
		    (entry->client_ip != NULL) ? entry->client_ip : "-",
		    time_buf,
		    (entry->request_line != NULL) ? entry->request_line : "-",
		    entry->status_code);
	}

	if (g_debug_mode) {
		(void)write(STDOUT_FILENO, line_buf, len);
	}

	if (g_log_fd >= 0) {
		(void)write(g_log_fd, line_buf, len);
	}
}

/*
 * Close log file descriptor.
 */
void
logger_cleanup(void)
{
	if (g_log_fd >= 0) {
		(void)close(g_log_fd);
		g_log_fd = -1;
	}
}
