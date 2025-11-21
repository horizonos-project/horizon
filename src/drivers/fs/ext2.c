// src/drivers/fs/ext2.c
//
// ext2 filesystem driver implementation for Horizon
// (c) 2025 HorizonOS Project
//

#include <stdint.h>
#include <stddef.h>
#include "../vfs/vfs.h"
#include "../block/blkdev.h"
#include "kernel/log.h"
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
 * @brief EXT2 Directory Entry (variable length)
 * 
 * Directories are just files containing a list of these entries.
 */
typedef struct {
    uint32_t inode;      // Inode number (0 = unused entry)
    uint16_t rec_len;    // Length of this entry (including name)
    uint8_t  name_len;   // Length of name
    uint8_t  file_type;  // File type (EXT2_FT_*)
    char     name[];     // Filename (not null-terminated!)
} __attribute__((packed)) ext2_dirent_t;

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
    uint32_t block_size;
    uint32_t inodes_per_group;
    uint32_t blocks_per_group;
    uint32_t num_block_groups;
    
    blkdev_t *device;
    
    ext2_superblock_t superblock;
    ext2_bgd_t *block_groups;
} ext2_state_t;

static ext2_state_t ext2_state = {0};

// ----------------- Forward Declarations -----------------

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
static int ext2_read_bgd_table(void);
static int ext2_read_inode(uint32_t inode_num, ext2_inode_t *inode);
static int ext2_find_inode_by_path(const char *path, uint32_t *inode_num);
static int ext2_search_directory(ext2_inode_t *dir_inode, const char *name, uint32_t *found_inode);
static int ext2_read_inode_data(ext2_inode_t *inode, uint32_t offset, void *buf, size_t count);
static uint32_t ext2_get_block_number(ext2_inode_t *inode, uint32_t file_block);

// ----------------- I/O Helpers -----------------

/**
 * @brief Read bytes from the block device
 */
static int ext2_device_read(uint32_t offset, void *buf, size_t size) {
    if (!ext2_state.device) {
        kprintf_both("[ext2] ERROR: No device mounted\n");
        return -1;
    }
    
    uint32_t start_sector = offset / 512;
    uint32_t end_sector = (offset + size - 1) / 512;
    uint32_t num_sectors = end_sector - start_sector + 1;
    
    uint8_t *temp = (uint8_t*)kalloc(num_sectors * 512);
    if (!temp) {
        kprintf_both("[ext2] ERROR: Failed to allocate temp buffer\n");
        return -1;
    }
    
    int ret = blkdev_read(ext2_state.device, start_sector, temp, num_sectors);
    if (ret < 0) {
        kprintf_both("[ext2] ERROR: Block device read failed\n");
        kfree(temp);
        return -1;
    }
    
    uint32_t offset_in_sector = offset % 512;
    memcpy(buf, temp + offset_in_sector, size);
    
    kfree(temp);
    return 0;
}

/**
 * @brief Read a filesystem block
 */
static int ext2_read_block(uint32_t block_num, void *buf) {
    if (ext2_state.block_size == 0) {
        kprintf_both("[ext2] ERROR: Block size not initialized\n");
        return -1;
    }
    
    uint32_t offset = block_num * ext2_state.block_size;
    return ext2_device_read(offset, buf, ext2_state.block_size);
}

// ----------------- FSOps Table -----------------

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

// ----------------- Filesystem Operations -----------------

static int ext2_init(void) {
    kprintf_both("[ext2] Initializing ext2 driver...\n");
    memset(&ext2_state, 0, sizeof(ext2_state));
    return 0;
}

static int ext2_mount(const char *device) {
    kprintf_both("[ext2] Mounting '%s'...\n", device ? device : "(NULL)");

    if (ext2_state.mounted) {
        kprintf_both("[ext2] ERROR: Already mounted\n");
        return -1;
    }
    
    if (!device) {
        kprintf_both("[ext2] ERROR: No device specified\n");
        return -1;
    }
    
    ext2_state.device = blkdev_find(device);
    if (!ext2_state.device) {
        kprintf_both("[ext2] ERROR: Block device '%s' not found\n", device);
        return -1;
    }
    
    if (ext2_read_superblock() < 0) {
        ext2_state.device = NULL;
        return -1;
    }
    
    if (ext2_read_bgd_table() < 0) {
        ext2_state.device = NULL;
        return -1;
    }
    
    ext2_state.mounted = true;
    kprintf_both("[ext2] Mount successful!\n");
    return 0;
}

static void ext2_unmount(void) {
    kprintf_both("[ext2] Unmounting...\n");

    if (ext2_state.block_groups) {
        kfree(ext2_state.block_groups);
        ext2_state.block_groups = NULL;
    }

    ext2_state.device = NULL;
    ext2_state.mounted = false;
}

static int ext2_open(const char *path, int flags, file_t *file) {
    kprintf_both("[ext2] open('%s', flags=%d)\n", path, flags);
    
    if (!ext2_state.mounted) {
        kprintf_both("[ext2] ERROR: Filesystem not mounted\n");
        return -1;
    }
    
    // Find the inode for this path
    uint32_t inode_num;
    if (ext2_find_inode_by_path(path, &inode_num) < 0) {
        return -1;
    }
    
    // Read the inode
    ext2_inode_t *inode = (ext2_inode_t*)kalloc(sizeof(ext2_inode_t));
    if (!inode) {
        return -1;
    }
    
    if (ext2_read_inode(inode_num, inode) < 0) {
        kfree(inode);
        return -1;
    }
    
    // Store inode in file structure
    file->fs_data = inode;
    file->offset = 0;
    file->flags = flags;
    
    return 0;
}

static int ext2_close(file_t *file) {
    if (file && file->fs_data) {
        kfree(file->fs_data);
        file->fs_data = NULL;
    }
    return 0;
}

static int ext2_read(file_t *file, void *buf, size_t count) {
    if (!file || !file->fs_data) {
        return -1;
    }
    
    ext2_inode_t *inode = (ext2_inode_t*)file->fs_data;
    
    int bytes_read = ext2_read_inode_data(inode, file->offset, buf, count);
    if (bytes_read > 0) {
        file->offset += bytes_read;
    }
    
    return bytes_read;
}

static int ext2_write(file_t *file, const void *buf, size_t count) {
    (void)file; (void)buf; (void)count;
    kprintf_both("[ext2] write() - read-only for now\n");
    return -1;
}

static int ext2_readdir(file_t *dir, dirent_t *entry) {
    (void)dir; (void)entry;
    kprintf_both("[ext2] readdir() - TODO\n");
    return 0;
}

static int ext2_stat(const char *path, stat_t *st) {
    uint32_t inode_num;
    if (ext2_find_inode_by_path(path, &inode_num) < 0) {
        return -1;
    }
    
    ext2_inode_t inode;
    if (ext2_read_inode(inode_num, &inode) < 0) {
        return -1;
    }
    
    st->inode = inode_num;
    st->size = inode.i_size;
    st->mode = inode.i_mode;
    
    // Determine type
    if (inode.i_mode & EXT2_S_IFDIR) {
        st->type = VFS_DIR;
    } else if (inode.i_mode & EXT2_S_IFREG) {
        st->type = VFS_FILE;
    } else {
        st->type = VFS_FILE;
    }
    
    return 0;
}

// ----------------- Helper Functions -----------------

static int ext2_read_superblock(void) {
    kprintf_both("[ext2] Reading superblock at offset %d...\n", EXT2_SUPER_BLOCK_OFFSET);
    
    if (ext2_device_read(EXT2_SUPER_BLOCK_OFFSET, 
                         &ext2_state.superblock, 
                         sizeof(ext2_superblock_t)) < 0) {
        kprintf_both("[ext2] ERROR: Failed to read superblock\n");
        return -1;
    }
    
    if (ext2_state.superblock.s_magic != EXT2_SUPER_MAGIC) {
        kprintf_both("[ext2] ERROR: Invalid magic number 0x%x (expected 0x%x)\n",
                ext2_state.superblock.s_magic, EXT2_SUPER_MAGIC);
        return -1;
    }
    
    kprintf_both("[ext2] Valid EXT2 filesystem detected!\n");
    
    ext2_state.block_size = 1024 << ext2_state.superblock.s_log_block_size;
    ext2_state.inodes_per_group = ext2_state.superblock.s_inodes_per_group;
    ext2_state.blocks_per_group = ext2_state.superblock.s_blocks_per_group;
    
    ext2_state.num_block_groups = 
        (ext2_state.superblock.s_blocks_count + ext2_state.blocks_per_group - 1) 
        / ext2_state.blocks_per_group;
    
    kprintf_both("[ext2] Block size: %u bytes\n", ext2_state.block_size);
    kprintf_both("[ext2] Total blocks: %u\n", ext2_state.superblock.s_blocks_count);
    kprintf_both("[ext2] Total inodes: %u\n", ext2_state.superblock.s_inodes_count);
    kprintf_both("[ext2] Inodes per group: %u\n", ext2_state.inodes_per_group);
    kprintf_both("[ext2] Blocks per group: %u\n", ext2_state.blocks_per_group);
    kprintf_both("[ext2] Block groups: %u\n", ext2_state.num_block_groups);
    
    return 0;
}

static int ext2_read_bgd_table(void) {
    kprintf_both("[ext2] Reading block group descriptor table...\n");
    
    uint32_t bgd_block = (ext2_state.block_size == 1024) ? 2 : 1;
    uint32_t bgd_offset = bgd_block * ext2_state.block_size;
    
    size_t bgd_table_size = ext2_state.num_block_groups * sizeof(ext2_bgd_t);
    ext2_state.block_groups = (ext2_bgd_t*)kalloc(bgd_table_size);
    if (!ext2_state.block_groups) {
        kprintf_both("[ext2] ERROR: Failed to allocate BGD table\n");
        return -1;
    }
    
    if (ext2_device_read(bgd_offset, ext2_state.block_groups, bgd_table_size) < 0) {
        kprintf_both("[ext2] ERROR: Failed to read BGD table\n");
        kfree(ext2_state.block_groups);
        ext2_state.block_groups = NULL;
        return -1;
    }
    
    kprintf_both("[ext2] BGD table loaded (%u groups)\n", ext2_state.num_block_groups);
    
    ext2_bgd_t *bg0 = &ext2_state.block_groups[0];
    kprintf_both("[ext2] Block Group 0:\n");
    kprintf_both("  Block bitmap: block %u\n", bg0->bg_block_bitmap);
    kprintf_both("  Inode bitmap: block %u\n", bg0->bg_inode_bitmap);
    kprintf_both("  Inode table:  block %u\n", bg0->bg_inode_table);
    kprintf_both("  Free blocks:  %u\n", bg0->bg_free_blocks_count);
    kprintf_both("  Free inodes:  %u\n", bg0->bg_free_inodes_count);
    
    return 0;
}

/**
 * @brief Read an inode from disk
 * 
 * EXT2 inodes are numbered starting from 1 (not 0).
 * Inode 2 is always the root directory.
 * 
 * @param inode_num Inode number (1-based)
 * @param inode Output buffer for inode data
 * @return 0 on success, -1 on error
 */
static int ext2_read_inode(uint32_t inode_num, ext2_inode_t *inode) {
    if (inode_num == 0) {
        kprintf_both("[ext2] ERROR: Invalid inode 0\n");
        return -1;
    }
    
    // Inodes are 1-indexed, but we need 0-indexed for math
    uint32_t inode_index = inode_num - 1;
    
    // Calculate which block group contains this inode
    uint32_t block_group = inode_index / ext2_state.inodes_per_group;
    uint32_t local_inode_index = inode_index % ext2_state.inodes_per_group;
    
    if (block_group >= ext2_state.num_block_groups) {
        kprintf_both("[ext2] ERROR: Inode %u out of range\n", inode_num);
        return -1;
    }
    
    // Get the inode table block for this group
    ext2_bgd_t *bgd = &ext2_state.block_groups[block_group];
    uint32_t inode_table_block = bgd->bg_inode_table;
    
    // Calculate byte offset within inode table
    // Each inode is 128 bytes (sizeof(ext2_inode_t))
    uint32_t inode_offset = local_inode_index * sizeof(ext2_inode_t);
    
    // Calculate which block within the inode table
    uint32_t block_offset = inode_offset / ext2_state.block_size;
    uint32_t offset_in_block = inode_offset % ext2_state.block_size;
    
    // Read the block containing the inode
    uint8_t *block_buf = (uint8_t*)kalloc(ext2_state.block_size);
    if (!block_buf) {
        kprintf_both("[ext2] ERROR: Failed to allocate inode read buffer\n");
        return -1;
    }
    
    if (ext2_read_block(inode_table_block + block_offset, block_buf) < 0) {
        kprintf_both("[ext2] ERROR: Failed to read inode table block\n");
        kfree(block_buf);
        return -1;
    }
    
    // Copy the inode data
    memcpy(inode, block_buf + offset_in_block, sizeof(ext2_inode_t));
    
    kfree(block_buf);
    
    klogf("[ext2] Read inode %u: size=%u, mode=0x%04x\n", 
          inode_num, inode->i_size, inode->i_mode);
    
    return 0;
}

/**
 * @brief Find inode by path
 */
static int ext2_find_inode_by_path(const char *path, uint32_t *inode_num) {
    if (!path || path[0] != '/') {
        kprintf_both("[ext2] ERROR: Path must be absolute\n");
        return -1;
    }
    
    // Start at root (inode 2)
    uint32_t current_inode = 2;
    
    if (strcmp(path, "/") == 0) {
        *inode_num = 2;
        return 0;
    }
    
    // Parse path
    char path_copy[256];
    strncpy(path_copy, path + 1, 255);
    path_copy[255] = '\0';
    
    char *token = strtok(path_copy, "/");
    while (token) {
        klogf("[ext2] Looking for '%s' in inode %u\n", token, current_inode);
        
        ext2_inode_t inode;
        if (ext2_read_inode(current_inode, &inode) < 0) {
            return -1;
        }
        
        if ((inode.i_mode & EXT2_S_IFDIR) == 0) {
            kprintf_both("[ext2] ERROR: Not a directory\n");
            return -1;
        }
        
        uint32_t found_inode = 0;
        if (ext2_search_directory(&inode, token, &found_inode) < 0) {
            kprintf_both("[ext2] ERROR: '%s' not found\n", token);
            return -1;
        }
        
        current_inode = found_inode;
        token = strtok(NULL, "/");
    }
    
    *inode_num = current_inode;
    return 0;
}

/**
 * @brief Search directory for filename
 */
static int ext2_search_directory(ext2_inode_t *dir_inode, const char *name, uint32_t *found_inode) {
    uint32_t dir_size = dir_inode->i_size;
    uint32_t bytes_read = 0;
    
    uint8_t *dir_buf = (uint8_t*)kalloc(dir_size);
    if (!dir_buf) {
        kprintf_both("[ext2] ERROR: Failed to allocate directory buffer\n");
        return -1;
    }
    
    if (ext2_read_inode_data(dir_inode, 0, dir_buf, dir_size) < 0) {
        kfree(dir_buf);
        return -1;
    }
    
    while (bytes_read < dir_size) {
        ext2_dirent_t *entry = (ext2_dirent_t*)(dir_buf + bytes_read);
        
        if (entry->inode == 0) {
            bytes_read += entry->rec_len;
            continue;
        }
        
        if (entry->name_len == strlen(name) && 
            memcmp(entry->name, name, entry->name_len) == 0) {
            *found_inode = entry->inode;
            klogf("[ext2] Found '%s' -> inode %u\n", name, entry->inode);
            kfree(dir_buf);
            return 0;
        }
        
        bytes_read += entry->rec_len;
    }
    
    kfree(dir_buf);
    return -1;
}

/**
 * @brief Read data from inode
 */
static int ext2_read_inode_data(ext2_inode_t *inode, uint32_t offset, void *buf, size_t count) {
    if (offset >= inode->i_size) {
        return 0;  // EOF
    }
    
    if (offset + count > inode->i_size) {
        count = inode->i_size - offset;
    }
    
    uint32_t bytes_read = 0;
    uint8_t *out = (uint8_t*)buf;
    
    uint8_t *block_buf = (uint8_t*)kalloc(ext2_state.block_size);
    if (!block_buf) {
        return -1;
    }
    
    while (bytes_read < count) {
        uint32_t file_block = (offset + bytes_read) / ext2_state.block_size;
        uint32_t offset_in_block = (offset + bytes_read) % ext2_state.block_size;
        uint32_t bytes_to_read = ext2_state.block_size - offset_in_block;
        
        if (bytes_to_read > count - bytes_read) {
            bytes_to_read = count - bytes_read;
        }
        
        uint32_t disk_block = ext2_get_block_number(inode, file_block);
        if (disk_block == 0) {
            memset(out + bytes_read, 0, bytes_to_read);
        } else {
            if (ext2_read_block(disk_block, block_buf) < 0) {
                kfree(block_buf);
                return -1;
            }
            memcpy(out + bytes_read, block_buf + offset_in_block, bytes_to_read);
        }
        
        bytes_read += bytes_to_read;
    }
    
    kfree(block_buf);
    return bytes_read;
}

/**
 * @brief Get disk block number for file block
 */
static uint32_t ext2_get_block_number(ext2_inode_t *inode, uint32_t file_block) {
    uint32_t ptrs_per_block = ext2_state.block_size / sizeof(uint32_t);
    
    // Direct blocks (0-11)
    if (file_block < 12) {
        return inode->i_block[file_block];
    }
    
    file_block -= 12;
    
    // Single indirect (12)
    if (file_block < ptrs_per_block) {
        uint32_t indirect_block = inode->i_block[12];
        if (indirect_block == 0) return 0;
        
        uint32_t *indirect_buf = (uint32_t*)kalloc(ext2_state.block_size);
        if (!indirect_buf) return 0;
        
        if (ext2_read_block(indirect_block, indirect_buf) < 0) {
            kfree(indirect_buf);
            return 0;
        }
        
        uint32_t block_num = indirect_buf[file_block];
        kfree(indirect_buf);
        return block_num;
    }
    
    // Double/triple indirect not implemented
    kprintf_both("[ext2] ERROR: Double/triple indirect not implemented\n");
    return 0;
}

// ----------------- Registration -----------------

int ext2_register(void) {
    kprintf_both("[ext2] Registering EXT2 filesystem driver...\n");
    int ret = vfs_register_fs(&ext2_ops);
    if (ret == 0) {
        kprintf_both("[ext2] Registration successful!\n");
        return 0;
    } else {
        kprintf_both("[ext2] Registration failed (code %d)\n", ret);
        return -1;
    }
}