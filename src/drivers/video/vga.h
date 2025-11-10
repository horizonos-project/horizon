#ifndef VGA_H
#define VGA_H

#include <stdint.h>

void vga_clear(void);
void vga_putc(char ch);
void vga_puts(const char *s);
void kclear(void);

#endif // VGA_H
