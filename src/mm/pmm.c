#include <stdint.h>
#include "../kernel/multiboot.h"
#include "../libk/kprint.h"
#include "mm.h"
#include "mm/mboot.h"
#include "../kernel/log.h"

#define FRAME_SIZE 4096
#define MAX_FRAMES (128 * 1024 * 1024 / FRAME_SIZE)  // 32768 fr for 128 MiB

static uint8_t frame_bitmap[MAX_FRAMES / 8];

static uint32_t total_frames = 0;
static uint32_t used_frames  = 0;

#define BIT_SET(a,b)   ((a)[(b)/8] |=  (1 << ((b)%8)))
#define BIT_CLEAR(a,b) ((a)[(b)/8] &= ~(1 << ((b)%8)))
#define BIT_TEST(a,b)  ((a)[(b)/8] &   (1 << ((b)%8)))

static inline int pmm_is_free(uint32_t frame) { 
    return !BIT_TEST(frame_bitmap, frame);
}

static inline void pmm_mark_used(uint32_t frame) {
    if (frame >= MAX_FRAMES) return;
    if (pmm_is_free(frame)) {
        used_frames++;
        BIT_SET(frame_bitmap, frame);
    }
}

static inline void pmm_mark_free(uint32_t frame) { 
    if (frame >= MAX_FRAMES) return;
    if (!pmm_is_free(frame)) {
        used_frames--;
        BIT_CLEAR(frame_bitmap, frame);
    }
}

void pmm_init(multiboot_info_t *mb) {
    kprintf("\n[pmm] Initalizing Physical Memory Manager...\n");

    for (uint32_t i = 0; i < sizeof(frame_bitmap); i++) {
        frame_bitmap[i] = 0xFF;
    }

    total_frames = 0;
    used_frames  = MAX_FRAMES;

    if (!(mb->flags & MB_INFO_MMAP)) {
        kprintf("[pmm] No memory map provided by bootloader!\n");
        goto no_mem_hang;
    }

    multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t*)(uintptr_t)mb->mmap_addr;
    uint32_t mmap_end = mb->mmap_addr + mb->mmap_length;

    // Do I like doing pointer-arith? No! Does this work? Yeah! So it stays
    for (; (uint32_t)mmap < mmap_end; mmap = (multiboot_mmap_entry_t*)((uint32_t)mmap + mmap->size + sizeof(mmap->size))) {
        uint64_t start = mmap->addr;
        uint64_t end   = mmap->addr + mmap->length;

        if (mmap->type == 1) { // Available RAM
            for (uint64_t addr = start; addr + FRAME_SIZE <= end; addr += FRAME_SIZE) {
                uint32_t frame = (uint32_t)(addr / FRAME_SIZE);
                if (frame < MAX_FRAMES) {
                    if (!pmm_is_free(frame)) {
                        // was reserved in initial state - now free
                        BIT_CLEAR(frame_bitmap, frame);
                        used_frames--;
                    }
                    total_frames++;
                }
            }

            kprintf("[pmm] Free region: 0x%08x - 0x%08x (%u KiB)\n",
                (uint32_t)start, (uint32_t)end, (uint32_t)(mmap->length / 1024));
        } else {
            kprintf("[pmm] Reserved: 0x%08x - 0x%08x\n",
                (uint32_t)mmap->addr, (uint32_t)(mmap->addr + mmap->length));
        }
    }

    // Reserving ~1MiB for legacy purposes (and it causes bugs all over)
    kprintf("[pmm] Reserving low 1 MiB region...\n");
    for (uint32_t addr = 0; addr < 0x00100000; addr += FRAME_SIZE) {
        uint32_t frame = addr / FRAME_SIZE;
        pmm_mark_used(frame);
    }

    // Reserving the kernel memory space to avoid... y'know, overwriting it.
    uint32_t kern_start_aligned = (uint32_t)&kernel_start & ~(FRAME_SIZE - 1);
    uint32_t kern_end_aligned   = ((uint32_t)&kernel_end + FRAME_SIZE - 1) & ~(FRAME_SIZE - 1);

    kprintf("[pmm] Reserving kernel: 0x%08x - 0x%08x (%u KiB)\n",
            kern_start_aligned, kern_end_aligned,
            (kern_end_aligned - kern_start_aligned) / 1024);

    for (uint32_t addr = kern_start_aligned; addr < kern_end_aligned; addr += FRAME_SIZE) {
        uint32_t frame = addr / FRAME_SIZE;
        pmm_mark_used(frame);
    }

    kprintf("[pmm] Initialization complete! %u total frames, %u used\n",
        total_frames, used_frames);
    goto ok;

no_mem_hang:
    klogf("Failed to initalize PMM! System halted! (PHYSICAL MEM FAILURE)\n");
    while (1) { __asm__ volatile("hlt"); }
ok:
    return;
}

// ----------------------------------------------------------------------------
// Publicly usable API thing
// ----------------------------------------------------------------------------

uint32_t pmm_alloc_frame(void) {
    for (uint32_t frame = 0; frame < MAX_FRAMES; frame++) {
        if (pmm_is_free(frame)) {
            pmm_mark_used(frame);
            return frame * FRAME_SIZE;
        }
    }

    // No memory (typically bad for the economy)
    klogf("[pmm] Out of physical memory!\n");
    return 0;
}

void pmm_free_frame(uint32_t addr) {
    if (addr % FRAME_SIZE != 0) {
        klogf("[pmm] pmm_free_frame: unaligned addr 0x%08x\n", addr);
        return;
    }

    uint32_t frame = addr / FRAME_SIZE;
    if (frame >= MAX_FRAMES) {
        klogf("[pmm] pmm_free_frame: addr out of range 0x%08x\n", addr);
        return;
    }

    pmm_mark_free(frame);
}

uint32_t pmm_alloc_nframes(uint32_t count) {
    if (count == 0) return 0;

    uint32_t found = 0;
    uint32_t start_frame = 0;

    for (uint32_t frame = 0; frame < MAX_FRAMES; frame++) {
        if (pmm_is_free(frame)) {
            if (found == 0)
                start_frame = frame;
            if (++found == count) {
                for (uint32_t i = 0; i < count; i++)
                    pmm_mark_used(start_frame + i);
                return start_frame * FRAME_SIZE;
            }
        } else {
            found = 0;
        }
    }

    klogf("[pmm] Out of contiguous frames (%u)\n", count);
    return 0;
}

// Debugging, very helpful
void pmm_dump_stats(void) {
    kprintf("[pmm] Stats: total=%u used=%u\n", total_frames, used_frames);
}
