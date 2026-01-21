/**
 * @file kprint.h
 * @brief Kernel Printf Implementation
 * 
 * Provides formatted output functions for the kernel. Similar to standard
 * printf but outputs to configurable backend(s) such as VGA text mode,
 * serial ports, or framebuffers.
 * 
 * The output destination can be changed at runtime using kprint_set_backend(),
 * allowing flexible logging to different devices during boot and operation.
 * 
 * @note This is a freestanding implementation - no libc dependency
 */

#ifndef KPRINT_H
#define KPRINT_H

#include <stdarg.h>
#include <stdint.h>

/**
 * @brief Function pointer type for character output
 * 
 * Backend functions should implement this signature to receive
 * individual characters from the formatter.
 * 
 * @param c Character to output
 */
typedef void (*kputc_fn_t)(char);

/**
 * @brief Set the output sink for kernel printing
 * 
 * Configures where kernel print output should be sent. The sink
 * function will be called for each character that needs to be displayed.
 * 
 * @param sink Function to call for each output character
 * 
 * @note This is an alias for kprint_set_backend()
 */
void kset_sink(void (*sink)(char));

/**
 * @brief Set the backend for kernel printing
 * 
 * Configures the character output function for all kernel print operations.
 * Common backends include VGA text mode, serial output, or framebuffers.
 * 
 * @param fn Function pointer to character output routine
 */
void kprint_set_backend(kputc_fn_t fn);

/**
 * @brief Formatted kernel print
 * 
 * Printf-style formatted output to the configured backend.
 * 
 * Supported format specifiers:
 * - %s: null-terminated string
 * - %c: single character
 * - %d, %i: signed decimal integer
 * - %u: unsigned decimal integer
 * - %x: unsigned hexadecimal (lowercase)
 * - %X: unsigned hexadecimal (uppercase)
 * - %p: pointer (as hexadecimal with 0x prefix)
 * - %%: literal percent sign
 * 
 * @param fmt Format string (printf-style)
 * @param ... Variable arguments matching format specifiers
 * 
 * Example:
 * @code
 * // ...
 * kprintf("Boot: %s at 0x%x\n", "kernel", 0x100000);
 * @endcode
 */
void kprintf(const char *fmt, ...);

/**
 * @brief Formatted kernel print with va_list
 * 
 * Like kprintf() but accepts a va_list instead of variadic arguments.
 * Useful for wrapping kprintf in other variadic functions.
 * 
 * @param fmt Format string (printf-style)
 * @param args Variable argument list (from va_start)
 * 
 * Example:
 * @code
 * void log(const char *fmt, ...) {
 *     va_list args;
 *     va_start(args, fmt);
 *     kvprintf(fmt, args);
 *     va_end(args);
 * }
 * @endcode
 */
void kvprintf(const char *fmt, va_list args);

/**
 * @brief Output a single character to the kernel print backend
 * 
 * Sends a single character to the currently configured output sink.
 * 
 * @param c Character to output
 */
void kputc(char c);

#endif // KPRINT_H
