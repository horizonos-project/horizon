#ifndef INITRAMFS_H
#define INITRAMFS_H

#include <stdint.h>

// tar header (ustar format)
struct tar_header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];      // "ustar\0"
    char version[2];    // "00"
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
} __attribute__((packed));

void initramfs_init(void);

#endif // INITRAMFS_H