#include "utilities/assert.h"
#include "utilities/vga_print.h"
#include "idt.h"

#include <stdint.h>

void kmain(void)
{
    vga_clear();
    vga_printf("Entering Protected Mode...\n");
    idt_init();
    vga_printf("IDT Loaded, PIC remapped, interrupts enabled!\n");
    vga_printf("\nWelcome to HorizonOS (x86)!\n");

    for (;;) __asm__ volatile("cli; hlt");
}
