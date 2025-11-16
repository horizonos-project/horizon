#include "vmm.h"
#include "kernel/log.h"
#include "pmm.h"
#include "../libk//kprint.h"
#include "../libk/string.h"
#include "../kernel/panic.h"

typedef struct {
    uint32_t entries[1024];
} page_table_t;

typedef struct {
    uint32_t entries[1024];
} page_directory_t;

// Kernel page directory (identity-mapped)
static page_directory_t *kernel_directory = NULL;

// Helper: Get page table for a virtual address, creating if needed
static page_table_t* get_page_table(uint32_t virt, bool create) {
    uint32_t dir_index = virt >> 22;
    
    // Check if page table exists
    if (kernel_directory->entries[dir_index] & PAGE_PRESENT) {
        // Extract physical address of page table
        uint32_t table_phys = kernel_directory->entries[dir_index] & ~0xFFF;
        return (page_table_t*)table_phys;
    }
    
    // Create new page table if requested
    if (create) {
        void *table_phys = pmm_alloc_frame();
        if (!table_phys) {
            panicf("[vmm] ERROR: Failed to allocate page table\n");
            return NULL;
        }
        
        // Clear the page table
        memset((void*)table_phys, 0, sizeof(page_table_t));
        
        kernel_directory->entries[dir_index] = 
            (uint32_t)table_phys | PAGE_PRESENT | PAGE_RW;
        
        kprintf("[vmm] Created page table at 0x%08x for virt 0x%08x\n",
              (uint32_t)table_phys, virt);
        
        return (page_table_t*)table_phys;
    }
    
    return NULL;
}

void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    // Align addresses
    virt &= ~0xFFF;
    phys &= ~0xFFF;
    
    page_table_t *table = get_page_table(virt, true);
    if (!table) {
        kprintf("vmm_map_page: Failed to get page table for 0x%08x", virt);
    }
    
    uint32_t table_index = (virt >> 12) & 0x3FF;
    
    table->entries[table_index] = phys | (flags & 0xFFF);
    
    // Invalidate TLB for this page
    __asm__ volatile("invlpg (%0)" :: "r"(virt) : "memory");
}

void vmm_unmap_page(uint32_t virt) {
    virt &= ~0xFFF;
    
    page_table_t *table = get_page_table(virt, false);
    if (!table) {
        return;  // Not mapped
    }
    
    uint32_t table_index = (virt >> 12) & 0x3FF;
    table->entries[table_index] = 0;
    
    // Invalidate TLB
    __asm__ volatile("invlpg (%0)" :: "r"(virt) : "memory");
}

void* vmm_alloc_page(uint32_t virt, uint32_t flags) {
    void *phys = pmm_alloc_frame();
    if (!phys) {
        kprintf("[vmm] ERROR: Failed to allocate physical frame\n");
        return NULL;
    }
    
    // Map the phys frame (if we have it)
    vmm_map_page(virt, (uint32_t)phys, flags);
    
    return (void*)virt;
}

void vmm_free_page(uint32_t virt) {
    virt &= ~0xFFF;
    
    // Get physaddr
    uint32_t phys = vmm_get_physical(virt);
    if (phys == 0) {
        return;  // Not mapped (rip)
    }

    vmm_unmap_page(virt);

    pmm_free_frame((void*)phys);
}

uint32_t vmm_get_physical(uint32_t virt) {
    page_table_t *table = get_page_table(virt, false);
    if (!table) {
        return 0;  // Not mapped
    }
    
    uint32_t table_index = (virt >> 12) & 0x3FF;
    uint32_t entry = table->entries[table_index];
    
    if (!(entry & PAGE_PRESENT)) {
        return 0;  // Not present
    }
    
    // Return physical address + offset within page
    return (entry & ~0xFFF) | (virt & 0xFFF);
}

bool vmm_is_mapped(uint32_t virt) {
    return vmm_get_physical(virt) != 0;
}

void vmm_init(void) {
    kprintf_both("[vmm], Initalizing Virtual Memory Manager...\n");

    kernel_directory = (page_directory_t*)pmm_alloc_frame();
    memset(kernel_directory, 0, sizeof(page_directory_t));

    kprintf_both("[vmm] Identity mapping 0 -> 16 MB...\n");

    for (uint32_t addr = 0; addr < 16 * 1024 * 1024; addr += PAGE_SIZE) {
        vmm_map_page(addr, addr, PAGE_PRESENT | PAGE_RW);
    }

    kprintf_both("[vmm] Identity mapping complete!\n");

    // Load page directory into CR3
    __asm__ volatile("mov %0, %%cr3" :: "r"(kernel_directory) : "memory");
    
    // Enable paging (set PG bit in CR0)
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0) : "memory");
    
    klogf("[vmm] Paging enabled (CR0.PG set)\n");
    klogf("[vmm] Virtual Memory Manager initialized\n");
}