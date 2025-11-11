#include <stdint.h>
#include "../kernel/multiboot.h"
#include "../libk/kprint.h"
#include "mm.h"
#include "mm/mboot.h"
#include "../kernel/log.h"

#define FRAME_SIZE 4096
#define MAX_FRAMES (128 * 1024 * 1024 / FRAME_SIZE)  // 128 MiB / 4 KiB = 32768

static uint8_t frame_bitmap[MAX_FRAMES / 8];

#define BIT_SET(a,b)   ((a)[(b)/8] |=  (1 << ((b)%8)))
#define BIT_CLEAR(a,b) ((a)[(b)/8] &= ~(1 << ((b)%8)))
#define BIT_TEST(a,b)  ((a)[(b)/8] &   (1 << ((b)%8)))

static inline void pmm_mark_used(uint32_t frame) { BIT_SET(frame_bitmap, frame); }
static inline void pmm_mark_free(uint32_t frame) { BIT_CLEAR(frame_bitmap, frame); }

void pmm_init(multiboot_info_t *mb) {
    kprintf("\n[pmm] Initalizing Physical Memory Manager...\n");

    for (uint32_t i = 0; i < sizeof(frame_bitmap); i++) {
        frame_bitmap[i] = 0xFF;
    }

    if (!(mb->flags & MB_INFO_MMAP)) {
        kprintf("[pmm] No memory map provided by bootloader!\n");
        goto no_mem_hang;
    }

    multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t*)(uintptr_t)mb->mmap_addr;
    uint32_t mmap_end = mb->mmap_addr + mb->mmap_length;

    // Do I like doing pointer-arith? No! Does this work? Yeah! So it stays
    for (; (uint32_t)mmap < mmap_end; mmap = (multiboot_mmap_entry_t*)((uint32_t)mmap + mmap->size + sizeof(mmap->size))) {
        if (mmap->type == 1) { // Available RAM
            uint64_t start = mmap->addr;
            uint64_t end   = mmap->addr + mmap->length;

            for (uint64_t addr = start; addr < end; addr += FRAME_SIZE) {
                if (addr / FRAME_SIZE < MAX_FRAMES)
                    pmm_mark_free(addr / FRAME_SIZE);
            }

            kprintf("[pmm] Free region: 0x%08x - 0x%08x (%u KiB)\n",
                (uint32_t)start, (uint32_t)end, (uint32_t)(mmap->length / 1024));
        } else {
            kprintf("[pmm] Reserved: 0x%08x - 0x%08x\n",
                (uint32_t)mmap->addr, (uint32_t)(mmap->addr + mmap->length));
        }
    }

    // Reserving the kernel memory space to avoid... y'know, overwriting it.

    uint32_t kern_start = (uint32_t)kernel_start;
    uint32_t kern_end   = (uint32_t)kernel_end;

    kern_start &= ~(FRAME_SIZE - 1);
    kern_end = (kern_end + FRAME_SIZE - 1) & ~(FRAME_SIZE - 1);

    kprintf("[pmm] Reserving kernel: 0x%08x - 0x%08x (%u KiB)\n",
            kern_start, kern_end, (kern_end - kern_start) / 1024);

    for (uint32_t addr = kern_start; addr < kern_end; addr += FRAME_SIZE) {
        pmm_mark_used(addr / FRAME_SIZE);
    }

    kprintf("[pmm] Initalization complete!\n");
    goto ok;

no_mem_hang:
    klogf("Failed to initalize PMM! System halted! (PHYSICAL MEM FAILURE)\n");
    while (1) { __asm__ volatile("hlt"); }
ok:
    return;
}