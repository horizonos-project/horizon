#include "vga.h"
#include <stddef.h>
#include <stdint.h>

static volatile uint16_t *const VRAM = (uint16_t*)0xB8000;
static uint8_t s_fg = 0x07, s_bg = 0x00; // light gray on black
static size_t  s_row = 0, s_col = 0;

static inline uint16_t vga_entry(char c, uint8_t fg, uint8_t bg)
{
    return (uint16_t)c | ((uint16_t)((bg<<4)|fg) << 8);
}

static void hw_set_cursor(size_t row, size_t col)
{
    uint16_t pos = (uint16_t)(row * VGA_WIDTH + col);
    outb(0x3D4, 0x0F); 
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    outb(0x3D4, 0x0E); 
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void scroll_if_needed(void)
{
    if (s_row < VGA_HEIGHT) return;
    // move rows up
    for (size_t r=1; r<VGA_HEIGHT; ++r)
        for (size_t c=0; c<VGA_WIDTH; ++c)
            VRAM[(r-1)*VGA_WIDTH + c] = VRAM[r*VGA_WIDTH + c];
    // clear last row
    for (size_t c=0; c<VGA_WIDTH; ++c)
        VRAM[(VGA_HEIGHT-1)*VGA_WIDTH + c] = vga_entry(' ', s_fg, s_bg);
    s_row = VGA_HEIGHT-1;
}

void vga_setcolor(uint8_t fg, uint8_t bg)
{
    s_fg=fg;
    s_bg=bg;
}

void vga_clear(void)
{
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i)
        VRAM[i] = vga_entry(' ', s_fg, s_bg);
    s_row = s_col = 0;
    hw_set_cursor(s_row, s_col);
}

void vga_init(void)
{
    vga_clear(); 
}

void vga_putc(char c)
{
    if (c=='\n')
    {
        s_col=0; 
        s_row++; 
        scroll_if_needed();
        hw_set_cursor(s_row,s_col);
        return; 
    }
    if (c=='\r')
    {
        s_col=0; 
        hw_set_cursor(s_row,s_col);
        return; 
    }
    if (c=='\b')
    {
        if (s_col)
        {
            --s_col; VRAM[s_row * VGA_WIDTH + s_col]=vga_entry(' ',s_fg,s_bg);
        } 
        hw_set_cursor(s_row,s_col); 
        return; 
    }
    VRAM[s_row * VGA_WIDTH + s_col] = vga_entry(c, s_fg, s_bg);
    if (++s_col>=VGA_WIDTH)
    {
        s_col=0; s_row++; 
        scroll_if_needed();
    }
    hw_set_cursor(s_row, s_col);
}

void vga_write(const char *s)
{ 
    while(*s) 
        vga_putc(*s++); 
}