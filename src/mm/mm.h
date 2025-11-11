#ifndef MM_H
#define MM_H

#include <stdint.h>
#include "../kernel/multiboot.h"

extern uint32_t kernel_start[];
extern uint32_t kernel_end[];

// Physical Memory Management

void pmm_init(multiboot_info_t *mb);
uint32_t pmm_alloc_frame(void);
uint32_t pmm_alloc_nframes(uint32_t count);
void pmm_free_frame(uint32_t addr);
void pmm_dump_stats(void);

// Virtual Memory Management

#define PAGE_SIZE 4096
#define PAGE_ENTRIES 1024  // 1024 * 4 bytes = 4 KiB per table
#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2
#define PAGE_USER    0x4

typedef uint32_t page_entry_t;
typedef struct {
    page_entry_t entries[1024];
} page_table_t;

typedef struct {
    page_entry_t entries[1024];
} page_directory_t;

void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags);
void vmm_enable_paging(void);
void vmm_switch_directory(page_directory_t* dir);
void vmm_init(void);

// Heap

#endif // MM_H