#include <stdint.h>
#include "multiboot.h"
#include "utilities/assert.h"
#include "../libk/kprint.h"
#include "../drivers/fs/ext2.h"

extern void vfs_init(void);

void kmain(uint32_t magic, uint32_t mb_info_addr) {
    kclear();
    kprintf("Horizon x86 - V%s - B%s\n\n", HORIZON_VERSION, HORIZON_BUILD_DATE);

    vfs_init();
    if (ext2_register() < 0) {
        goto bad_ext2_fs;
    } else {
        kprintf("\nFilesystem (ext2) appears to be OK.\n");
    }

    // This should only be jumped to after the kernel has finished everything
    // it needs to during its lifecycle
    goto hang_ok;

hang_ok:
    kprintf("\n\nSystem has finished executing and the kernel is halted.\n");
    kprintf("You can now power down the PC.\n");
    goto hang;

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

hang:
    while(1) { asm volatile("hlt"); }
}
