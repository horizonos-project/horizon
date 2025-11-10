#include <stdint.h>
#include "multiboot.h"
#include "utilities/assert.h"
#include "../libk/kprint.h"
#include "../drivers/fs/ext2.h"

// This begins the giant block of extern functions
extern void vfs_init(void);
extern int dummy_fs_init(void);
extern void serial_init(void);
extern void serial_puts(const char *s);

void kmain(uint32_t magic, uint32_t mb_info_addr) {
    serial_init();
    kclear();
    kprintf("Horizon x86 - Ver: %s | Build Date: %s\n\n", HORIZON_VERSION, HORIZON_BUILD_DATE);

    if (magic != MULTIBOOT_MAGIC)
        goto not_multiboot;

    multiboot_info_t *mb = (multiboot_info_t *)(uintptr_t)mb_info_addr;

    vfs_init();
    
    if (dummy_fs_init() < 0) {
        goto bad_vfs;
    } else {
        kprintf("\nVFS appears to be OK.\n");
    }

    if (ext2_register() < 0) {
        goto bad_ext2_fs;
    } else {
        kprintf("Filesystem (ext2) appears to be OK.\n");
    }

    display_mb_info(mb);

    // This should only be jumped to after the kernel has finished everything
    // it needs to during its lifecycle
    goto hang_ok;

hang_ok:
    kprintf("\n\nSystem has finished executing and the kernel is halted.\n");
    kprintf("You can now power down the PC.\n");
    goto hang;

// Not unused, this will come back when the OS runs post POST tests and checks on itself
test_fail:
    kprintf("\n\nA test has failed and the system was halted!\n");
    goto hang;

not_multiboot:
    kprintf("\n\nHorizon was not booted on a MULTIBOOT compliant system!\n");
    kprintf("Please reboot Horizon on MULTIBOOT compliant loader.\n");
    goto hang;

bad_ext2_fs:
    kprintf("\n\nSystem failed to register the ext2 filesystem.\n");
    kprintf("The system has been halted to prevent potential damages.\n");
    goto hang;

bad_vfs:
    kprintf("\n\nSystem failed to initalize the VFS.\n");
    kprintf("The system has been halted to prevent damage to the machine.\n");
    goto hang;

hang:
    while(1) { __asm__ volatile("hlt"); }
}
