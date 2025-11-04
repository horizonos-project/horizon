#pragma once

#include <stdint.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

//
void vga_init(void);

//
void vga_putc(char c);

//
void vga_write(const char *s);

//
void vga_setcolor(uint8_t fg, uint8_t bg);

//
void vga_clear(void);
