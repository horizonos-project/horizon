#include "kprint.h"

static void (*kputc_sink)(char) = 0;

void kset_sink(void (*sink)(char)) {
    kputc_sink = sink;
}

void kputc(char c) {
    if (kputc_sink)
        kputc_sink(c);
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

static void tf_put_hex(uint32_t val, int width) {
    const char *hex = "0123456789ABCDEF";
    if (width <= 0) width = 8;
    
    for (int i = (width - 1) * 4; i >= 0; i -= 4)
        kputc(hex[(val >> i) & 0xF]);
}

void kvprintf(const char *fmt, va_list args) {
    for (const char *p = fmt; *p; p++) {
        if (*p != '%') {
            kputc(*p);
            continue;
        }

        p++; // skip '%'

        int width = 0;
        int zero_pad = 0;
        int long_flag = 0;

        if (*p == '0') { zero_pad = 1; p++; }
        while (*p >= '0' && *p <= '9') {
            width = width * 10 + (*p - '0');
            p++;
        }
        if (*p == 'l') { long_flag = 1; p++; }

        switch (*p) {
        case 's': {
            const char *s = va_arg(args, const char*);
            if (!s) s = "(null)";
            while (*s) kputc(*s++);
            break;
        }
        case 'c': {
            char c = (char)va_arg(args, int);
            kputc(c);
            break;
        }
        case 'd':
        case 'i': {
            int32_t val = long_flag
                ? va_arg(args, int32_t)
                : va_arg(args, int);
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
            if (!width) width = 8;
            if (zero_pad) {
                // zero pad manually
                const char *hex = (*p == 'X')
                    ? "0123456789ABCDEF"
                    : "0123456789abcdef";
                for (int i = (width - 1) * 4; i >= 0; i -= 4)
                    kputc(hex[(val >> i) & 0xF]);
            } else {
                tf_put_hex(val, width);
            }
            break;
        }
        case 'p': {
            uint32_t ptr = va_arg(args, uint32_t);
            kputc('0'); kputc('x');
            tf_put_hex(ptr, 8);
            break;
        }
        case '%':
            kputc('%');
            break;
        default:
            // unknown sequence, print literally
            kputc('%');
            kputc(*p);
            break;
        }
    }

    va_end(args);
}

/* public varargs wrapper */
void kprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    kvprintf(fmt, args);
    va_end(args);
}