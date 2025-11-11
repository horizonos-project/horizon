#ifndef MM_H
#define MM_H

#include <stdint.h>
#include "../kernel/multiboot.h"

extern uint32_t kernel_start[];
extern uint32_t kernel_end[];

// Physical Memory Management

void pmm_init(multiboot_info_t *mb);
uint32_t pmm_alloc_frame(void);
void pmm_free_frame(uint32_t addr);

// Virtual Memory Management

// Heap

#endif // MM_H