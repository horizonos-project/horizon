/**
 * @file vfs.h
 * @brief Virtual File System Layer
 * 
 * The VFS is the abstraction layer that sits between syscalls and actual
 * filesystems (ext2, initramfs, etc.). It provides a unified interface
 * so that user programs don't need to care whether they're reading from a
 * real disk, RAM, or /proc.
 * 
 * Architecture:
 * ```
 * User: open("/etc/motd")
 *   ↓
 * Syscall: sys_open()
 *   ↓
 * VFS: vfs_open() → Find filesystem for "/" → Call fs->open()
 *   ↓
 * Filesystem: initramfs_open() / ext2_open() / tmpfs_open()
 * ```
 * 
 * The VFS manages:
 * - File descriptor table (maps FD numbers to open files)
 * - Filesystem registration (pluggable FS backends)
 * - Mount points (which filesystem handles which paths)
 * - Path resolution (walking directory trees)
 * 
 * This design means adding a new filesystem is just:
 * 1. Implement the fs_ops_t vtable
 * 2. Call vfs_register_fs()
 * 3. Mount it somewhere
 * 
 * No syscall changes needed! Clean separation of concerns.
 */

#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/** @brief Regular file */
#define VFS_FILE    0x01

/** @brief Directory */
#define VFS_DIR     0x02

/** @brief Symbolic link (coming "soon") */
#define VFS_SYMLINK 0x03

/** @brief Open for reading only */
#define O_RDONLY    0x00

/** @brief Open for writing only */
#define O_WRONLY    0x01

/** @brief Open for reading and writing */
#define O_RDWR      0x02

/** @brief Seek relative to start of file */
#define SEEK_SET    0

/** @brief Seek relative to current position */
#define SEEK_CUR    1

/** @brief Seek relative to end of file */
#define SEEK_END    2

/** @brief Maximum number of open file descriptors (global, for now) */
#define VFS_MAX_FDS 256

typedef struct fs_ops fs_ops_t;

/**
 * @brief Open file descriptor
 * 
 * Represents an open file in the system. Each process will eventually
 * have its own FD table, but for now this is global.
 * 
 * The file struct is filesystem-agnostic - all FS-specific details are
 * hidden behind fs_data and accessed through fs_ops function pointers.
 */
typedef struct file {
    bool in_use;        /**< Whether this FD slot is allocated */
    uint32_t offset;    /**< Current read/write position in file */
    int flags;          /**< Open flags (O_RDONLY, O_WRONLY, O_RDWR) */
    
    void *fs_data;      /**< Filesystem-specific data (inode, etc.) */
    fs_ops_t *fs_ops;   /**< Operations vtable - who owns this file? */
} file_t;

typedef struct dirent dirent_t;

/**
 * @brief Directory entry (dirent)
 * 
 * Returned by readdir() (coming at some point) when listing directory content.
 * Contains just enough info to identify the file - use stat()
 * to get detailed information.
 */
struct dirent {
    uint32_t inode;     /**< Inode number (filesystem-specific) */
    uint8_t type;       /**< File type (VFS_FILE, VFS_DIR, etc.) */
    // TODO: Add name field when we need it
};

/**
 * @brief File stat information
 * 
 * Metadata about a file returned by stat(). Similar to POSIX struct stat
 * but simplified for our needs (no timestamps, ownership, etc. yet).
 */
typedef struct {
    uint32_t inode;     /**< Inode number */
    uint32_t size;      /**< File size in bytes */
    uint8_t type;       /**< File type (VFS_FILE, VFS_DIR, etc.) */
    uint16_t mode;      /**< Permissions (rwxrwxrwx, not enforced yet) */
} stat_t;

/**
 * @brief Filesystem operations vtable
 * 
 * Each filesystem implementation (ext2, tmpfs, initramfs, procfs, etc.)
 * fills out this structure with function pointers to its implementations.
 * 
 * The VFS calls these functions to perform actual filesystem operations.
 * Not all filesystems need to implement all functions (e.g., read-only
 * filesystems can leave write as NULL).
 * 
 * Yes, this is a LOT of function pointers. Welcome to VFS development! 💀
 */
struct fs_ops {
    const char *name;   /**< Filesystem name (e.g., "ext2", "tmpfs") */
    
    /* Lifecycle operations */
    int (*init)(void);                          /**< Initialize filesystem driver */
    int (*mount)(const char *device);           /**< Mount filesystem from device */
    void (*unmount)(void);                      /**< Unmount and cleanup */
    
    /* File operations */
    int (*open)(const char *path, int flags, file_t *file);    /**< Open file */
    int (*close)(file_t *file);                                /**< Close file */
    int (*read)(file_t *file, void *buf, size_t count);        /**< Read from file */
    int (*write)(file_t *file, const void *buf, size_t count); /**< Write to file */
    
    /* Directory operations */
    int (*readdir)(file_t *dir, dirent_t *entry);              /**< Read directory entry */
    
    /* Metadata operations (very neat indeed!) */
    int (*stat)(const char *path, stat_t *st);                 /**< Get file metadata */
};

/**
 * @brief Initialize the VFS subsystem
 * 
 * Sets up the file descriptor table and prepares for filesystem registration.
 * Must be called early in kernel initialization.
 * 
 * @return 0 on success, -1 on failure
 */
int vfs_init(void);

/**
 * @brief Register a filesystem driver
 * 
 * Adds a new filesystem type to the VFS. After registration, the filesystem
 * can be mounted and used.
 * 
 * @param ops Pointer to filled-out fs_ops_t structure
 * @return 0 on success, -1 on failure (table full, etc.)
 * 
 * Example:
 * @code
 * fs_ops_t initramfs_ops = {
 *     .name = "initramfs",
 *     .open = initramfs_open,
 *     .read = initramfs_read,
 *     // ... etc
 * };
 * vfs_register_fs(&initramfs_ops);
 * @endcode
 */
int vfs_register_fs(fs_ops_t *ops);

/**
 * @brief Mount a filesystem at a mount point
 * 
 * Associates a filesystem with a path in the VFS tree. After mounting,
 * files under that path are handled by the mounted filesystem.
 * 
 * @param fs_name Name of registered filesystem (e.g., "ext2")
 * @param device Device path (e.g., "/dev/sda1") or NULL for pseudo-filesystems
 * @param mountpoint Path to mount at (e.g., "/", "/mnt/disk")
 * @return 0 on success, -1 on failure
 * 
 * Example:
 * @code
 * vfs_mount("initramfs", NULL, "/");           // Root filesystem
 * vfs_mount("ext2", "/dev/sda1", "/mnt");     // Mount disk
 * vfs_mount("procfs", NULL, "/proc");         // Pseudo-filesystem
 * @endcode
 */
int vfs_mount(const char *fs_name, const char *device, const char *mountpoint);

/**
 * @brief Open a file
 * 
 * Opens a file and returns a file descriptor. The VFS resolves the path,
 * finds the appropriate filesystem, and calls its open() function.
 * 
 * @param path File path (e.g., "/etc/motd")
 * @param flags Open flags (O_RDONLY, O_WRONLY, O_RDWR)
 * @return File descriptor (>= 0) on success, -1 on failure
 * 
 * Example:
 * @code
 * int fd = vfs_open("/etc/motd", O_RDONLY);
 * if (fd < 0) {
 *     kprintf("Failed to open file\n");
 * }
 * @endcode
 */
int vfs_open(const char *path, int flags);

/**
 * @brief Close a file descriptor
 * 
 * Closes an open file and frees the FD slot. Calls the filesystem's
 * close() function to perform any necessary cleanup.
 * 
 * @param fd File descriptor to close
 * @return 0 on success, -1 on failure (invalid FD)
 */
int vfs_close(int fd);

/**
 * @brief Read from a file
 * 
 * Reads up to count bytes from the file at its current offset.
 * Updates the file offset after reading.
 * 
 * @param fd File descriptor
 * @param buf Buffer to read into
 * @param count Maximum number of bytes to read
 * @return Number of bytes read, 0 on EOF, -1 on error
 */
int vfs_read(int fd, void *buf, size_t count);

/**
 * @brief Write to a file
 * 
 * Writes up to count bytes to the file at its current offset.
 * Updates the file offset after writing.
 * 
 * @param fd File descriptor
 * @param buf Buffer containing data to write
 * @param count Number of bytes to write
 * @return Number of bytes written, -1 on error
 * 
 * @note Not all filesystems support writing (e.g., initramfs is read-only)
 */
int vfs_write(int fd, const void *buf, size_t count);

/**
 * @brief Get file metadata
 * 
 * Retrieves information about a file without opening it.
 * 
 * @param path File path
 * @param st Pointer to stat_t structure to fill
 * @return 0 on success, -1 on failure (file not found, etc.)
 * 
 * Example:
 * @code
 * stat_t st;
 * if (vfs_stat("/etc/motd", &st) == 0) {
 *     kprintf("File size: %u bytes\n", st.size);
 * }
 * @endcode
 */
int vfs_stat(const char *path, stat_t *st);

#endif // VFS_H