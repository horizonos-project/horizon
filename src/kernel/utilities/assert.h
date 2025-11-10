// nostdlib halt macro for Horizon
// (c) 2025- HorizonOS Project
//

#include "vga_print.h"

#define K_ASSERT(_e, _msg) do \
    { if (!_e) { vga_clear(); vga_puts("Exception raised!"); \
        __asm__ volatile("cli"); \
        __asm__ volatile("hlt"); }} while (0);
