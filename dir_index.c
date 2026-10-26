/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 3: Directory Indexing Implementation (Alphanumeric HTML Generator)
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "dir_index.h"

/*
 * Comparator for alphanumeric sorting of directory entries.
 */
static int
entry_cmp(const void *a, const void *b)
{
	const char *const *ia = (const char *const *)a;
	const char *const *ib = (const char *const *)b;

	return strcmp(*ia, *ib);
}

/*
 * Check if directory contains an "index.html" file.
 */
int
dir_has_index_file(const char *dir_path, char *out_index_path, size_t max_len)
{
	struct stat sb;
	char candidate[PATH_MAX];

	(void)snprintf(candidate, sizeof(candidate), "%s/index.html",
	    dir_path);
	if (stat(candidate, &sb) == 0 && S_ISREG(sb.st_mode)) {
		if (out_index_path != NULL) {
			(void)snprintf(out_index_path, max_len, "%s",
			    candidate);
		}
		return 1;
	}
	return 0;
}

/*
 * Generate HTML directory listing in alphanumeric order, ignoring dotfiles.
 */
int
dir_generate_index_html(const char *dir_path, const char *req_uri,
    char **html_out, size_t *html_len)
{
	DIR *dir;
	struct dirent *dp;
	char **entries;
	char *buf;
	size_t cap, count, i, len, needed;

	if (dir_path == NULL || req_uri == NULL || html_out == NULL) {
		return -1;
	}

	dir = opendir(dir_path);
	if (dir == NULL) {
		return -1;
	}

	cap = 64;
	count = 0;
	entries = malloc(cap * sizeof(char *));
	if (entries == NULL) {
		(void)closedir(dir);
		return -1;
	}

	while ((dp = readdir(dir)) != NULL) {
		/* Ignore files starting with a '.' */
		if (dp->d_name[0] == '.') {
			continue;
		}

		if (count >= cap) {
			char **tmp;
			cap *= 2;
			tmp = realloc(entries, cap * sizeof(char *));
			if (tmp == NULL) {
				for (i = 0; i < count; i++) {
					free(entries[i]);
				}
				free(entries);
				(void)closedir(dir);
				return -1;
			}
			entries = tmp;
		}

		entries[count] = strdup(dp->d_name);
		if (entries[count] == NULL) {
			for (i = 0; i < count; i++) {
				free(entries[i]);
			}
			free(entries);
			(void)closedir(dir);
			return -1;
		}
		count++;
	}
	(void)closedir(dir);

	/* Sort entries alphanumerically */
	qsort(entries, count, sizeof(char *), entry_cmp);

	/* Estimate buffer size and allocate */
	needed = 4096 + (count * 256);
	buf = malloc(needed);
	if (buf == NULL) {
		for (i = 0; i < count; i++) {
			free(entries[i]);
		}
		free(entries);
		return -1;
	}

	len = (size_t)snprintf(buf, needed,
	    "<!DOCTYPE html>\n"
	    "<html>\n"
	    "<head><title>Index of %s</title></head>\n"
	    "<body>\n"
	    "<h1>Index of %s</h1>\n"
	    "<hr>\n"
	    "<ul>\n",
	    req_uri, req_uri);

	for (i = 0; i < count; i++) {
		len += (size_t)snprintf(buf + len, needed - len,
		    "  <li><a href=\"%s%s%s\">%s</a></li>\n",
		    req_uri,
		    (req_uri[strlen(req_uri) - 1] == '/') ? "" : "/",
		    entries[i], entries[i]);
		free(entries[i]);
	}
	free(entries);

	len += (size_t)snprintf(buf + len, needed - len,
	    "</ul>\n"
	    "<hr>\n"
	    "<i>sws/1.0 Server</i>\n"
	    "</body>\n"
	    "</html>\n");

	*html_out = buf;
	if (html_len != NULL) {
		*html_len = len;
	}

	return 0;
}
