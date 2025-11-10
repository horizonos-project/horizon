#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdbool.h>

// function pointers for backends
typedef void (*log_write_t)(const char *s);

void log_init(void);
void log_register_sink(log_write_t sink);
void log_puts(const char *s);
void klogf(const char *fmt, ...);
void kprintf_both(const char *fmt, ...);

#endif
