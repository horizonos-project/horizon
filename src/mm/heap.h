#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

// Initialize kernel heap
void kheap_init(void);

// Allocate memory from kernel heap
// Returns: Virtual address, or NULL on failure
void* kalloc(size_t size);

// Free memory (stub for now - bump allocator doesn't support freeing)
void kfree(void *ptr);

// Get heap statistics
uint32_t kheap_get_used(void);
uint32_t kheap_get_size(void);

#endif // HEAP_H