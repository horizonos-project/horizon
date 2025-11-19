/**
 * @file io.h
 * @brief x86 Port I/O Operations
 * 
 * Provides inline assembly wrappers for x86 IN and OUT instructions,
 * which are used to communicate with hardware devices via I/O ports.
 * 
 * Common port ranges:
 * - 0x20-0x21: PIC (Programmable Interrupt Controller)
 * - 0x3D4-0x3D5: VGA CRTC (CRT Controller)
 * - 0x3F8-0x3FF: COM1 serial port
 * - 0x40-0x43: PIT (Programmable Interval Timer)
 * - 0x60, 0x64: PS/2 keyboard controller
 * - 0x1F0-0x1F7: Primary ATA/IDE hard disk
 * 
 * @note These are privileged instructions - only work in ring 0
 */

#ifndef IO_H
#define IO_H

#include <stdint.h>

/**
 * @brief Output byte to I/O port
 * 
 * Writes an 8-bit value to the specified I/O port using the OUT instruction.
 * Used to send commands and data to hardware devices.
 * 
 * @param port I/O port number (0-65535)
 * @param value Byte value to write
 * 
 * Example:
 * @code
 * outb(0x3F8, 'H');  // Send 'H' to COM1 serial port
 * @endcode
 * 
 * @note Inline function - no call overhead
 */
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile(
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

/**
 * @brief Read byte from I/O port
 * 
 * Reads an 8-bit value from the specified I/O port using the IN instruction.
 * Used to read status and data from hardware devices.
 * 
 * @param port I/O port number (0-65535)
 * @return Byte value read from port
 * 
 * Example:
 * @code
 * uint8_t status = inb(0x3FD);  // Read COM1 line status
 * @endcode
 * 
 * @note Inline function - no call overhead
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    
    __asm__ volatile(
        "inb %1, %0"
        : "=a"(ret)
        : "Nd"(port)
    );
    return ret;
}

/**
 * @brief It's like inb but 16-bit (wide)
 * 
 * @param port 
 * @return uint16_t 
 */
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif // IO_H