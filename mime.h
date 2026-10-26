/*
 * Copyright (c) 2026 CS631 Group Project
 * Member 3: MIME Type Detection Interface (magic(5) integration)
 */

#ifndef MEMBER3_MIME_H
#define MEMBER3_MIME_H

/*
 * Initialize the libmagic database.
 * Returns 0 on success, -1 on failure.
 */
int		mime_init(void);

/*
 * Detect MIME type for a file path using magic(5) patterns.
 * Returns a static or library-managed string (e.g. "text/html").
 */
const char	*mime_get_type(const char *filepath);

/*
 * Clean up libmagic database handle.
 */
void		mime_cleanup(void);

#endif /* MEMBER3_MIME_H */
