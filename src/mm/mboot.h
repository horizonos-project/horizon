#ifndef MBOOT_MMAP_H
#define MBOOT_MMAP_H

#include <stdint.h>

typedef struct {
    uint32_t size;
    uint64_t addr;
    uint64_t length;
    uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

#endif