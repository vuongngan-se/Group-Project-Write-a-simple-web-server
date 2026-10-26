# Member 3: VFS, Security, User Directories & Directory Indexing

## Overview
This module handles all URI-to-filesystem translation and features required by `sws(1)`:
1. **URI Resolution & Security (`uri.c`, `uri.h`)**:
   - Resolves relative URIs against document root (`docroot`).
   - Translates `/~user/` to `/home/<user>/sws/` via `getpwnam(3)`.
   - Protects against Path Traversal (`../` escape attacks), returning `403 Forbidden`.
   - Returns resource classification (`SWS_TYPE_FILE`, `SWS_TYPE_DIR`, `SWS_TYPE_CGI`).
2. **Directory Indexing (`dir_index.c`, `dir_index.h`)**:
   - Checks if `index.html` exists inside requested directory.
   - If missing, generates an HTML directory index.
   - Sorts directory entries in **alphanumeric order** using `qsort`.
   - Ignores all dotfiles (entries starting with `.`).
3. **MIME Detection (`mime.c`, `mime.h`)**:
   - Integrates with `libmagic` (`magic(5)`) for accurate `Content-Type` detection.
   - Includes reliable fallback extension mapping.

## API Usage Example
```c
struct resource_info res;
int status = uri_resolve(req.uri, conf.docroot, conf.cflag, &res);
if (status == 200) {
    if (res.resource_type == SWS_TYPE_DIR) {
        char index_path[PATH_MAX];
        if (dir_has_index_file(res.resolved_path, index_path, sizeof(index_path))) {
            /* Serve index.html */
        } else {
            char *html = NULL;
            size_t len = 0;
            dir_generate_index_html(res.resolved_path, req.uri, &html, &len);
            /* Send HTML index and free(html) */
        }
    }
}
```
