#include <stdint.h>
#include "libk/kprint.h"
#include "kernel/io.h"

#define COM1_PORT 0x3F8

void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);    // disable interrupts
    outb(COM1_PORT + 3, 0x80);    // enable DLAB
    outb(COM1_PORT + 0, 0x03);    // baud divisor low (38400 baud)
    outb(COM1_PORT + 1, 0x00);    // high byte
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop
    outb(COM1_PORT + 2, 0xC7);    // enable FIFO, clear them
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static inline int serial_ready(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_putc(char ch) {
    while (!serial_ready());
    outb(COM1_PORT, ch);
}

void serial_puts(const char *s) {
    while (*s)
        serial_putc(*s++);
}