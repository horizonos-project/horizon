/**
 * @file pmm.h
 * @brief Physical Memory Manager
 * 
 * Manages allocation and deallocation of physical memory frames.
 * The PMM operates on 4KB frames and tracks which physical memory
 * regions are available for use by the kernel and user processes.
 * 
 * @note This module works with PHYSICAL addresses, not virtual addresses.
 */

#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stdbool.h>

/** @brief Size of a single physical frame in bytes (4KB) */
#define FRAME_SIZE 4096

/**
 * @brief Initialize the physical memory manager
 * 
 * Parses the multiboot memory map to determine available physical
 * memory regions and sets up the frame bitmap for tracking allocation.
 * Must be called early in the boot process before any memory allocation.
 * 
 * @param mboot_info Pointer to multiboot information structure
 */
void pmm_init(void *mboot_info);

/**
 * @brief Allocate a single physical frame
 * 
 * Finds and allocates one 4KB physical memory frame from the pool
 * of available frames. The frame is marked as used in the bitmap.
 * 
 * @return Physical address of the allocated frame, or NULL if out of memory
 * @warning Returned address is PHYSICAL, not virtual
 */
void* pmm_alloc_frame(void);

/**
 * @brief Free a physical frame
 * 
 * Returns a previously allocated frame to the pool of available frames.
 * The frame is marked as free in the bitmap and can be allocated again.
 * 
 * @param phys_addr Physical address of the frame to free (must be frame-aligned)
 * @warning Must pass a PHYSICAL address, not virtual
 */
void pmm_free_frame(void *phys_addr);

/**
 * @brief Mark a specific frame as used
 * 
 * Manually marks a frame as allocated in the bitmap. Used during
 * initialization to reserve frames for the kernel, multiboot structures,
 * and other critical regions that should not be allocated.
 * 
 * @param frame_number Frame number (physical_address / FRAME_SIZE)
 */
void pmm_mark_used(uint32_t frame_number);

/**
 * @brief Mark a specific frame as free
 * 
 * Manually marks a frame as available in the bitmap. Use with caution
 * as incorrectly freeing in-use frames can cause system instability.
 * 
 * @param frame_number Frame number (physical_address / FRAME_SIZE)
 */
void pmm_mark_free(uint32_t frame_number);

/**
 * @brief Get total number of frames in the system
 * 
 * @return Total count of physical memory frames (both free and used)
 */
uint32_t pmm_get_total_frames(void);

/**
 * @brief Get number of free frames available
 * 
 * @return Count of unallocated frames currently available for allocation
 */
uint32_t pmm_get_free_frames(void);

/**
 * @brief Get number of frames currently in use
 * 
 * @return Count of allocated frames (total - free)
 */
uint32_t pmm_get_used_frames(void);

/**
 * @brief Print memory statistics to kernel log
 * 
 * Displays information about total, free, and used frames,
 * as well as the percentage of memory utilization.
 */
void pmm_dump_stats(void);

#endif // PMM_H