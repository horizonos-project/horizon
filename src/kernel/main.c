#include <stdint.h>
#include "kernel/io.h"
#include "kernel/isr.h"
#include "kernel/pic.h"
#include "kernel/syscall/syscall.h"
#include "multiboot.h"
#include "libk/kprint.h"
#include "drivers/video/vga.h"
#include "drivers/fs/ext2.h"
#include "log.h"
#include "drivers/serial/serial.h"
#include "mm/mm.h"
#include "idt.h"

// External subsystems
extern int vfs_init(void);
extern int dummy_fs_init(void);
extern void serial_init(void);
extern void serial_puts(const char *s);

// Kernel heap memory management
extern void kheap_init(void);
extern void *kalloc(uint32_t size);
extern void kfree(void *ptr);

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

void kmain(uint32_t magic, uint32_t mb_info_addr) {
    serial_init();
    kclear();

    if (magic != MULTIBOOT_MAGIC)
        goto not_multiboot;

    multiboot_info_t *mb = (multiboot_info_t *)(uintptr_t)mb_info_addr;

    // Initalizing the subsystems like log and fs
    
    log_init();
   
    klogf("[ok] Logging initalized.\n");
    klogf("[ok] VGA/Serial ready.\n");

    idt_init();
    isr_install();
    irq_install();
    syscall_init();
    syscall_register_all();

    pit_init(100);
    pit_check();

    pic_clear_mask(0);

    uint8_t mask = inb(0x21);
    klogf("[pic] PIC1 mask is 0x%02x (bit 0 should be 0)\n", mask);

    klogf("[ok] IDT loaded and exceptions are online.\n");
    klogf("[ok] ISR and IRQ are also OK.\n");

    if (vfs_init() < 0) {
        klogf("[fail] VFS failure.\n");
        goto bad_vfs;
    }

    if (dummy_fs_init() < 0) {
        klogf("[fail] DummyFS init fail.\n");
        goto test_fail;
    }

    klogf("[ok] DummyFS initalized.\n");

    if (ext2_register() < 0) {
        klogf("[fail] ext2 registration failed.\n");
        goto bad_ext2_fs;
    }

    klogf("[ok] ext2 registered successfully.\n");

    // multiboot info
    display_mb_info(mb);

    pmm_init(mb);
    pmm_dump_stats();
    klogf("[pmm] Physical Memory Management is OK.\n");

    vmm_init();
    klogf("[vmm] Virtual Memory Management is OK.\n");

    kheap_init();
    klogf("[heap] Kernel heap as been allocated.\n");

    __asm__ volatile("sti");

    klogf("[cpu] Interrupts enabled via sti\n");

    // Check EFLAGS
    uint32_t eflags;
    __asm__ volatile("pushf; pop %0" : "=r"(eflags));
    klogf("[cpu] EFLAGS: 0x%08x\n", eflags);
    klogf("[cpu] IF bit (bit 9): %u\n", (eflags >> 9) & 1);

    if (!((eflags >> 9) & 1)) {
        klogf("[cpu] CRITICAL: Interrupts are NOT enabled!\n");
        klogf("sti didn't work");
    }

    // This should only be jumped to after the kernel has finished everything
    // it needs to during its lifecycle
    goto hang_ok;

hang_ok:
    klogf("System is in a halting state! (EXEC OK)\n");
    kprintf("\nSystem has finished executing and the kernel is halted.\n");
    kprintf("You can now power down the PC.\n");
    goto hang;

// Not unused, this will come back when the OS runs post POST tests and checks on itself
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
    kprintf("\nSystem failed to initalize the VFS.\n");
    kprintf("The system has been halted to prevent damage to the machine.\n");
    goto hang;

hang:
    while(1) { 
        __asm__ volatile("hlt"); 
    }
}
