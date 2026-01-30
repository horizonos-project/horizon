#ifndef SYSCALL_DEFS_H
#define SYSCALL_DEFS_H

#include <stdint.h>

/*
 * Canonical Linux-i386 syscall ABI:
 *   EBX, ECX, EDX, ESI, EDI, EBP
 */
#define SYSCALL_ARGS \
    uint32_t a1, uint32_t a2, uint32_t a3, \
    uint32_t a4, uint32_t a5, uint32_t a6

/*
 * Declare / define a syscall implementation.
 *
 * Usage:
 *   SYSCALL(sys_write) {
 *       uint32_t fd   = a1;
 *       uint32_t buf  = a2;
 *       uint32_t len  = a3;
 *       ...
 *   }
 */
#define SYSCALL(name) \
    int32_t name(SYSCALL_ARGS)

/**
 * For those unfinished syscalls in your life
 */
#define SYSCALL_IGNORE() do { (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6; } while (0)

#endif /* SYSCALL_DEFS_H */