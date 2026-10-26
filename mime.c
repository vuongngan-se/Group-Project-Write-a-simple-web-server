/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 3: MIME Type Detection Implementation using libmagic (magic(5))
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <magic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "mime.h"

static magic_t g_magic = NULL;

/*
 * Fallback MIME mapping based on common file extensions.
 */
static const char *
fallback_mime(const char *path)
{
	const char *dot;

	dot = strrchr(path, '.');
	if (dot == NULL) {
		return "application/octet-stream";
	}

	if (strcasecmp(dot, ".html") == 0 || strcasecmp(dot, ".htm") == 0) {
		return "text/html";
	}
	if (strcasecmp(dot, ".css") == 0) {
		return "text/css";
	}
	if (strcasecmp(dot, ".js") == 0) {
		return "application/javascript";
	}
	if (strcasecmp(dot, ".txt") == 0 || strcasecmp(dot, ".c") == 0 ||
	    strcasecmp(dot, ".h") == 0) {
		return "text/plain";
	}
	if (strcasecmp(dot, ".png") == 0) {
		return "image/png";
	}
	if (strcasecmp(dot, ".jpg") == 0 || strcasecmp(dot, ".jpeg") == 0) {
		return "image/jpeg";
	}
	if (strcasecmp(dot, ".gif") == 0) {
		return "image/gif";
	}
	if (strcasecmp(dot, ".pdf") == 0) {
		return "application/pdf";
	}

	return "application/octet-stream";
}

/*
 * Initialize libmagic database cookie.
 */
int
mime_init(void)
{
	if (g_magic != NULL) {
		return 0;
	}

	g_magic = magic_open(MAGIC_MIME_TYPE);
	if (g_magic == NULL) {
		return -1;
	}

	if (magic_load(g_magic, NULL) != 0) {
		magic_close(g_magic);
		g_magic = NULL;
		return -1;
	}

	return 0;
}

/*
 * Determine Content-Type header value via magic(5) patterns.
 */
const char *
mime_get_type(const char *filepath)
{
	const char *res;

	if (filepath == NULL) {
		return "application/octet-stream";
	}

	if (g_magic != NULL) {
		res = magic_file(g_magic, filepath);
		if (res != NULL && strlen(res) > 0) {
			return res;
		}
	}

	return fallback_mime(filepath);
}

/*
 * Close libmagic database.
 */
void
mime_cleanup(void)
{
	if (g_magic != NULL) {
		magic_close(g_magic);
		g_magic = NULL;
	}
}
