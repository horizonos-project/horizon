#include <stdint.h>
#include <stdbool.h>

// Kernel headers & libk
#include "kernel/io.h"
#include "kernel/isr.h"
#include "kernel/pic.h"
#include "kernel/syscall/syscall.h"
#include "kernel/usermode.h"
#include "libk/kprint.h"

// Misc subsystems
#include "log.h"
#include "mm/mm.h"
#include "idt.h"
#include "gdt.h"
#include "tss.h"
#include "multiboot.h"

// Kernel drivers
#include "drivers/block/blkdev.h"
#include "drivers/serial/serial.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/video/vga.h"
#include "drivers/fs/ext2.h"
#include "drivers/ata/ata.h"
#include "drivers/fs/initramfs.h"
#include "drivers/vfs/vfs.h"

// External subsystems
extern int vfs_init(void);
extern int dummy_fs_init(void);
extern void serial_init(void);
extern void serial_puts(const char *s);

// Kernel heap memory management
extern void kheap_init(void);
extern void *kalloc(uint32_t size);
extern void kfree(void *ptr);

// Userland flag
bool userland = false;

// This is potentially no longer *needed* but keep it around just in case.
void dump_eflags(const char *msg) {
    uint32_t eflags;
    __asm__ volatile("pushf; pop %0" : "=r"(eflags));
    klogf("%s: EFLAGS=0x%08x IF=%u\n",
            msg, eflags, (eflags >> 9) & 1);
}

// Display Multiboot info during boot
void display_mb_info(multiboot_info_t *mb) {
    kprintf_both("[mb] - Multiboot Information\n");

    if (mb->flags & MB_INFO_MEM) {
        kprintf_both("[mb] Lower memory: %u KiB\n", mb->mem_lower);
        kprintf_both("[mb] Upper memory: %u KiB\n", mb->mem_upper);
    } else {
        kprintf_both("[mb] No memory info provided.\n");
        goto no_mb_mem_info;
    }

    if (mb->flags & MB_INFO_BOOT_DEVICE) {
        kprintf_both("[mb] Boot device: 0x%08x\n", mb->boot_device);
    }

    if (mb->flags & MB_INFO_CMDLINE) {
        const char *cmd = (const char *)(uintptr_t)mb->cmdline;
        kprintf_both("[mb] Cmdline: %s\n", cmd ? cmd : "(none)");
    }

    if (mb->flags & MB_INFO_MODS) {
        kprintf_both("[mb] Modules count: %u\n", mb->mods_count);
        kprintf_both("[mb] Modules addr:  0x%08x\n", mb->mods_addr);
    }

    if (mb->flags & MB_INFO_MMAP) {
        kprintf_both("[mb] Memory map: length=%u, addr=0x%08x\n", 
                mb->mmap_length, mb->mmap_addr);
    }

    kprintf_both("[mb] - End Multiboot Information\n");
    return;

no_mb_mem_info:
    klogf("System is in a halting state! (NO MEM INFO)\n");
    kprintf("No memory information has been provided and the system cannot continue.\n");
    kprintf("Power off the machine and check RAM slots or memory chips.\n");
    kprintf("The system has been halted to prevent undefined behavior.\n");
    while (1) { __asm__ volatile("hlt"); }
}

void test_heap(void) {
    klogf("\n[test] ===== Testing Heap =====\n");
    
    // Test 1: Small allocation
    void *ptr1 = kalloc(64);
    klogf("[test] kalloc(64) = 0x%08x\n", (uint32_t)ptr1);
    
    if (ptr1) {
        *(uint32_t*)ptr1 = 0xCAFEBABE;
        klogf("[test] Wrote 0xCAFEBABE, read: 0x%08x\n", *(uint32_t*)ptr1);
    }
    
    // Test 2: Medium allocation
    void *ptr2 = kalloc(1024);
    klogf("[test] kalloc(1024) = 0x%08x\n", (uint32_t)ptr2);
    
    if (ptr2) {
        *(uint32_t*)ptr2 = 0xDEADBEEF;
        klogf("[test] Wrote 0xDEADBEEF, read: 0x%08x\n", *(uint32_t*)ptr2);
    }
    
    // Test 3: Large allocation (triggers multiple page allocations)
    void *ptr3 = kalloc(32 * 1024);  // 32 KB
    klogf("[test] kalloc(32768) = 0x%08x\n", (uint32_t)ptr3);
    
    if (ptr3) {
        *(uint32_t*)ptr3 = 0xBADC0FFE;
        klogf("[test] Wrote 0xBADC0FFE, read: 0x%08x\n", *(uint32_t*)ptr3);
    }
    
    klogf("[test] Heap used: %u KB of %u KB\n", 
          kheap_get_used() / 1024, kheap_get_size() / 1024);
    
    pmm_dump_stats();
    
    klogf("[test] ===== Heap Test Complete =====\n\n");
}

void kmain(uint32_t magic, uint32_t mb_info_addr) {
    serial_init();
    kclear();

    if (magic != MULTIBOOT_MAGIC)
        goto not_multiboot;

    multiboot_info_t *mb = (multiboot_info_t *)(uintptr_t)mb_info_addr;

    // ========== Phase 1: Basic Hardware & Logging ==========
    
    log_init();
   
    klogf("[ok] Logging initialized.\n");
    klogf("[ok] VGA/Serial ready.\n");

    // ========== Phase 2: CPU & Interrupt Setup ==========
    
    idt_init();
    gdt_install();
    isr_install();
    irq_install();
    syscall_init();
    syscall_register_all();

    pit_init(100);
    pit_check();

    pic_clear_mask(0);
    pic_clear_mask(1);

    keyboard_init();

    uint8_t mask = inb(0x21);
    klogf("[pic] PIC1 mask is 0x%02x (bit 0 should be 0)\n", mask);

    klogf("[ok] IDT loaded and exceptions are online.\n");
    klogf("[ok] ISR and IRQ are also OK.\n");

    // ========== Phase 3: Memory Management ==========
    
    display_mb_info(mb);

    pmm_init(mb);
    pmm_dump_stats();
    klogf("[pmm] Physical Memory Management is OK.\n");

    vmm_init();
    klogf("[vmm] Virtual Memory Management is OK.\n");
    
    kheap_init();
    klogf("[heap] Kernel heap has been allocated.\n");
    test_heap();

    // ========== Phase 4: Block Devices & Filesystems ==========
    
    if (vfs_init() < 0) {
        klogf("[fail] VFS failure.\n");
        goto bad_vfs;
    }

    // Initialize block device layer
    blkdev_init();
    klogf("[ok] Block device layer initialized.\n");

    // Initialize ATA driver (if present)
    if (ata_init() < 0) {
        klogf("[warn] No ATA drives detected. Continuing without disk.\n");
    } else {
        klogf("[ok] ATA drive detected and registered.\n");
    }

    // Initialize initramfs (static data, no heap needed for init)
    // initramfs_init();

    // Register filesystem drivers
    if (ext2_register() < 0) {
        klogf("[fail] ext2 registration failed.\n");
        goto bad_ext2_fs;
    }
    klogf("[ok] ext2 registered successfully.\n");

    klogf("[ext2] Mounting root filesystem from /dev/hda...\n");

    if (vfs_mount("ext2", "hda", "/") < 0) {
        klogf("[panic] Failed to mount root filesystem (ext2 on hda)\n");
        goto bad_ext2_fs;
    }

    klogf("[ok] Root filesystem mounted at '/'\n");

    // ========== Phase 5: Ring 3 & Process Setup ==========

    uint32_t k_stack = (uint32_t)kalloc(4096);
    if (!k_stack) {
        kprintf_both("[kernel] FATAL: Failed to alloc kernel stack!\n");
        goto bad_kalloc;
    }

    k_stack += 4096;

    kprintf_both("[kernel] Allocated kernel stack at 0x%08x\n", k_stack);
    tss_install(k_stack);
    
    dump_eflags("[cpu] Before sti\n");
    __asm__ volatile("sti");

    // klogf("[kbd] press any key...\n");
    // while (keyboard_getchar() < 0) {
    //     __asm__ volatile("hlt");
    // }
    // klogf("[kbd] got key!\n");

    dump_eflags("[cpu] After sti\n");
    
    klogf("[cpu] Interrupts enabled via sti\n");
    
    // Check EFLAGS
    uint32_t eflags;
    __asm__ volatile("pushf; pop %0" : "=r"(eflags));
    klogf("[cpu] EFLAGS: 0x%08x\n", eflags);
    klogf("[cpu] IF bit (bit 9): %u\n", (eflags >> 9) & 1);
    
    if (!((eflags >> 9) & 1)) {
        klogf("[cpu] CRITICAL: Interrupts are NOT enabled!\n");
        klogf("[cpu] sti didn't work\n");
    }

    kprintf_both("[ring3] The kernel is now ready for ring3 operations.\n");

    // Allocate user stack
    void *user_stack_frame = pmm_alloc_frame();
    if (!user_stack_frame) {
        kprintf_both("[kernel] Failed to allocate user stack frame!\n");
        goto bad_kalloc;
    }

    uint32_t user_stack_phys = (uint32_t)user_stack_frame;
    uint32_t user_stack_virt = 0x00800000;

    klogf("[ring3] Allocating user stack:\n");
    klogf("[ring3]   Physical: 0x%08x\n", user_stack_phys);
    klogf("[ring3]   Virtual:  0x%08x\n", user_stack_virt);

    // Map it as user-accessible
    vmm_map_page(user_stack_virt, user_stack_phys, PAGE_PRESENT | PAGE_RW | PAGE_USER);

    // Stack grows down, so pass top of page
    uint32_t user_stack_top = user_stack_virt + 4096;

    klogf("[ring3]   Stack top: 0x%08x\n", user_stack_top);

    // ========== Phase 6: Launch Userspace ==========

    jump_to_elf("/sbin/init");

    // ========== System Halt (Should Never Reach Here) ==========

    goto hang_ok;

hang_ok:
    klogf("System is in a halting state! (EXEC OK)\n");
    kprintf("\nSystem has finished executing and the kernel is halted.\n");
    kprintf("You can now power down the PC.\n");
    goto hang;

test_fail:
    klogf("System is in a halting state! (TEST FAIL)\n");
    kprintf("\nA test has failed and the system was halted!\n");
    goto hang;

not_multiboot:
    klogf("System is in a halting state! (NOT MULTIBOOT)\n");
    kprintf("\nHorizon was not booted on a MULTIBOOT compliant system!\n");
    kprintf("Please reboot Horizon on a MULTIBOOT compliant loader.\n");
    goto hang;

bad_ext2_fs:
    klogf("System is in a halting state! (EXT2 BAD)\n");
    kprintf("\nSystem failed to register the ext2 filesystem.\n");
    kprintf("The system has been halted to prevent potential damages.\n");
    goto hang;

bad_vfs:
    klogf("System is in a halting state! (VFS BAD)\n");
    kprintf("\nSystem failed to initialize the VFS.\n");
    kprintf("The system has been halted to prevent damage to the machine.\n");
    goto hang;

bad_kalloc:
    kprintf_both("System is halting! (KALLOC FAILURE)\n");
    kprintf_both("System halted to prevent damages.\n");
    goto hang;

hang:
    while(1) { 
        __asm__ volatile("hlt"); 
    }
}
