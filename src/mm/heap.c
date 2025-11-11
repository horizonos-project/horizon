#include <stdint.h>
#include "../libk/kprint.h"
#include "mm.h"
#include "../kernel/log.h"

#define HEAP_INITIAL_PAGES 16  // 16 * 4KiB = 64KiB initial heap

static uint32_t heap_start = 0;
static uint32_t heap_end   = 0;
static uint32_t heap_curr  = 0;

void kheap_init(void) {
    kprintf("[heap] Initializing kernel heap...\n");

    heap_start = pmm_alloc_nframes(HEAP_INITIAL_PAGES);
    if (!heap_start) {
        klogf("[heap] Failed to allocate heap base!\n");
        while (1) __asm__("hlt");
    }

    heap_end  = heap_start + HEAP_INITIAL_PAGES * 4096;
    heap_curr = heap_start;

    kprintf("[heap] Heap region: 0x%08x - 0x%08x (%u KiB)\n",
            heap_start, heap_end, (heap_end - heap_start) / 1024);
}

// Simple bump allocator
void *kalloc(uint32_t size) {
    if (heap_curr + size > heap_end) {
        // TODO: grow heap later
        klogf("[heap] Out of memory!\n");
        return 0;
    }

    void *addr = (void *)heap_curr;
    heap_curr += (size + 7) & ~7; // align to 8 bytes
    return addr;
}

void kfree(void *ptr) {
    // no-op for now; bump allocator can't reclaim
    (void)ptr;
}
