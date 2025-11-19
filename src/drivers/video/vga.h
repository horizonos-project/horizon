/**
 * @file vga.h
 * @brief VGA Text Mode Driver
 * 
 * Provides simple 80x25 text mode output using the legacy VGA text buffer
 * at 0xB8000. This is the simplest way to get visual output on x86 and
 * works on virtually all hardware (even ancient stuff!).
 * 
 * The VGA text buffer is a memory-mapped region where each character cell
 * consists of two bytes:
 * - Byte 0: ASCII character code
 * - Byte 1: Attribute byte (foreground/background color)
 * 
 * Features:
 * - 80 columns x 25 rows
 * - 16 foreground colors, 8 background colors
 * - Hardware cursor (blinking underline)
 * - Automatic scrolling when screen fills
 * 
 * Limitations:
 * - Text mode only (no graphics)
 * - Fixed resolution (80x25)
 * - No unicode (ASCII only)
 * - Will be replaced by framebuffer eventually...
 */

#ifndef VGA_H
#define VGA_H

#include <stdint.h>

/**
 * @brief Clear the VGA screen
 * 
 * Fills the entire 80x25 text buffer with spaces and the default
 * color attribute. Resets the cursor to the top-left corner (0,0).
 * 
 * Example:
 * @code
 * vga_clear();  // Fresh slate!
 * vga_puts("HorizonOS booting...\n");
 * @endcode
 */
void vga_clear(void);

/**
 * @brief Write a single character to VGA
 * 
 * Outputs a character at the current cursor position and advances
 * the cursor. Does not yet special characters such as:
 * - '\\r': Return to start of current line
 * - '\\t': Tab (usually 4-8 spaces)
 * - '\\b': Backspace (move cursor back one position)
 * 
 * Automatically scrolls the screen up when the cursor reaches the
 * bottom row.
 * 
 * @param ch ASCII character to display
 * 
 * Example:
 * @code
 * vga_putc('H');
 * vga_putc('i');
 * vga_putc('!');
 * vga_putc('\n');
 * @endcode
 */
void vga_putc(char ch);

/**
 * @brief Write a string to VGA
 * 
 * Outputs a null-terminated string by repeatedly calling vga_putc().
 * Convenience wrapper for printing text.
 * 
 * @param s Null-terminated string to display
 * 
 * Example:
 * @code
 * vga_puts("Kernel initialized successfully!\n");
 * @endcode
 */
void vga_puts(const char *s);

/**
 * @brief Clear screen (alias for vga_clear)
 * 
 * Kernel-specific alias for clearing the display. Useful when you
 * want to emphasize this is a kernel operation vs raw VGA access.
 * 
 * @note Literally just calls vga_clear() - use whichever name feels right!
 */
void kclear(void);

#endif // VGA_H
