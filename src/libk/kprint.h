#ifndef KPRINT_H
#define KPRINT_H

#include <stdarg.h>
#include <stdint.h>

typedef void (*kputc_fn_t)(char);

void kset_sink(void (*sink)(char));
void kprint_set_backend(kputc_fn_t fn);
void kprintf(const char *fmt, ...);
void kvprintf(const char *fmt, va_list args);

#endif // KPRINT_H
