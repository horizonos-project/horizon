/**
 * @file file.h
 * @brief File Descriptor Table Management
 * 
 * Manages the global file descriptor table - the mapping between integer
 * FD numbers (what userspace uses) and actual file_t structures (what the
 * VFS uses internally).
 * 
 * When a user opens "/etc/motd", the VFS:
 * 1. Calls fd_alloc() to get a free FD slot
 * 2. Fills in the file_t structure (offset, flags, fs_data)
 * 3. Returns the FD number (e.g., 3) to userspace
 * 
 * When the user later calls read(3, ...), the syscall:
 * 1. Calls fd_get(3) to retrieve the file_t structure
 * 2. Uses that to call the filesystem's read() function
 * 
 * Simple but critical! Without this, we'd be passing raw pointers to
 * userspace (bad idea) or have no way to track open files.
 * 
 * @note Currently global (one FD table for the whole system). Eventually
 *       this will be per-process when we have proper process management.
 *       Whenever that is, probably by summer 2026.
 */

#ifndef VFS_FILE_H
#define VFS_FILE_H

#include "vfs.h"

/**
 * @brief Initialize the file descriptor table
 * 
 * Sets up the global FD table and marks standard streams (stdin, stdout,
 * stderr) as reserved. Must be called during VFS initialization.
 * 
 * FD allocation after init:
 * - 0, 1, 2: Reserved for stdin, stdout, stderr
 * - 3+: Available for user files
 * 
 * @note Call this before any file operations!
 */
void fd_table_init(void);

/**
 * @brief Allocate a new file descriptor
 * 
 * Finds the next available FD slot, marks it as in-use, and returns
 * both the FD number and a pointer to the file_t structure for filling in.
 * 
 * @param out Output parameter - receives pointer to allocated file_t
 * @return File descriptor number (>= 3), or -1 if table is full
 * 
 * Example:
 * @code
 * file_t *file;
 * int fd = fd_alloc(&file);
 * if (fd >= 0) {
 *     file->offset = 0;
 *     file->flags = O_RDONLY;
 *     file->fs_data = inode;
 *     file->fs_ops = &ext2_ops;
 * }
 * @endcode
 */
int fd_alloc(file_t **out);

/**
 * @brief Free a file descriptor
 * 
 * Marks an FD slot as available for reuse. Should be called when a file
 * is closed to prevent FD leaks.
 * 
 * @param fd File descriptor to free
 * 
 * @note Does NOT call the filesystem's close() function - that should
 *       be done by vfs_close() before calling this!
 */
void fd_free(int fd);

/**
 * @brief Get file structure for a file descriptor
 * 
 * Looks up an FD number and returns the corresponding file_t structure.
 * Used by syscalls to convert the user's FD number into something the
 * VFS can actually work with.
 * 
 * @param fd File descriptor number
 * @return Pointer to file_t structure, or NULL if invalid FD
 * 
 * Example:
 * @code
 * file_t *file = fd_get(3);
 * if (file) {
 *     file->fs_ops->read(file, buffer, 128);
 * }
 * @endcode
 */
file_t* fd_get(int fd);

#endif // VFS_FILE_H