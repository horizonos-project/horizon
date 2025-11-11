#include <stdint.h>
#include "multiboot.h"
#include "libk/kprint.h"
#include "libk/time.h"
#include "drivers/video/vga.h"
#include "drivers/fs/ext2.h"
#include "log.h"
#include "drivers/serial/serial.h"
#include "mm/mm.h"
#include "panic.h"

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
    sleep(1500);

    pmm_init(mb);
    pmm_dump_stats();
    kheap_init();
    sleep(1000);

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
    while(1) { __asm__ volatile("hlt"); }
}
