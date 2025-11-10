// src/drivers/fs/ext2.c
//
// ext2 filesystem driver implementation for Horizon
// (c) 2025 HorizonOS Project
//

#include <stdint.h>
#include <stddef.h>
#include "../vfs/vfs.h"
#include "kernel/utilities/assert.h"
#include "libk/kprint.h"

#define EXT2_SUPER_MAGIC 0xEF53

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

// FSOps table

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

// --------------------- Dummy implementations ---------------------

static int ext2_init(void) {
    kprintf("\n[ext2] init()\n");
    return 0;
}

static int ext2_mount(const char *device) {
    kprintf("\n[ext2] mount('%s')\n", device ? device : "(null)");
    // For now, pretend success
    return 0;
}

static void ext2_unmount(void) {
    kprintf("\n[ext2] unmount()\n");
}

static int ext2_open(const char *path, int flags, file_t *file) {
    kprintf("\n[ext2] open('%s', flags=%d)\n", path, flags);
    return -1; // ENOENT-style stub
}

static int ext2_close(file_t *file) {
    (void)file;
    kprintf("\n[ext2] close()\n");
    return 0;
}

static int ext2_read(file_t *file, void *buf, size_t count) {
    (void)file; (void)buf; (void)count;
    kprintf("\n[ext2] read() not implemented\n");
    return 0;
}

static int ext2_write(file_t *file, const void *buf, size_t count) {
    (void)file; (void)buf; (void)count;
    kprintf("\n[ext2] write() not implemented\n");
    return 0;
}

static int ext2_readdir(file_t *dir, dirent_t *entry) {
    (void)dir; (void)entry;
    kprintf("\n[ext2] readdir() not implemented\n");
    return 0;
}

static int ext2_stat(const char *path, stat_t *st) {
    (void)path; (void)st;
    kprintf("\n[ext2] stat() not implemented\n");
    return -1;
}

// Registration
int ext2_register() {
    kprintf("\n[ext2] Registering EXT2 filesystem...\n");
    int ret = vfs_register_fs(&ext2_ops);
    if (ret == 0) {
        kprintf("[ext2] Registration successful!\n");
        return 0;
    } else {
        kprintf("[ext2] Registration failed (code %d)\n", ret);
        return -1;
    }
}