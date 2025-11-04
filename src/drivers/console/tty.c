#include "tty.h"
#include "vga.h"

void tty_init(void)
{
    vga_init();
}

void tty_write(const char *s)
{
    vga_write(s);
}

void tty_putc(char c)
{
    vga_putc(c);
}