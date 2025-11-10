#ifndef VGA_PRINT_H
#define VGA_PRINT_H

// TODO: Repurpose and rework into a form of kprintf()
// DONE: Refer to 'src/libk/kprint.h'

#include <stdint.h>

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEM     ((volatile uint16_t*)0xB8000)
#define VGA_ATTR    0x07

static uint8_t cursor_row = 0;
static uint8_t cursor_col = 0;

// Scrolling
static void vga_scroll(void) {
    for (int r = 1; r < VGA_HEIGHT; r++) {
        for (int c = 0; c < VGA_WIDTH; c++) {
            VGA_MEM[(r - 1) * VGA_WIDTH + c] = VGA_MEM[r * VGA_WIDTH + c];
        }
    }

    // Clear the last row
    for (int c = 0; c < VGA_WIDTH; c++) {
        VGA_MEM[(VGA_HEIGHT - 1) * VGA_WIDTH + c] = (VGA_ATTR << 8) | ' ';
    }

    cursor_row = VGA_HEIGHT - 1;
    cursor_col = 0;
}

// Clear
static inline void vga_clear(void) {
    for (int r = 0; r < VGA_HEIGHT; r++) {
        for (int c = 0; c < VGA_WIDTH; c++) {
            VGA_MEM[r * VGA_WIDTH + c] = (VGA_ATTR << 8) | ' ';
        }
    }
    cursor_row = 0;
    cursor_col = 0;
}

static inline void vga_newline(void) {
    cursor_col = 0;
    cursor_row++;
    if (cursor_row >= VGA_HEIGHT) {
        vga_scroll();
    }
}

static inline void vga_putc(char ch) {
    if (ch == '\n') {
        vga_newline();
        return;
    }

    VGA_MEM[cursor_row * VGA_WIDTH + cursor_col] = (VGA_ATTR << 8) | (uint8_t)ch;

    cursor_col++;
    if (cursor_col >= VGA_WIDTH) {
        vga_newline();
    }
}

// Print a single digit number (for debugging only)
static inline void vga_put_dec(uint32_t num) {
    char buf[11]; // up to 4294967295\0
    int i = 0;

    if (num == 0) {
        vga_putc('0');
        return;
    }

    while (num > 0 && i < 10) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    while (i > 0) {
        vga_putc(buf[--i]);
    }
}

static inline void vga_puts(const char *s) {
    while (*s) {
        vga_putc(*s++);
    }
}

static inline void vga_put_hex(uint32_t val) {
    const char *hex = "0123456789ABCDEF";
    vga_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        vga_putc(hex[(val >> i) & 0xF]);
    }
}

#endif // VGA_PRINT_H
