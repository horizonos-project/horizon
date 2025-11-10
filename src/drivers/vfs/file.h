#ifndef VFS_FILE_H
#define VFS_FILE_H

#include "vfs.h"

// FD table management
void fd_table_init(void);
int fd_alloc(file_t **out);
void fd_free(int fd);
file_t* fd_get(int fd);

#endif // VFS_FILE_H