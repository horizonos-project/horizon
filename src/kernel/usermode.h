/**
 * @file usermode.h
 * @brief User Mode (Ring 3) Transition
 * 
 * Functions for transitioning from kernel mode (ring 0) to user mode (ring 3).
 * This is the Great Leap - going from "I can do anything" to "please sir,
 * may I have a syscall?"
 * 
 * The transition involves:
 * - Setting up user stack
 * - Loading user code segment (ring 3)
 * - Switching to user data segment (ring 3)
 * - Using IRET to drop privilege level
 * 
 * Once in usermode, the only way back to kernel is through interrupts
 * (syscalls, exceptions, hardware IRQs).
 * 
 * @note This is a one-way trip! No coming back without an interrupt.
 */

#ifndef USERMODE_H
#define USERMODE_H

#include <stdint.h>

/**
 * @brief Jump to user mode with specified stack
 * 
 * Performs the privilege level transition from ring 0 to ring 3 using
 * the IRET instruction. Sets up user segments, user stack, and drops
 * to ring 3.
 * 
 * This is typically used for the initial transition to userspace or
 * when manually setting up a user context.
 * 
 * @param user_stack Virtual address of the user stack top (ring 3 accessible)
 * 
 * @warning This function does not return! Execution continues at the
 *          address specified in the context setup (usually via IRET)
 * 
 * @note Assumes user code segment/data segment are already set up in GDT.
 *       This will hard crash Horizon if not set up proper.
 */
void jump_to_usermode(uint32_t user_stack);

/**
 * @brief Load and execute an ELF binary in user mode
 * 
 * The "easy mode" userspace launcher! Loads an ELF executable from the
 * VFS, sets up its memory space, stack, and jumps to its entry point
 * in ring 3.
 * 
 * Steps performed:
 * - 1. Open and parse ELF file from VFS
 * - 2. Allocate memory for program segments (code, data, BSS)
 * - 3. Load segments into memory
 * - 4. Set up user stack
 * - 5. Jump to ELF entry point in ring 3
 * 
 * @param path VFS path to ELF executable (e.g., "/bin/hello")
 * 
 * @warning This function does not return! The current kernel context
 *          is abandoned in favor of the user program.
 * 
 * Example:
 * @code
 * jump_to_elf("/bin/init");  // Boot into init process
 * // Never returns!
 * @endcode
 * 
 * @note Currently used for initial userspace bootstrap. Eventually
 *       this will be called by the scheduler for each new process.
 */
void jump_to_elf(const char *path);

#endif