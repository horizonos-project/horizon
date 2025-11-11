#include <stdint.h>
#include "../libk/string.h"
#include "../libk/kprint.h"
#include "../kernel/log.h"
#include "../kernel/panic.h"
#include "mm.h"

static page_directory_t* kernel_page_directory = 0;

// Assembly helpers for enabling this at the hardware level

void vmm_switch_directory(page_directory_t* dir) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(dir));
}

void vmm_enable_paging(void) {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  // Set PG bit
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));
}

void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t dir_idx = virt >> 22;
    uint32_t tbl_idx = (virt >> 12) & 0x3FF;

    page_table_t *table;

    if (kernel_page_directory->entries[dir_idx] & PAGE_PRESENT) {
        table = (page_table_t*)(kernel_page_directory->entries[dir_idx] & ~0xFFF);
    } else {
        table = (page_table_t*)pmm_alloc_frame();
        if (!table)
            panicf("Failed to allocate page table!");

        memset(table, 0, sizeof(page_table_t));
        kernel_page_directory->entries[dir_idx] =
            ((uint32_t)table) | PAGE_PRESENT | PAGE_RW;
    }

    table->entries[tbl_idx] = (phys & ~0xFFF) | (flags & 0xFFF);
}


void vmm_init(void) {
    kprintf("[vmm] Initializing virtual memory...\n");

    // Allocate page directory
    kernel_page_directory = (page_directory_t*)pmm_alloc_frame();
    if (!kernel_page_directory)
        panicf("Failed to allocate kernel page directory!");

    memset(kernel_page_directory, 0, sizeof(page_directory_t));

    // Identity map first 16 MiB (enough for kernel + heap)
    // We may need more later, this is fine for now
    for (uint32_t addr = 0; addr < 16 * 1024 * 1024; addr += PAGE_SIZE)
        vmm_map_page(addr, addr, PAGE_PRESENT | PAGE_RW);

    // Load page directory into CR3 and enable paging
    vmm_switch_directory(kernel_page_directory);
    vmm_enable_paging();

    kprintf("[vmm] Paging enabled.\n");
}

