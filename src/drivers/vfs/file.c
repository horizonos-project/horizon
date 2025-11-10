#include "file.h"

static file_t fd_table[VFS_MAX_FDS];

void fd_table_init(void) {
    for (int i = 0; i < VFS_MAX_FDS; i++) {
        fd_table[i].in_use = false;
    }
}

int fd_alloc(file_t **out) {
    for (int i = 0; i < VFS_MAX_FDS; i++) {
        if (!fd_table[i].in_use) {
            fd_table[i].in_use = true;
            *out = &fd_table[i];
            return i;
        }
    }
    return -1;  // Out of FDs
}

void fd_free(int fd) {
    if (fd >= 0 && fd < VFS_MAX_FDS) {
        fd_table[fd].in_use = false;
    }
}

file_t* fd_get(int fd) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !fd_table[fd].in_use) {
        return NULL;
    }
    return &fd_table[fd];
}