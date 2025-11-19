/**
 * @file ext2.h
 * @brief ext2 Filesystem Driver
 * 
 * The Second Extended Filesystem - a classic Linux filesystem and the
 * gateway to REAL persistent storage! Unlike initramfs (RAM-based) or
 * tmpfs (also RAM), ext2 actually reads from and writes to disk.
 * 
 * ext2 features:
 * - Inodes for file metadata
 * - Block groups for organizing disk space
 * - Directory entries with named files
 * - Support for symlinks, permissions, timestamps
 * - Relatively simple (no journaling like ext3/ext4)
 * 
 * Why ext2?
 * - Well-documented format (easier than NTFS, FAT is too simple)
 * - No journaling complexity (ext3/ext4 add this on top)
 * - Linux compatibility (can mount real ext2 partitions!)
 * - Good learning experience for filesystem internals
 * 
 * This header is shockingly empty because all the complexity is hidden
 * in the implementation. We just expose one function to the VFS:
 * "here's my filesystem, please register it!"
 * 
 * The real work (superblock parsing, inode reading, block allocation,
 * directory traversal) all happens behind the fs_ops_t vtable.
 * 
 */

#pragma once

/**
 * @brief Register ext2 filesystem with VFS
 * 
 * Initializes the ext2 driver and registers it with the VFS layer.
 * After registration, ext2 partitions can be mounted:
 * 
 * @code
 * ext2_register();
 * vfs_mount("ext2", "/dev/sda1", "/mnt");
 * @endcode
 * 
 * The function sets up the ext2 fs_ops_t structure with implementations of:
 * - init: Initialize ext2 driver
 * - mount: Parse superblock, load block groups
 * - open: Resolve path, load inode, create file_t
 * - read: Read file data from disk blocks
 * - write: Write file data to disk blocks (if implemented)
 * - stat: Get file metadata from inode
 * - readdir: List directory contents
 * 
 * @return 0 on success, -1 on failure
 * 
 * @note Call this after VFS initialization, before mounting any ext2 partitions
 * @warning Actual disk I/O required - make sure ATA driver is working!
 * 
 * @todo Implement write support (currently read-only?)
 * @todo Add journaling (ext3) or extent support (ext4) eventually
 */
int ext2_register(void);
