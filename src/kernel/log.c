#include "log.h"
#include "../libk/kprint.h"
#include "../drivers/serial/serial.h"
#include "../drivers/video/vga.h"

static void log_vga_putc(char c)  { vga_putc(c); }
static void log_serial_putc(char c) { serial_putc(c); }

void log_init(void) {
    kset_sink(log_serial_putc);
    kprintf("[log] Serial sink ready\n");

    // kset_sink(log_vga_putc);
    // kprintf("[ok] Logging initialized.\n");
    // kprintf("[ok] VGA/Serial ready.\n");
}

// Print to both VGA + serial safely
void kprintf_both(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);

    // VGA
    kset_sink(log_vga_putc);
    kvprintf(fmt, args);

    // Restore Serial
    kset_sink(log_serial_putc);
    kvprintf(fmt, args_copy);

    va_end(args_copy);
    va_end(args);
}

// Log to serial only
void klogf(const char *fmt, ...) {
    va_list args1, args2;

    va_start(args1, fmt);
    va_copy(args2, args1);

    kvprintf(fmt, args2);

    va_end(args1);
    va_end(args2);
}
