#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

void kheap_init(void);

// Allocate memory from kernel heap
// Returns virtual address, or NULL on failure
void* kalloc(size_t size);

// Free memory (not implemented in bump allocator, but API exists)
void kfree(void *ptr);

#endif // HEAP_H