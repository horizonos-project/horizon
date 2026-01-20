#include "pmm.h"
#include "../kernel/multiboot.h"
#include "../libk/kprint.h"
#include "../libk/string.h"
#include "kernel/log.h"
#include "mm/mboot.h"
#include <stdint.h>

// Bitmap allocator: 1 bit per frame
// 0 = free, 1 = used
#define MAX_MEMORY (1024 * 1024 * 1024)  // Support up to 1GB (yay)
#define MAX_FRAMES (MAX_MEMORY / FRAME_SIZE)
#define BITMAP_SIZE (MAX_FRAMES / 8)

static uint8_t frame_bitmap[BITMAP_SIZE];
static uint32_t total_frames = 0;
static uint32_t used_frames = 0;

static inline bool test_bit(uint32_t frame) {
    return frame_bitmap[frame / 8] & (1 << (frame % 8));
}

static inline void set_bit(uint32_t frame) {
    frame_bitmap[frame / 8] |= (1 << (frame % 8));
}

static inline void clear_bit(uint32_t frame) {
    frame_bitmap[frame / 8] &= ~(1 << (frame % 8));
}

void pmm_mark_used(uint32_t frame) {
    if (frame >= MAX_FRAMES) return;
    
    if (!test_bit(frame)) {
        set_bit(frame);
        used_frames++;
    }
}

void pmm_mark_free(uint32_t frame) {
    if (frame >= MAX_FRAMES) return;
    
    if (test_bit(frame)) {
        clear_bit(frame);
        used_frames--;
    }
}

void *pmm_alloc_frame(void) {
    //Finding first free frame
    for (uint32_t i = 0; i < MAX_FRAMES; i++) {
        if (!test_bit(i)) {
            pmm_mark_used(i);
            return (void*)(i * FRAME_SIZE);
        }
    }
    
    // Out of memory (still bad for the economy)
    return NULL;
}

void pmm_free_frame(void *phys_addr) {
    uint32_t frame = (uint32_t)phys_addr / FRAME_SIZE;
    pmm_mark_free(frame);
}

void pmm_init(void *mboot_ptr) {
    multiboot_info_t *mb = (multiboot_info_t*)mboot_ptr;
    
    klogf("[pmm] Initializing Physical Memory Manager...\n");
    
    // Mark all frames as used initially
    memset(frame_bitmap, 0xFF, BITMAP_SIZE);
    total_frames = 0;
    used_frames = 0;
    
    // Check for memory map
    if (!(mb->flags & MB_INFO_MMAP)) {
        klogf("[pmm] ERROR: No memory map provided by bootloader!\n");
        return;
    }
    
    // Parse multiboot memory map
    multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t*)(uintptr_t)mb->mmap_addr;
    uint32_t mmap_end = mb->mmap_addr + mb->mmap_length;
    
    klogf("[pmm] Parsing memory map...\n");
    
    while ((uint32_t)mmap < mmap_end) {
        if (mmap->type == 1) {  // Type 1 = Available RAM
            uint64_t start = mmap->addr;
            uint64_t end = mmap->addr + mmap->length;
            
            klogf("[pmm] Free region: 0x%08x - 0x%08x (%u KiB)\n",
                  (uint32_t)start, (uint32_t)end, 
                  (uint32_t)(mmap->length / 1024));
            
            // Mark frames in this region as free
            for (uint64_t addr = start; addr < end; addr += FRAME_SIZE) {
                uint32_t frame = addr / FRAME_SIZE;
                if (frame < MAX_FRAMES) {
                    if (total_frames < frame + 1) {
                        total_frames = frame + 1;
                    }
                    pmm_mark_free(frame);
                }
            }
        } else {
            klogf("[pmm] Reserved: 0x%08x - 0x%08x (type %u)\n",
                  (uint32_t)mmap->addr, 
                  (uint32_t)(mmap->addr + mmap->length),
                  (uint32_t)mmap->type);
        }
        
        // Move to next entry (variable size)
        mmap = (multiboot_mmap_entry_t*)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }
    
    // Reserve kernel memory
    extern uint8_t kernel_start[], kernel_end[];
    uint32_t kernel_start_addr = (uint32_t)kernel_start;
    uint32_t kernel_end_addr = (uint32_t)kernel_end;
    
    // Align kernel end up to next frame
    kernel_end_addr = (kernel_end_addr + FRAME_SIZE - 1) & ~(FRAME_SIZE - 1);
    
    klogf("[pmm] Reserving kernel: 0x%08x - 0x%08x (%u KiB)\n",
          kernel_start_addr, kernel_end_addr,
          (kernel_end_addr - kernel_start_addr) / 1024);
    
    for (uint32_t addr = kernel_start_addr; addr < kernel_end_addr; addr += FRAME_SIZE) {
        pmm_mark_used(addr / FRAME_SIZE);
    }
    
    // Reserve multiboot info structure
    uint32_t mb_start = (uint32_t)mb & ~(FRAME_SIZE - 1);
    uint32_t mb_end = ((uint32_t)mb + sizeof(multiboot_info_t) + FRAME_SIZE - 1) & ~(FRAME_SIZE - 1);
    
    klogf("[pmm] Reserving multiboot info: 0x%08x - 0x%08x\n", mb_start, mb_end);
    for (uint32_t addr = mb_start; addr < mb_end; addr += FRAME_SIZE) {
        pmm_mark_used(addr / FRAME_SIZE);
    }
    
    // Reserve memory map itself
    if (mb->flags & MB_INFO_MMAP) {
        uint32_t mmap_start = mb->mmap_addr & ~(FRAME_SIZE - 1);
        uint32_t mmap_end_addr = (mb->mmap_addr + mb->mmap_length + FRAME_SIZE - 1) & ~(FRAME_SIZE - 1);
        
        klogf("[pmm] Reserving memory map: 0x%08x - 0x%08x\n", mmap_start, mmap_end_addr);
        for (uint32_t addr = mmap_start; addr < mmap_end_addr; addr += FRAME_SIZE) {
            pmm_mark_used(addr / FRAME_SIZE);
        }
    }
    
    klogf("[pmm] Initialization complete\n");
}

uint32_t pmm_get_total_frames(void) {
    return total_frames;
}

uint32_t pmm_get_free_frames(void) {
    return total_frames - used_frames;
}

uint32_t pmm_get_used_frames(void) {
    return used_frames;
}

// Now available in MB and KB! Yay!
void pmm_dump_stats(void) {
    uint32_t actual_used = 0;
    for (uint32_t i = 0; i < total_frames; i++) {
        if (test_bit(i)) {
            actual_used++;
        }
    }
    
    uint32_t actual_free = total_frames - actual_used;
    
    uint32_t total_kb = (total_frames * FRAME_SIZE) / 1024;
    uint32_t used_kb = (actual_used * FRAME_SIZE) / 1024;
    uint32_t free_kb = (actual_free * FRAME_SIZE) / 1024;
    
    klogf("[pmm] ===== Memory Statistics =====\n");
    klogf("[pmm] Total: %u frames (%u KB | %u MB)\n", 
          total_frames, total_kb, total_kb / 1024);
    klogf("[pmm] Used:  %u frames (%u KB | %u MB)\n", 
          actual_used, used_kb, used_kb / 1024);
    klogf("[pmm] Free:  %u frames (%u KB | %u MB)\n", 
          actual_free, free_kb, free_kb / 1024);
    klogf("[pmm] ==============================\n");
}