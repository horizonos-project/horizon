#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stdbool.h>

#define FRAME_SIZE 4096

// Initialize physical memory manager with multiboot info
void pmm_init(void *mboot_info);

// Allocate a single physical frame
// Returns: PHYSICAL address of frame, or NULL if out of memory
void* pmm_alloc_frame(void);

// Free a physical frame
// phys_addr: PHYSICAL address of frame to free
void pmm_free_frame(void *phys_addr);

// Mark a specific frame as used (for reserving kernel, multiboot structures, etc.)
void pmm_mark_used(uint32_t frame_number);

// Mark a specific frame as free
void pmm_mark_free(uint32_t frame_number);

// Get statistics
uint32_t pmm_get_total_frames(void);
uint32_t pmm_get_free_frames(void);
uint32_t pmm_get_used_frames(void);

// Print statistics
void pmm_dump_stats(void);

#endif // PMM_H