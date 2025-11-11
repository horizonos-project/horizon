#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

#define MULTIBOOT_MAGIC        0x2BADB002

// Bit flags in the multiboot_info->flags field
#define MB_INFO_MEM            (1 << 0)
#define MB_INFO_BOOT_DEVICE    (1 << 1)
#define MB_INFO_CMDLINE        (1 << 2)
#define MB_INFO_MODS           (1 << 3)
#define MB_INFO_MMAP           (1 << 6)

typedef struct multiboot_info {
    uint32_t flags;         // Indicates which fields are valid

    // Memory info (only valid if flags[0] set)
    uint32_t mem_lower;
    uint32_t mem_upper;

    // Boot device (valid if flags[1] set)
    uint32_t boot_device;

    // Kernel command line (valid if flags[2] set)
    uint32_t cmdline;

    // Modules (valid if flags[3] set)
    uint32_t mods_count;
    uint32_t mods_addr;

    // ELF section headers (skip for now)
    uint32_t syms[4];

    // Memory map (valid if flags[6] set)
    uint32_t mmap_length;
    uint32_t mmap_addr;
} __attribute__((packed)) multiboot_info_t;

void display_mb_info(multiboot_info_t *mb);

#endif
