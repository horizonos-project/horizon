#include "vfs.h"
#include "file.h"
#include "libk/string.h"

// Mount table (for now, just root)
static fs_ops_t *root_fs = NULL;

// Registered filesystems
#define MAX_FS_TYPES 8
static fs_ops_t *registered_fs[MAX_FS_TYPES];
static int num_fs_types = 0;

int vfs_init(void) {
    fd_table_init();
    num_fs_types = 0;
    root_fs = NULL;
    return 0;
}

int vfs_register_fs(fs_ops_t *ops) {
    if (num_fs_types >= MAX_FS_TYPES) return -1;
    registered_fs[num_fs_types++] = ops;
    return 0;
}

int vfs_mount(const char *fs_name, const char *device, const char *mountpoint) {
    // Find filesystem by name
    fs_ops_t *fs = NULL;
    for (int i = 0; i < num_fs_types; i++) {
        if (strcmp(registered_fs[i]->name, fs_name) == 0) {
            fs = registered_fs[i];
            break;
        }
    }
    
    if (!fs) return -1;  // FS not found
    
    // For now, only support root mount
    if (strcmp(mountpoint, "/") != 0) return -1;
    
    if (fs->mount && fs->mount(device) < 0) return -1;
    
    root_fs = fs;
    return 0;
}

int vfs_open(const char *path, int flags) {
    if (!root_fs || !root_fs->open) return -1;
    
    file_t *file;
    int fd = fd_alloc(&file);
    if (fd < 0) return -1;
    
    file->flags = flags;
    file->offset = 0;
    file->fs_ops = root_fs;
    
    if (root_fs->open(path, flags, file) < 0) {
        fd_free(fd);
        return -1;
    }
    
    return fd;
}

int vfs_close(int fd) {
    file_t *file = fd_get(fd);
    if (!file) return -1;
    
    if (file->fs_ops->close) {
        file->fs_ops->close(file);
    }
    
    fd_free(fd);
    return 0;
}

int vfs_read(int fd, void *buf, size_t count) {
    file_t *file = fd_get(fd);
    if (!file || !file->fs_ops->read) return -1;
    
    return file->fs_ops->read(file, buf, count);
}

int vfs_stat(const char *path, stat_t *st) {
    if (!root_fs || !root_fs->stat) return -1;
    return root_fs->stat(path, st);
}