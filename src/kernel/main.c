#include "utilities/assert.h"
#include "utilities/vga_print.h"
#include "idt.h"

#include <stdint.h>

#define OK 0

void kmain(void)
{
    vga_clear();
    vga_printf("Entering Protected Mode...\n");

    if(idt_init() == OK)
        vga_printf("Protected mode entry OK.\n");
    else
        vga_printf("Protected mode entry fail!\n");

    for (;;) __asm__ volatile("cli; hlt");
}
