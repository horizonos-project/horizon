#ifndef VGA_PRINTF_H
#define VGA_PRINTF_H

// Even though keyboard drivers are going to be impl'd soon (November 3, 2025)
// These will still exist, possibly repurposed into kprintf() to peek into the kernel

#include <stdint.h>

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEM     ((volatile uint16_t*)0xB8000)
#define VGA_ATTR    0x07

static uint8_t cursor_row = 0;
static uint8_t cursor_col = 0;

// Scrolling
static void vga_scroll(void)
{
    for (int r = 1; r < VGA_HEIGHT; r++)
    {
        for (int c = 0; c < VGA_WIDTH; c++)
        {
            VGA_MEM[(r - 1) * VGA_WIDTH + c] = VGA_MEM[r * VGA_WIDTH + c];
        }
    }

    // Clear the last row
    for (int c = 0; c < VGA_WIDTH; c++)
    {
        VGA_MEM[(VGA_HEIGHT - 1) * VGA_WIDTH + c] = (VGA_ATTR << 8) | ' ';
    }
    cursor_row = VGA_HEIGHT - 1;
    cursor_col = 0;
}

// Clear
inline void vga_clear(void) {
    for (int r = 0; r < VGA_HEIGHT; r++)
    {
        for (int c = 0; c < VGA_WIDTH; c++)
        {
            VGA_MEM[r * VGA_WIDTH + c] = (VGA_ATTR << 8) | ' ';
        }
    }
    cursor_row = 0;
    cursor_col = 0;
}

static void vga_newline(void) {
    cursor_col = 0;
    cursor_row++;
    if (cursor_row >= VGA_HEIGHT) {
        vga_scroll();
    }
}

inline void vga_putc(char ch) {
    if (ch == '\n') {
        vga_newline();
        return;
    }

    VGA_MEM[cursor_row * VGA_WIDTH + cursor_col] =
        (VGA_ATTR << 8) | (uint8_t)ch;

    cursor_col++;
    if (cursor_col >= VGA_WIDTH) {
        vga_newline();
    }
}

inline void vga_printf(const char *s)
{
    while (*s) {
        vga_putc(*s++);
    }
}

#endif // VGA_PRINTF_H
