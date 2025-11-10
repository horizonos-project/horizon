#include <stdint.h>
#include "utilities/assert.h"
#include "utilities/vga_print.h"
#include "../drivers/vfs/vfs.h"

void kmain(void) {
    vga_clear();
    vga_puts("Horizon x86 - ");
    vga_puts(HORIZON_VERSION);
    vga_puts(" : Built on (");
    vga_puts(HORIZON_BUILD_DATE);
    vga_puts(")\n");

    // This should only be jumped to after the kernel has finished everything
    // it needs to during its lifecycle
    goto hang_ok;

hang_ok:
    vga_puts("System has finished executing and the kernel has halted.\n");
    vga_puts("You can now power down the PC.\n");
    while(1) { asm volatile("hlt"); }

test_fail:
    vga_puts("\nA test has failed and the system was halted!\n");
    while(1) { asm volatile("hlt"); }
}
