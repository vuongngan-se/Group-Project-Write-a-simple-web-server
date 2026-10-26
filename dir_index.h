/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 3: Directory Indexing Subsystem Header
 */

#ifndef MEMBER3_DIR_INDEX_H
#define MEMBER3_DIR_INDEX_H

#include <sys/types.h>
#include <stddef.h>

/*
 * Check if the given directory contains an "index.html" file.
 * Returns 1 if found and fills out_index_path, 0 otherwise.
 */
int	dir_has_index_file(const char *dir_path, char *out_index_path,
	    size_t max_len);

/*
 * Generate an HTML directory listing sorted in alphanumeric order.
 * Files starting with '.' are ignored.
 * The generated HTML string must be freed by the caller.
 * Returns 0 on success, -1 on failure.
 */
int	dir_generate_index_html(const char *dir_path, const char *req_uri,
	    char **html_out, size_t *html_len);

#endif /* MEMBER3_DIR_INDEX_H */
