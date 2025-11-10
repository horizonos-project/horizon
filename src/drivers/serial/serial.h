#pragma once

#define COM1_PORT 0x3F8

void serial_init(void);
void serial_putc(char ch);
void serial_puts(const char *s);