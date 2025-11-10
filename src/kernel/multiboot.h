#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>
#include "log.h"

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

void display_mb_info(multiboot_info_t *mb) {
    kprintf_both("[mb] - Multiboot Information\n");

    if (mb->flags & MB_INFO_MEM) {
        kprintf_both("[mb] Lower memory: %u KB\n", mb->mem_lower);
        kprintf_both("[mb] Upper memory: %u KB\n", mb->mem_upper);
    } else {
        kprintf_both("[mb] No memory info provided.\n");
    }

    if (mb->flags & MB_INFO_BOOT_DEVICE) {
        kprintf_both("[mb] Boot device: 0x%08x\n", mb->boot_device);
    }

    if (mb->flags & MB_INFO_CMDLINE) {
        const char *cmd = (const char *)(uintptr_t)mb->cmdline;
        kprintf_both("[mb] Cmdline: %s\n", cmd ? cmd : "(none)");
    }

    if (mb->flags & MB_INFO_MODS) {
        kprintf_both("[mb] Modules count: %u\n", mb->mods_count);
        kprintf_both("[mb] Modules addr:  0x%08x\n", mb->mods_addr);
    }

    if (mb->flags & MB_INFO_MMAP) {
        kprintf_both("[mb] Memory map: length=%u, addr=0x%08x\n", 
                mb->mmap_length, mb->mmap_addr);
    }

    kprintf_both("[mb] - End Multiboot Information\n");
}

#endif
