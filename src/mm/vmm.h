#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE 4096

// Page flags
#define PAGE_PRESENT  0x001
#define PAGE_RW       0x002
#define PAGE_USER     0x004
#define PAGE_ACCESSED 0x020
#define PAGE_DIRTY    0x040

/**
 * @brief Initialize VMM (creates kernel page directory, identity maps low memory, enables paging)
 */
void vmm_init(void);

/**
 * @brief Map a virtual address to a physical address
 * 
 * @param virt Virtual address to map (will be page-aligned)
 * @param phys Physical address to map to (should be from pmm_alloc_frame())
 * @param flags Page flags (PAGE_PRESENT | PAGE_RW | etc.)
 */
void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags);

/**
 * @brief Unmap a virtual address
 * 
 * @param virt Address to unmap. Does NOT free the physical frame. call pmm_free_frame() separately if needed
 */
void vmm_unmap_page(uint32_t virt);

/**
 * @brief Allocate a virtual page (allocates physical frame, maps it, returns virtual address)
 * 
 * @param virt Virtual address to map at
 * @param flags Page flags
 * @return void* The virtual address, or NULL on failure
 */
void* vmm_alloc_page(uint32_t virt, uint32_t flags);

/**
 * @brief Free a virtual page (unmaps and frees underlying physical frame)
 * 
 * @param virt Virtual address to free
 */
void vmm_free_page(uint32_t virt);

/**
 * @brief Get physical address for a virtual address
 * 
 * @param virt Virtual address
 * @return uint32_t Physical address, or 0 if not mapped
 */
uint32_t vmm_get_physical(uint32_t virt);

/**
 * @brief Checks if the virtual address provided is mapped to physical mem
 * 
 * @param virt Virtual address
 * @return true The address is mapped
 * @return false The address is NOT mapped
 */
bool vmm_is_mapped(uint32_t virt);

#endif // VMM_H