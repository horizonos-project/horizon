#include "heap.h"
#include "kernel/log.h"
#include "vmm.h"

// Heap configuration
#define HEAP_START      0x10000000  // Start at 256 MB virtual
#define HEAP_MAX_SIZE   (64 * 1024 * 1024)  // Max 64 MB

// Heap state
static uint32_t heap_start = 0;
static uint32_t heap_end = 0;
static uint32_t heap_current = 0;

void kheap_init(void) {
    klogf("[heap] Initializing kernel heap...\n");
    
    heap_start = HEAP_START;
    heap_current = heap_start;
    heap_end = heap_start;  // Will grow on demand
    
    klogf("[heap] Heap virtual address: 0x%08x\n", heap_start);
    klogf("[heap] Maximum size: %u MB\n", HEAP_MAX_SIZE / (1024 * 1024));
    klogf("[heap] Kernel heap initialized\n");
}

void* kalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    // Align to 8 bytes
    size = (size + 7) & ~7;
    
    // Check if we need more pages
    while (heap_current + size > heap_end) {
        // Check max size
        if (heap_end >= heap_start + HEAP_MAX_SIZE) {
            klogf("[heap] ERROR: Heap exhausted (max %u MB reached)\n",
                  HEAP_MAX_SIZE / (1024 * 1024));
            return NULL;
        }
        
        // Allocate and map one more page
        if (!vmm_alloc_page(heap_end, PAGE_PRESENT | PAGE_RW)) {
            klogf("[heap] ERROR: Failed to allocate page for heap\n");
            return NULL;
        }
        
        heap_end += PAGE_SIZE;
        
        // Log when we grow (but not too spammy)
        static uint32_t last_log_size = 0;
        uint32_t current_size = heap_end - heap_start;
        if (current_size - last_log_size >= 64 * 1024) {  // Log every 64KB
            klogf("[heap] Grew to %u KB\n", current_size / 1024);
            last_log_size = current_size;
        }
    }
    
    // Allocate from bump pointer
    void *ptr = (void*)heap_current;
    heap_current += size;
    
    return ptr;
}

void kfree(void *ptr) {
    // Bump allocator doesn't support freeing
    // Not yet at least, we'll get around to it.
    (void)ptr;
}

uint32_t kheap_get_used(void) {
    return heap_current - heap_start;
}

uint32_t kheap_get_size(void) {
    return heap_end - heap_start;
}