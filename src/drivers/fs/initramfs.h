/**
 * @file initramfs.h
 * @brief Initial RAM Filesystem Driver
 * 
 * Loads and mounts a TAR archive as the initial root filesystem during boot.
 * This provides essential files (/bin/init, /etc/*, libraries) before the
 * real root filesystem can be mounted.
 * 
 * The initramfs is loaded by the bootloader (GRUB) as a multiboot module
 * and placed in memory at a known address. We parse it as a USTAR tar
 * archive and register it as a VFS filesystem.
 * 
 * Why initramfs?
 * - Provides files before disk drivers are loaded
 * - Contains /bin/init (the first userspace process)
 * - Can hold kernel modules needed to mount real root
 * - Simple format (USTAR tar) - easy to parse
 * 
 * Just like Linux! We're following in the footsteps of the pros. 
 * Also this was easier than anything else I could think of.
 * 
 * @note This is READ-ONLY - writes to initramfs are not supported
 * @see https://en.wikipedia.org/wiki/Initramfs
 */

#ifndef INITRAMFS_H
#define INITRAMFS_H

#include <stdint.h>

/**
 * @brief USTAR tar archive header
 * 
 * Each file in a tar archive is preceded by a 512-byte header describing
 * the file's name, size, permissions, and type. We parse these headers
 * to build a directory tree in memory.
 * 
 * USTAR format details:
 * - magic: Must be "ustar\0" to be valid
 * - version: Should be "00"
 * - typeflag: '0' or '\0' = regular file, '5' = directory, '2' = symlink
 * - size: Octal ASCII string (e.g., "00001234" = 668 bytes)
 * - name: File path (up to 100 chars, or use prefix for longer paths)
 * 
 * After the header comes the file data, padded to 512-byte blocks.
 * 
 * @note Structure must be exactly 512 bytes and packed!
 */
struct tar_header {
    char name[100];      /**< File name (null-terminated if < 100 chars) */
    char mode[8];        /**< File permissions (octal, e.g., "0000644") */
    char uid[8];         /**< Owner user ID (octal) */
    char gid[8];         /**< Owner group ID (octal) */
    char size[12];       /**< File size in bytes (octal ASCII) */
    char mtime[12];      /**< Modification time (Unix timestamp, octal) */
    char checksum[8];    /**< Header checksum (octal) */
    char typeflag;       /**< File type: '0'=file, '5'=dir, '2'=symlink */
    char linkname[100];  /**< Link target (for symlinks) */
    char magic[6];       /**< Magic: "ustar\0" */
    char version[2];     /**< Version: "00" */
    char uname[32];      /**< Owner user name */
    char gname[32];      /**< Owner group name */
    char devmajor[8];    /**< Device major number (for device files) */
    char devminor[8];    /**< Device minor number (for device files) */
    char prefix[155];    /**< Path prefix (for names > 100 chars) */
    char padding[12];    /**< Padding to 512 bytes */
} __attribute__((packed));

/**
 * @brief Initialize the initramfs filesystem
 * 
 * Parses the tar archive loaded by the bootloader and registers it as
 * a VFS filesystem. Creates an in-memory directory structure for fast
 * file lookups.
 * 
 * Steps:
 * - 1. Find initramfs in multiboot modules
 * - 2. Parse tar headers to build file list
 * - 3. Validate checksums and file sizes
 * - 4. Register as VFS filesystem ("initramfs")
 * - 5. Mount at "/" (root)
 * 
 * After initialization, files can be opened via VFS:
 * @code
 * int fd = vfs_open("/bin/hello", O_RDONLY);
 * vfs_read(fd, buffer, 1024);
 * @endcode
 * 
 * @note Must be called after VFS initialization and multiboot parsing
 * @note Initramfs is READ-ONLY - open() with O_WRONLY will fail
 */
void initramfs_init(void);

#endif // INITRAMFS_H