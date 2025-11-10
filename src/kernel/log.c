#include "log.h"
#include "../libk/kprint.h"
#include "../drivers/serial/serial.h"
#include "../drivers/video/vga.h"

static void log_vga_putc(char c)  { vga_putc(c); }
static void log_serial_putc(char c) { serial_putc(c); }

void log_init(void) {
    kset_sink(log_serial_putc);
    kprintf("[log] Serial sink ready\n");

    kset_sink(log_vga_putc);
    kprintf("[ok] Logging initialized.\n");
    kprintf("[ok] VGA/Serial ready.\n");
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

    // Serial
    kset_sink(log_serial_putc);
    kvprintf(fmt, args_copy);

    // Restore default (VGA)
    kset_sink(log_vga_putc);

    va_end(args_copy);
    va_end(args);
}

// Write to both sinks
void klogf(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    // VGA
    kset_sink(log_vga_putc);
    kprintf(fmt, args);

    // Serial
    kset_sink(log_serial_putc);
    kprintf(fmt, args);

    // Restore VGA
    kset_sink(log_vga_putc);
    va_end(args);
}
