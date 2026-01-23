/**
 * @file syscall.h
 * @brief System Call Interface
 * 
 * Provides the bridge between user mode (ring 3) and kernel mode (ring 0).
 * When userspace needs kernel services (file I/O, memory allocation, process
 * management), it triggers interrupt 0x80 with a syscall number in EAX.
 * 
 * The syscall handler:
 * - 1. Saves user context (registers)
 * - 2. Looks up syscall number in dispatch table
 * - 3. Calls the corresponding kernel function
 * - 4. Returns result in EAX
 * - 5. Restores user context and returns to ring 3
 * 
 * We use Linux-compatible syscall numbers so userspace tools are (somewhat)
 * portable. Not all syscalls are implemented yet - some are stubs!
 * 
 * @note Horizon unique syscalls start at 200 and end at 254. Due to how few
 *       OS-specific syscalls we will need, I believe 54 is plenty. If we need
 *       any more... that may complicate things.
 * 
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#include "kernel/isr.h"
#include <stdint.h>

/** @brief Null pointer constant for unimplemented syscalls */
#define SYSCALL_NULL ((void*)0)

/** @brief Terminate process */
#define SYS_EXIT    1

/** @brief Create child process (not implemented yet) */
#define SYS_FORK    2

/** @brief Read from file descriptor */
#define SYS_READ    3

/** @brief Write to file descriptor */
#define SYS_WRITE   4

/** @brief Open file */
#define SYS_OPEN    5

/** @brief Close file descriptor */
#define SYS_CLOSE   6

/** @brief Execute program (not fully implemented yet) */
#define SYS_EXECVE  11

/** @brief Get process ID */
#define SYS_GETPID  20

/** @brief Set alarm timer (stub) */
#define SYS_ALARM   27

/** @brief Adjust program break (heap allocation) */
#define SYS_BRK     45

/** @brief Clears VGA memory (HorizonOS specific) */
#define SYS_CLEAR_VGA 200

/**
 * @brief Syscall handler function type
 * 
 * All syscall implementations follow this signature. Takes up to 5
 * arguments from registers (EBX, ECX, EDX, ESI, EDI) and returns
 * a result in EAX.
 * 
 * @param arg1 First argument (from EBX)
 * @param arg2 Second argument (from ECX)
 * @param arg3 Third argument (from EDX)
 * @param arg4 Fourth argument (from ESI)
 * @param arg5 Fifth argument (from EDI)
 * @return Syscall result (returned to user in EAX)
 * 
 * Example implementation:
 * @code
 * uint32_t sys_getpid(uint32_t u1, uint32_t u2, uint32_t u3, 
 *                     uint32_t u4, uint32_t u5) {
 *     (void)u1; (void)u2; (void)u3; (void)u4; (void)u5;
 *     return current_process->pid;
 * }
 * @endcode
 */
typedef uint32_t (*syscall_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

/** @brief Maximum number of syscalls supported */
#define MAX_SYSCALLS 255

/**
 * @brief Initialize the syscall subsystem
 * 
 * Sets up interrupt 0x80 as the syscall vector and initializes
 * the syscall dispatch table. Must be called during kernel init
 * before userspace is entered.
 * 
 * @note Call this after IDT and interrupt handlers are set up
 */
void syscall_init(void);

/**
 * @brief Register a single syscall handler
 * 
 * Adds a syscall implementation to the dispatch table. The handler
 * will be called when userspace triggers INT 0x80 with the specified
 * syscall number in EAX.
 * 
 * @param num Syscall number (e.g., SYS_WRITE = 4)
 * @param func Pointer to syscall implementation function
 * 
 * Example:
 * @code
 * syscall_register(SYS_WRITE, sys_write);
 * syscall_register(SYS_EXIT, sys_exit);
 * @endcode
 */
void syscall_register(uint8_t num, syscall_t func);

/**
 * @brief Register all implemented syscalls
 * 
 * Convenience function that registers all the syscall implementations
 * in one go. Called during syscall_init().
 * 
 * Currently registers:
 * - sys_exit, sys_write, sys_read, sys_open, sys_close
 * - sys_getpid, sys_brk, sys_fork (stub), sys_execve (stub), sys_alarm (stub)
 * 
 * @note Add new syscalls here as they're implemented
 */
void syscall_register_all(void);

/**
 * @brief Main syscall dispatcher (INT 0x80 handler)
 * 
 * Called when userspace executes INT 0x80. Reads the syscall number
 * from EAX, looks it up in the dispatch table, and calls the
 * corresponding handler with arguments from EBX, ECX, EDX, ESI, EDI.
 * 
 * The return value is placed in EAX and returned to userspace.
 * 
 * @param r Pointer to saved CPU register state
 * 
 * @internal This is called by the assembly interrupt stub
 */
void syscall_handler(regs_t *r);

#endif // SYSCALL_H