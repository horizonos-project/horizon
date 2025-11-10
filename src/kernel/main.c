#include <stdint.h>
#include "multiboot.h"
#include "utilities/assert.h"
#include "../libk/kprint.h"
#include "../drivers/vfs/vfs.h"

void kmain(uint32_t magic, uint32_t mb_info_addr) {
    kclear();
    kprintf("Horizon x86 - V%s - B%s\n\n", HORIZON_VERSION, HORIZON_BUILD_DATE);

    // This should only be jumped to after the kernel has finished everything
    // it needs to during its lifecycle
    goto hang_ok;

hang_ok:
    kprintf("System has finished executing and the kernel is halted.\n");
    kprintf("You can now power down the PC.\n");
    goto hang;

test_fail:
    kprintf("\nA test has failed and the system was halted!\n");
    goto hang;

not_multiboot:
    kprintf("Horizon was not booted on a MULTIBOOT compliant system!\n");
    kprintf("Please reboot Horizon on MULTIBOOT compliant loader.\n");
    goto hang;

hang:
    while(1) { asm volatile("hlt"); }
}
