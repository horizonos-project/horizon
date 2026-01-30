#ifndef SYS_PROCESS_H
#define SYS_PROCESS_H
/**
 * @file sys_process.h
 * @brief HorizonOS system call implementations (kernel side).
 *
 * This header declares the kernel syscall entrypoints that are registered
 * in the syscall dispatch table (INT 0x80).
 *
 * ## ABI / Calling Convention (Linux-i386 style)
 * Syscall number:
 *   - EAX = syscall number
 *
 * Arguments:
 *   - EBX, ECX, EDX, ESI, EDI, EBP = up to 6 arguments
 *
 * Return:
 *   - EAX = result (>= 0 on success)
 *   - EAX = -errno on failure (negative error number)
 *
 * Notes:
 * - All syscall arguments are passed as 32-bit values (ILP32 environment).
 * - Pointer arguments are passed as user virtual addresses (uint32_t) and must
 *   be validated before dereference (future: copyin/copyout).
 * - Many syscalls here are stubs until scheduling / full userspace exists.
 */

#include <stdint.h>
#include "kernel/errno.h"  // EPERM, ENOENT, ... + SYSCALL_ERR()
#include "syscall_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Kernel syscall function signature.
 *
 * All syscall handlers must match this signature so they can be placed in the
 * syscall dispatch table without casting.
 *
 * @return >=0 on success, -errno on failure.
 */
typedef int32_t (*syscall_impl_t)(
    uint32_t a1, uint32_t a2, uint32_t a3,
    uint32_t a4, uint32_t a5, uint32_t a6
);

/* -------------------------------------------------------------------------- */
/* Helpers                                                                     */
/* -------------------------------------------------------------------------- */

/**
 * @brief Mark unused syscall arguments to silence warnings in stub handlers.
 */
#define SYSCALL_UNUSED_ARGS(...) do { (void)sizeof((int[]){0, ((void)(__VA_ARGS__), 0)...}); } while (0)
/* If your compiler doesn't like the above trick, you can use:
 *   #define SYSCALL_UNUSED_ARGS(...) do { (void)0; } while (0)
 * and explicitly (void)arg; lines instead.
 */

/* -------------------------------------------------------------------------- */
/* Syscalls                                                                     */
/* -------------------------------------------------------------------------- */

/**
 * @brief SYS_EXIT (1): Terminate the calling process.
 *
 * Linux semantics: exit() does not return.
 * Horizon currently has no scheduler; this halts the system.
 *
 * @param status Exit code (low 8 bits typically used by shells).
 * @return Never returns; if it does, returns -ENOSYS / or 0 by convention.
 */
SYSCALL(sys_exit);

/**
 * @brief SYS_GETPID (20): Get process ID.
 *
 * Horizon currently returns PID 1 (single "process").
 *
 * @return PID on success.
 */
SYSCALL(sys_getpid);

/**
 * @brief SYS_WRITE (4): Write to a file descriptor.
 *
 * Horizon currently supports:
 *  - fd 1 (stdout): VGA + serial
 *  - fd 2 (stderr): VGA + serial
 * Other FDs: returns -EBADF (until writable FS / VFS write is added)
 *
 * @param fd    File descriptor.
 * @param buf   User pointer to bytes to write.
 * @param count Number of bytes.
 * @return Number of bytes written on success, -errno on failure.
 */
SYSCALL(sys_write);

/**
 * @brief SYS_READ (3): Read from a file descriptor.
 *
 * Horizon currently supports:
 *  - fd 0 (stdin): keyboard stream (blocks until at least 1 byte available)
 * Other FDs: forwarded to VFS read (if supported)
 *
 * @param fd    File descriptor.
 * @param buf   User pointer to destination buffer.
 * @param count Max bytes to read.
 * @return Bytes read (0 = EOF) or -errno on failure.
 */
SYSCALL(sys_read);

/**
 * @brief SYS_OPEN (5): Open a file.
 *
 * Currently calls vfs_open(path, flags).
 *
 * @param pathname User pointer to NUL-terminated path string.
 * @param flags    Open flags (O_RDONLY etc).
 * @param mode     File mode (unused until create support exists).
 * @return FD (>=0) on success, -errno on failure.
 */
SYSCALL(sys_open);

/**
 * @brief SYS_CLOSE (6): Close a file descriptor.
 *
 * Horizon currently disallows closing stdio fds (0/1/2) as a stopgap.
 *
 * @param fd File descriptor.
 * @return 0 on success, -errno on failure.
 */
SYSCALL(sys_close);

/**
 * @brief SYS_FORK (2): Create a child process.
 *
 * Stub until scheduler / process model is implemented.
 *
 * Linux semantics: returns 0 in child, child's PID in parent, -errno on failure.
 *
 * @return -EAGAIN (stub) or proper fork result later.
 */
SYSCALL(sys_fork);

/**
 * @brief SYS_EXECVE (11): Execute a program image.
 *
 * Stub until ELF loader + proper userspace process replacement exists.
 *
 * @param filename User pointer to path.
 * @param argv     User pointer to argv array (char*[]), NUL-terminated.
 * @param envp     User pointer to envp array (char*[]), NUL-terminated.
 * @return On success, does not return. On failure, -errno.
 */
SYSCALL(sys_execve);

/**
 * @brief SYS_BRK (45): Adjust program break.
 *
 * Horizon currently implements a simple heap starting at 0x40000000 and maps
 * pages on-demand in the current address space.
 *
 * Linux semantics:
 *  - brk(0) returns current break
 *  - brk(new) returns new break on success (or current break if rejected)
 *
 * Important: Linux does not return -errno for brk failures; it returns the
 * current break. Horizon follows that "return current break" behavior.
 *
 * @param addr New break address, or 0 to query.
 * @return Current/new break value.
 */
SYSCALL(sys_brk);

/**
 * @brief SYS_ALARM (27): Set an alarm timer in seconds.
 *
 * Stub until timer interrupts + signals exist.
 *
 * Linux semantics:
 *  - alarm(0) cancels pending alarm
 *  - returns seconds remaining on previous alarm
 *
 * @param seconds Seconds until SIGALRM.
 * @return 0 (stub) or remaining seconds later.
 */
SYSCALL(sys_alarm);

/**
 * @brief Horizon syscall: clear VGA output buffer.
 *
 * Horizon-specific syscall range begins at 500 (by current project convention).
 *
 * @return 0 on success.
 */
SYSCALL(sys_clear_vga);

#ifdef __cplusplus
}
#endif

#endif /* SYS_PROCESS_H */
