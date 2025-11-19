/**
 * @file serial.h
 * @brief Serial Port (UART) Driver
 * 
 * Provides output to the COM1 serial port, which is incredibly useful for:
 * - Debugging without cluttering the VGA screen
 * - Logging kernel messages to a file (via serial redirection)
 * - Remote debugging when VGA isn't available
 * - Triple fault debugging (serial often survives when VGA doesn't!)
 * 
 * Serial output can be captured in QEMU with:
 * ```
 * qemu-system-i386 -serial file:serial.log
 * qemu-system-i386 -serial stdio
 * ```
 * 
 * @note Serial output works even when everything else is on fire!
 */

#pragma once

#define COM1_PORT 0x3F8

/**
 * @brief COM1 serial port I/O base address
 * 
 * Standard IBM PC COM1 port location. Other COM ports:
 * - COM2: 0x2F8
 * - COM3: 0x3E8
 * - COM4: 0x2E8
 * 
 * @note This is pretty important if you want to debug this mess.
 */
#define COM1_PORT 0x3F8

/**
 * @brief Initialize the serial port
 * 
 * Configures COM1 for basic 8-N-1 operation:
 * - Baud rate: Usually 38400 or 115200
 * - Data bits: 8
 * - Parity: None
 * - Stop bits: 1
 * 
 * After initialization, serial_putc() and serial_puts() can be used.
 * 
 * @note Should be called very early in boot - even before VGA if possible!
 */
void serial_init(void);

/**
 * @brief Write a character to the serial port
 * 
 * Sends a single character to COM1. Blocks until the transmit buffer
 * is ready (busy-waits on the UART status register).
 * 
 * @param ch Character to transmit
 * 
 * Example:
 * @code
 * serial_putc('[');
 * serial_putc('O');
 * serial_putc('K');
 * serial_putc(']');
 * serial_putc('\n');
 * @endcode
 */
void serial_putc(char ch);

/**
 * @brief Write a string to the serial port
 * 
 * Convenience wrapper that sends a null-terminated string by
 * repeatedly calling serial_putc().
 * 
 * @param s Null-terminated string to transmit
 * 
 * Example:
 * @code
 * serial_puts("[boot] Kernel initialized\n");
 * serial_puts("[mm] PMM ready\n");
 * @endcode
 */
void serial_puts(const char *s);