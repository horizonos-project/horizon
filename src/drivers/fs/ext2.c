// src/drivers/fs/ext2.c
//
// ext2 filesystem driver implementation for Horizon
// (c) 2025 HorizonOS Project
//

#include <stdint.h>
#include <stddef.h>
#include "../vfs/vfs.h"
#include "../block/blkdev.h"
#include "libk/kprint.h"
#include "libk/string.h"
#include "mm/mm.h"

#define EXT2_SUPER_MAGIC 0xEF53
#define EXT2_SUPER_BLOCK_OFFSET 1024

// Inode types
#define EXT2_S_IFREG  0x8000  // Regular file
#define EXT2_S_IFDIR  0x4000  // Directory
#define EXT2_S_IFLNK  0xA000  // Symbolic link

// Directory entry types
#define EXT2_FT_UNKNOWN  0
#define EXT2_FT_REG_FILE 1
#define EXT2_FT_DIR      2
#define EXT2_FT_SYMLINK  7

/**
 * @brief EXT2 Superblock (1024 bytes)
 * 
 * The superblock contains critical filesystem metadata. We only
 * define the fields we actually need - there are more fields but
 * most are for advanced features we don't support yet.
 */
typedef struct {
    uint32_t s_inodes_count;      // Total number of inodes
    uint32_t s_blocks_count;      // Total number of blocks
    uint32_t s_r_blocks_count;    // Reserved blocks
    uint32_t s_free_blocks_count; // Free blocks
    uint32_t s_free_inodes_count; // Free inodes
    uint32_t s_first_data_block;  // First data block (0 or 1)
    uint32_t s_log_block_size;    // Block size = 1024 << s_log_block_size
    uint32_t s_log_frag_size;     // Fragment size (ignore for now)
    uint32_t s_blocks_per_group;  // Blocks per block group
    uint32_t s_frags_per_group;   // Fragments per group
    uint32_t s_inodes_per_group;  // Inodes per block group
    uint32_t s_mtime;             // Mount time
    uint32_t s_wtime;             // Write time
    uint16_t s_mnt_count;         // Mount count
    uint16_t s_max_mnt_count;     // Max mount count
    uint16_t s_magic;             // Magic signature (0xEF53)
    uint16_t s_state;             // Filesystem state
    uint16_t s_errors;            // Error handling
    uint16_t s_minor_rev_level;   // Minor revision level
    uint32_t s_lastcheck;         // Last check time
    uint32_t s_checkinterval;     // Check interval
    uint32_t s_creator_os;        // Creator OS
    uint32_t s_rev_level;         // Revision level
    uint16_t s_def_resuid;        // Default reserved user ID
    uint16_t s_def_resgid;        // Default reserved group ID
    // TODO: We need more of these later down the line.
} __attribute__((packed)) ext2_superblock_t;

/**
 * @brief EXT2 Block Group Descriptor (32 bytes)
 * 
 * Describes where to find the bitmaps and inode table for a block group.
 */
typedef struct {
    uint32_t bg_block_bitmap;      // Block number of block bitmap
    uint32_t bg_inode_bitmap;      // Block number of inode bitmap
    uint32_t bg_inode_table;       // Block number of inode table
    uint16_t bg_free_blocks_count; // Free blocks in group
    uint16_t bg_free_inodes_count; // Free inodes in group
    uint16_t bg_used_dirs_count;   // Number of directories
    uint16_t bg_pad;               // Padding
    uint8_t  bg_reserved[12];      // Reserved
} __attribute__((packed)) ext2_bgd_t;

/**
 * @brief EXT2 Inode (128 bytes)
 * 
 * Contains metadata about a file/directory and pointers to its data blocks.
 */
typedef struct {
    uint16_t i_mode;        // File mode (type + permissions)
    uint16_t i_uid;         // Owner user ID
    uint32_t i_size;        // File size in bytes
    uint32_t i_atime;       // Access time
    uint32_t i_ctime;       // Creation time
    uint32_t i_mtime;       // Modification time
    uint32_t i_dtime;       // Deletion time
    uint16_t i_gid;         // Group ID
    uint16_t i_links_count; // Hard link count
    uint32_t i_blocks;      // Number of 512-byte blocks
    uint32_t i_flags;       // File flags
    uint32_t i_osd1;        // OS-dependent
    uint32_t i_block[15];   // Block pointers (12 direct, 1 indirect, 1 double, 1 triple)
    uint32_t i_generation;  // File generation (for NFS)
    uint32_t i_file_acl;    // File ACL
    uint32_t i_dir_acl;     // Directory ACL
    uint32_t i_faddr;       // Fragment address
    uint8_t  i_osd2[12];    // OS-dependent
} __attribute__((packed)) ext2_inode_t;

/**
 * @brief In-memory EXT2 filesystem state
 * 
 * Cached information about the mounted filesystem.
 */
typedef struct {
    bool mounted;
    uint32_t block_size;         // Computed from superblock
    uint32_t inodes_per_group;
    uint32_t blocks_per_group;
    uint32_t num_block_groups;
    
    blkdev_t *device;
    
    ext2_superblock_t superblock;
    ext2_bgd_t *block_groups;    // Array of block group descriptors
} ext2_state_t;

static ext2_state_t ext2_state = {0};

// ----------------- Forward decl ext2 functions  -----------------

static int  ext2_init(void);
static int  ext2_mount(const char *device);
static void ext2_unmount(void);

static int  ext2_open(const char *path, int flags, file_t *file);
static int  ext2_close(file_t *file);
static int  ext2_read(file_t *file, void *buf, size_t count);
static int  ext2_write(file_t *file, const void *buf, size_t count);
static int  ext2_readdir(file_t *dir, dirent_t *entry);
static int  ext2_stat(const char *path, stat_t *st);

static int ext2_read_superblock(void);
static int ext2_read_inode(uint32_t inode_num, ext2_inode_t *inode);
static int ext2_find_inode_by_path(const char *path, uint32_t *inode_num);

// --------------------- I/O helpers ! --------------------- 

/**
 * @brief Read bytes from the block device
 * 
 * Handles reading arbitrary byte ranges by reading the necessary
 * sectors and extracting the requested data.
 * 
 * @param offset Byte offset from start of device
 * @param buf Buffer to read into
 * @param size Number of bytes to read
 * @return 0 on success, -1 on error
 */
static int ext2_device_read(uint32_t offset, void *buf, size_t size) {
    if (!ext2_state.device) {
        kprintf("[ext2] ERROR: No device mounted\n");
        return -1;
    }
    
    // Calculate which sectors we need
    uint32_t start_sector = offset / 512;
    uint32_t end_sector = (offset + size - 1) / 512;
    uint32_t num_sectors = end_sector - start_sector + 1;
    
    // Allocate temporary buffer for sector-aligned read
    uint8_t *temp = (uint8_t*)kmalloc(num_sectors * 512);
    if (!temp) {
        kprintf("[ext2] ERROR: Failed to allocate temp buffer\n");
        return -1;
    }
    
    // Read sectors
    int ret = blkdev_read(ext2_state.device, start_sector, temp, num_sectors);
    if (ret < 0) {
        kprintf("[ext2] ERROR: Block device read failed\n");
        kfree(temp);
        return -1;
    }
    
    // Extract requested bytes
    uint32_t offset_in_sector = offset % 512;
    memcpy(buf, temp + offset_in_sector, size);
    
    kfree(temp);
    return 0;
}

/**
 * @brief Read a filesystem block
 * 
 * EXT2 blocks can be 1024, 2048, or 4096 bytes. This function
 * reads an entire block.
 * 
 * @param block_num Block number (not byte offset!)
 * @param buf Buffer to read into (must be block_size bytes)
 * @return 0 on success, -1 on error
 */
static int ext2_read_block(uint32_t block_num, void *buf) {
    if (ext2_state.block_size == 0) {
        kprintf("[ext2] ERROR: Block size not initialized\n");
        return -1;
    }
    
    uint32_t offset = block_num * ext2_state.block_size;
    return ext2_device_read(offset, buf, ext2_state.block_size);
}

// --------------------- FSOps table ---------------------

static fs_ops_t ext2_ops = {
    .name     = "ext2",
    .init     = ext2_init,
    .mount    = ext2_mount,
    .unmount  = ext2_unmount,
    .open     = ext2_open,
    .close    = ext2_close,
    .read     = ext2_read,
    .write    = ext2_write,
    .readdir  = ext2_readdir,
    .stat     = ext2_stat,
};

// --------------------- ext2 public api ---------------------

static int ext2_init(void) {
    kprintf("[ext2] Initalizing ext2 driver...\n");
    memset(&ext2_state, 0, sizeof(ext2_state));
    return 0;
}

static int ext2_mount(const char *device) {
    kprintf("[ext2] Mounting '%s'...\n", device ? device : "(NULL)");

    if (ext2_state.mounted) {
        kprintf("[ext2] ERROR: Already mounted\n");
        return -1;
    }
    
    if (!device) {
        kprintf("[ext2] ERROR: No device specified\n");
        return -1;
    }
    
    // CHANGED: Find block device by name
    ext2_state.device = blkdev_find(device);
    if (!ext2_state.device) {
        kprintf("[ext2] ERROR: Block device '%s' not found\n", device);
        return -1;
    }
    
    // Read and validate superblock
    if (ext2_read_superblock() < 0) {
        ext2_state.device = NULL;
        return -1;
    }
    
    // Read block group descriptor table
    if (ext2_read_bgd_table() < 0) {
        ext2_state.device = NULL;
        return -1;
    }
    
    ext2_state.mounted = true;
    kprintf("[ext2] Mount successful!\n");
    return 0;
}

static void ext2_unmount(void) {
    kprintf("[ext2] Unmounting...\n");

    if (ext2_state.block_groups) {
        kfree(ext2_state.block_groups);
        ext2_state.block_groups = NULL;
    }

    ext2_state.device = NULL;
    ext2_state.mounted = false;
}

static int ext2_open(const char *path, int flags, file_t *file) {
    kprintf("[ext2] open('%s', flags=%d)\n", path, flags);
    
    if (!ext2_state.mounted) {
        kprintf("[ext2] ERROR: Filesystem not mounted\n");
        return -1;
    }
    
    // TODO: Phase 4 - Find inode by path
    // TODO: Phase 5 - Set up file_t structure
    
    return -1; // ENOENT for now
}

static int ext2_close(file_t *file) {
    (void)file;
    // Nothing to clean up yet
    return 0;
}

static int ext2_read(file_t *file, void *buf, size_t count) {
    (void)file; (void)buf; (void)count;
    kprintf("[ext2] read() - TODO\n");
    return 0;
}

static int ext2_write(file_t *file, const void *buf, size_t count) {
    (void)file; (void)buf; (void)count;
    kprintf("[ext2] write() - read-only for now\n");
    return -1; // EROFS
}

static int ext2_readdir(file_t *dir, dirent_t *entry) {
    (void)dir; (void)entry;
    kprintf("[ext2] readdir() - TODO\n");
    return 0;
}

static int ext2_stat(const char *path, stat_t *st) {
    (void)path; (void)st;
    kprintf("[ext2] stat() - TODO\n");
    return -1;
}

// ----------------- Helper Functions -----------------

/**
 * @brief Read and validate the EXT2 superblock
 */
static int ext2_read_superblock(void) {
    kprintf("[ext2] Reading superblock at offset %d...\n", EXT2_SUPER_BLOCK_OFFSET);
    
    // Read the superblock from byte 1024
    if (ext2_device_read(EXT2_SUPER_BLOCK_OFFSET, 
                         &ext2_state.superblock, 
                         sizeof(ext2_superblock_t)) < 0) {
        kprintf("[ext2] ERROR: Failed to read superblock\n");
        return -1;
    }
    
    // Validate magic number
    if (ext2_state.superblock.s_magic != EXT2_SUPER_MAGIC) {
        kprintf("[ext2] ERROR: Invalid magic number 0x%x (expected 0x%x)\n",
                ext2_state.superblock.s_magic, EXT2_SUPER_MAGIC);
        return -1;
    }
    
    kprintf("[ext2] Valid EXT2 filesystem detected!\n");
    
    // Calculate block size
    ext2_state.block_size = 1024 << ext2_state.superblock.s_log_block_size;
    ext2_state.inodes_per_group = ext2_state.superblock.s_inodes_per_group;
    ext2_state.blocks_per_group = ext2_state.superblock.s_blocks_per_group;
    
    // Calculate number of block groups
    ext2_state.num_block_groups = 
        (ext2_state.superblock.s_blocks_count + ext2_state.blocks_per_group - 1) 
        / ext2_state.blocks_per_group;
    
    kprintf("[ext2] Block size: %u bytes\n", ext2_state.block_size);
    kprintf("[ext2] Total blocks: %u\n", ext2_state.superblock.s_blocks_count);
    kprintf("[ext2] Total inodes: %u\n", ext2_state.superblock.s_inodes_count);
    kprintf("[ext2] Inodes per group: %u\n", ext2_state.inodes_per_group);
    kprintf("[ext2] Blocks per group: %u\n", ext2_state.blocks_per_group);
    kprintf("[ext2] Block groups: %u\n", ext2_state.num_block_groups);
    
    return 0;
}

/**
 * @brief Read block group descriptor table
 * 
 * The BGD table comes immediately after the superblock.
 * Its location depends on block size:
 * - 1024 byte blocks: BGD starts at block 2
 * - 2048+ byte blocks: BGD starts at block 1
 */
static int ext2_read_bgd_table(void) {
    kprintf("[ext2] Reading block group descriptor table...\n");
    
    // Calculate BGD table location
    uint32_t bgd_block = (ext2_state.block_size == 1024) ? 2 : 1;
    uint32_t bgd_offset = bgd_block * ext2_state.block_size;
    
    // Allocate memory for BGD table
    size_t bgd_table_size = ext2_state.num_block_groups * sizeof(ext2_bgd_t);
    ext2_state.block_groups = (ext2_bgd_t*)kmalloc(bgd_table_size);
    if (!ext2_state.block_groups) {
        kprintf("[ext2] ERROR: Failed to allocate BGD table\n");
        return -1;
    }
    
    // Read BGD table
    if (ext2_device_read(bgd_offset, ext2_state.block_groups, bgd_table_size) < 0) {
        kprintf("[ext2] ERROR: Failed to read BGD table\n");
        kfree(ext2_state.block_groups);
        ext2_state.block_groups = NULL;
        return -1;
    }
    
    kprintf("[ext2] BGD table loaded (%u groups)\n", ext2_state.num_block_groups);
    
    // Debug: Print first block group info
    ext2_bgd_t *bg0 = &ext2_state.block_groups[0];
    kprintf("[ext2] Block Group 0:\n");
    kprintf("  Block bitmap: block %u\n", bg0->bg_block_bitmap);
    kprintf("  Inode bitmap: block %u\n", bg0->bg_inode_bitmap);
    kprintf("  Inode table:  block %u\n", bg0->bg_inode_table);
    kprintf("  Free blocks:  %u\n", bg0->bg_free_blocks_count);
    kprintf("  Free inodes:  %u\n", bg0->bg_free_inodes_count);
    
    return 0;
}

static int ext2_read_inode(uint32_t inode_num, ext2_inode_t *inode) {
    // TODO: Phase 3
    (void)inode_num; (void)inode;
    return -1;
}

static int ext2_find_inode_by_path(const char *path, uint32_t *inode_num) {
    // TODO: Phase 4
    (void)path; (void)inode_num;
    return -1;
}

// ----------------- Registration -----------------

int ext2_register(void) {
    kprintf("[ext2] Registering EXT2 filesystem driver...\n");
    int ret = vfs_register_fs(&ext2_ops);
    if (ret == 0) {
        kprintf("[ext2] Registration successful!\n");
        return 0;
    } else {
        kprintf("[ext2] Registration failed (code %d)\n", ret);
        return -1;
    }
}