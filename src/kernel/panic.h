/**
 * @file panic.h
 * @brief Kernel Panic Handler
 * 
 * When things go catastrophically wrong and the kernel cannot continue
 * safely (triple fault territory, corrupt state, impossible conditions),
 * we panic. This is the nuclear option - halts the system after displaying
 * an error message.
 * 
 * Common panic scenarios:
 * - Page fault in kernel code with no handler
 * - Failed assertion in critical code path
 * - Hardware malfunction detected
 * - Corrupt kernel data structures
 * - Out of memory during critical operation
 * 
 * Think of it as the kernel's way of saying "I give up, please reboot me"
 *
 * @note Once panicf() is called, the system is HALTED - cannot recover
 */

#pragma once

/**
 * @brief Trigger a kernel panic with formatted message
 * 
 * Displays a formatted error message and halts the system. This function
 * never returns - the CPU is put into an infinite halt loop.
 * 
 * The panic handler typically:
 * - Disables interrupts (no more surprises)
 * - Displays the panic message to all available outputs
 * - Dumps CPU register state (if available)
 * - Halts the CPU with HLT instruction
 * 
 * @param fmt Format string (printf-style) describing the panic reason
 * @param ... Variable arguments for format string
 * 
 * @return Never returns - system is halted
 * 
 * Example:
 * @code
 * if (page_directory == NULL) {
 *     panicf("Failed to allocate page directory at 0x%x", addr);
 * }
 * 
 * if (magic != EXPECTED_MAGIC) {
 *     panicf("Corrupted data structure: expected 0x%x, got 0x%x",
 *            EXPECTED_MAGIC, magic);
 * }
 * @endcode
 * 
 * @note The __attribute__((noreturn)) tells the compiler this function
 *       never returns, allowing for optimizations and better warnings
 */
void panicf(const char *fmt, ...) __attribute__((noreturn));