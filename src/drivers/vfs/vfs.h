#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define VFS_FILE    0x01
#define VFS_DIR     0x02
#define VFS_SYMLINK 0x03

#define O_RDONLY    0x00
#define O_WRONLY    0x01
#define O_RDWR      0x02

#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

#define VFS_MAX_FDS 256

typedef struct fs_ops fs_ops_t;

typedef struct file {
    bool in_use;
    uint32_t offset;
    int flags;

    void *fs_data;      // FS-specific data
    fs_ops_t *fs_ops;   // Who owns this?
} file_t;

typedef struct dirent dirent_t;

// VFS Directory entry
struct dirent {
    uint32_t inode;
    uint8_t type; // file, dir, etc..
};

// File stat info
typedef struct {
    uint32_t inode;
    uint32_t size;
    uint8_t type;
    uint16_t mode;
} stat_t;

// FS Operations vtable
struct fs_ops {
    // There's enough funcpointers here to give someone a heart attack

    const char *name;

    // Lifecycle
    int (*init)(void);
    int (*mount)(const char *device);
    void (*unmount)(void);

    // File operations
    int (*open)(const char *path, int flags, file_t *file);
    int (*close)(file_t *file);
    int (*read)(file_t *file, void *buf, size_t count);
    int (*write)(file_t *file, const void *buf, size_t count);

    // Directory operations
    int (*readdir)(file_t *dir, dirent_t *entry);

    // Metadata (very neat)
    int (*stat)(const char *path, stat_t *st);
};

// Public VFS API
int vfs_init(void);
int vfs_register_fs(fs_ops_t *ops);
int vfs_mount(const char *fs_name, const char *device, const char *mountpoint);

// File operations - intentionally POSIX-ish for future cross-compilation
int vfs_open(const char *path, int flags);
int vfs_close(int fd);
int vfs_read(int fd, void *buf, size_t count);
int vfs_write(int fd, const void *buf, size_t count);
int vfs_stat(const char *path, stat_t *st);

#endif // VFS_H