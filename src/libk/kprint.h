#ifndef KPRINT_H
#define KPRINT_H

#include <stdarg.h>
#include <stdint.h>
#include "../kernel/utilities/vga_print.h"

static void kclear() {
    vga_clear();
}

static void tf_put_dec(uint32_t val) {
    char buf[10];
    int i = 0;

    if (val == 0) {
        vga_putc('0');
        return;
    }

    while (val > 0 && i < 10) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    while (i--) vga_putc(buf[i]);
}

static void tf_put_hex(uint32_t val) {
    const char *hex = "0123456789ABCDEF";
    vga_puts("0x");
    for (int i = 28; i >= 0; i -= 4)
        vga_putc(hex[(val >> i) & 0xF]);
}

static inline void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char *p = fmt; *p; p++) {
        if (*p != '%') {
            vga_putc(*p);
            continue;
        }

        p++; // skip '%'
        switch (*p) {
            case 's': {
                const char *s = va_arg(args, const char*);
                if (!s) s = "(null)";
                vga_puts(s);
                break;
            }
            case 'd': {
                int val = va_arg(args, int);
                if (val < 0) {
                    vga_putc('-');
                    val = -val;
                }
                tf_put_dec((uint32_t)val);
                break;
            }
            case 'x': {
                uint32_t val = va_arg(args, uint32_t);
                tf_put_hex(val);
                break;
            }
            case 'p': {
                uint32_t ptr = va_arg(args, uint32_t);
                tf_put_hex(ptr);
                break;
            }
            case '%':
                vga_putc('%');
                break;
            default:
                vga_putc('%');
                vga_putc(*p);
                break;
        }
    }

    va_end(args);
}

#endif // TINYPRINTF_H
