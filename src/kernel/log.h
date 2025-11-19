/**
 * @file log.h
 * @brief Kernel Logging System
 * 
 * Provides a flexible logging infrastructure that can output to multiple
 * destinations simultaneously (VGA console, serial port, framebuffer, etc.).
 * 
 * The logging system uses a sink-based architecture where multiple output
 * backends can be registered. Each log message is dispatched to all
 * registered sinks, allowing redundant logging for debugging.
 * 
 * Common use cases:
 * - Boot messages visible on screen (VGA) and captured via serial (debugging)
 * - Kernel warnings/errors logged to both console and persistent storage
 * - Debug output sent only to serial port to avoid cluttering the display
 */

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdbool.h>

/**
 * @brief Log output sink function type
 * 
 * Backend functions should implement this signature to receive
 * log messages. The function will be called with complete formatted
 * strings ready for output.
 * 
 * @param s Null-terminated string to output
 */
typedef void (*log_write_t)(const char *s);

/**
 * @brief Initialize the kernel logging system
 * 
 * Sets up the logging infrastructure and prepares for sink registration.
 * Should be called early in kernel initialization before any log output.
 * 
 * @note Must be called before log_register_sink() or klogf()
 */
void log_init(void);

/**
 * @brief Register a logging output sink
 * 
 * Adds a new output destination for kernel log messages. Multiple sinks
 * can be registered, and all will receive copies of each log message.
 * 
 * @param sink Function to call for each log message
 * 
 * Example:
 * @code
 * log_register_sink(serial_write);  // Log to serial port
 * log_register_sink(vga_write);     // Also log to VGA display
 * @endcode
 */
void log_register_sink(log_write_t sink);

/**
 * @brief Write a string to all log sinks
 * 
 * Outputs a raw string to all registered logging backends.
 * No formatting or newline handling - string is output as-is.
 * 
 * @param s Null-terminated string to log
 * 
 * @note For formatted output, use klogf() instead
 */
void log_puts(const char *s);

/**
 * @brief Formatted kernel logging
 * 
 * Printf-style formatted logging to all registered sinks.
 * Typically used for kernel debug messages, status updates, and warnings.
 * 
 * Outputs are prefixed with tags like [boot], [mm], [vfs], etc. for clarity.
 * 
 * @param fmt Format string (printf-style)
 * @param ... Variable arguments matching format specifiers
 * 
 * Example:
 * @code
 * klogf("[boot] Kernel loaded at 0x%x\n", kernel_addr);
 * klogf("[mm] PMM initialized: %u MB available\n", mb_free);
 * @endcode
 * 
 * @see kprintf() for format specifier details
 */
void klogf(const char *fmt, ...);

/**
 * @brief Print to both kernel log and user-visible output
 * 
 * Dual-output function that sends formatted text to both the kernel
 * logging system (serial, log files) and user-facing display (VGA/framebuffer).
 * 
 * Useful for messages that should be visible to users but also captured
 * in logs for debugging (e.g., critical errors, boot progress).
 * 
 * @param fmt Format string (printf-style)
 * @param ... Variable arguments matching format specifiers
 * 
 * Example:
 * @code
 * kprintf_both("Welcome to HorizonOS!\n");
 * kprintf_both("Critical error: Failed to mount root filesystem\n");
 * @endcode
 */
void kprintf_both(const char *fmt, ...);

#endif
