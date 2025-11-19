/**
 * @file heap.h
 * @brief Kernel Heap Allocator
 * 
 * Provides dynamic memory allocation for the kernel through kalloc/kfree.
 * Currently implements a simple bump allocator that grows upward from a
 * base address. Free operations are stubbed (memory is not reclaimed).
 * 
 * @note This is a temporary implementation. A proper heap with free support
 *       (e.g., linked list or buddy allocator) should be implemented later.
 * 
 * @warning kfree() is currently a no-op - memory cannot be reclaimed!
 */

#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Initialize the kernel heap
 * 
 * Sets up the kernel heap allocator with an initial memory region.
 * Must be called after the VMM is initialized and before any kalloc() calls.
 * 
 * @note Currently uses a bump allocator strategy
 */
void kheap_init(void);

/**
 * @brief Allocate memory from the kernel heap
 * 
 * Allocates a block of at least 'size' bytes from the kernel heap.
 * The returned pointer is aligned and points to virtual memory.
 * 
 * @param size Number of bytes to allocate
 * @return Virtual address of allocated memory, or NULL on failure
 * 
 * @warning Memory allocated with kalloc() cannot currently be freed!
 */
void* kalloc(size_t size);

/**
 * @brief Free allocated memory
 * 
 * @param ptr Pointer to memory previously allocated with kalloc()
 * 
 * @warning STUB IMPLEMENTATION - Does nothing! The bump allocator
 *          does not support freeing memory. This function exists
 *          for API compatibility with future heap implementations.
 * 
 * @todo Implement proper heap with free support
 */
void kfree(void *ptr);

/**
 * @brief Get amount of heap memory currently allocated
 * 
 * @return Number of bytes allocated from the heap (since init)
 */
uint32_t kheap_get_used(void);

/**
 * @brief Get total size of the heap
 * 
 * @return Total heap size in bytes (maximum addressable space)
 */
uint32_t kheap_get_size(void);

#endif // HEAP_H