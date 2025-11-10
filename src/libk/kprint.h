#ifndef KPRINT_H
#define KPRINT_H

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "../kernel/utilities/vga_print.h"

extern void serial_putc(char ch);

// Basic utilities ------------------------------------------------------------

static inline void kclear(void) { vga_clear(); }

static inline void kputc(char ch) {
    vga_putc(ch);
    serial_putc(ch);
}

static void tf_put_dec(uint32_t val) {
    char buf[10];
    int i = 0;
    if (val == 0) { kputc('0'); return; }
    while (val > 0 && i < 10) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    while (i--) kputc(buf[i]);
}

static void tf_put_hex(uint32_t val, int width, bool upper) {
    const char *hex = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    for (int i = (width - 1) * 4; i >= 0; i -= 4)
        kputc(hex[(val >> i) & 0xF]);
}

// Core printf ------------------------------------------------------------

static inline void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char *p = fmt; *p; p++) {
        if (*p != '%') {
            kputc(*p);
            continue;
        }

        p++; // skip '%'
        int width = 0;
        bool zero_pad = false;
        bool long_flag = false;

        // Parse flags like '0' or width numbers
        if (*p == '0') {
            zero_pad = true;
            p++;
        }
        while (*p >= '0' && *p <= '9') {
            width = width * 10 + (*p - '0');
            p++;
        }
        if (*p == 'l') { // support %lu, %lx, etc
            long_flag = true;
            p++;
        }

        switch (*p) {
            case 's': {
                const char *s = va_arg(args, const char*);
                if (!s) s = "(null)";
                vga_puts(s);
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                kputc(c);
                break;
            }
            case 'd':
            case 'i': {
                int32_t val = long_flag ? va_arg(args, int32_t) : va_arg(args, int);
                if (val < 0) {
                    kputc('-');
                    val = -val;
                }
                tf_put_dec((uint32_t)val);
                break;
            }
            case 'u': {
                uint32_t val = va_arg(args, uint32_t);
                tf_put_dec(val);
                break;
            }
            case 'x':
            case 'X': {
                uint32_t val = va_arg(args, uint32_t);
                if (!width) width = 8; // default width
                if (zero_pad) {
                    const char *hex = (*p == 'X') ? "0123456789ABCDEF" : "0123456789abcdef";
                    for (int i = (width - 1) * 4; i >= 0; i -= 4)
                        kputc(hex[(val >> i) & 0xF]);
                } else {
                    tf_put_hex(val, width, (*p == 'X'));
                }
                break;
            }
            case 'p': {
                uint32_t ptr = va_arg(args, uint32_t);
                vga_puts("0x");
                tf_put_hex(ptr, 8, false);
                break;
            }
            case '%': {
                kputc('%');
                break;
            }
            default: {
                // Unrecognized format — print literally
                kputc('%');
                kputc(*p);
                break;
            }
        }
    }

    va_end(args);
}

#endif // KPRINT_H
