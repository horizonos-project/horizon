#ifndef SYSCALL_H
#define SYSCALL_H

#include "kernel/isr.h"
#include <stdint.h>

#define SYSCALL_NULL ((void*)0)

// Linux compatible syscall numbers
#define SYS_EXIT    1
#define SYS_FORK    2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_EXECVE  11
#define SYS_GETPID  20
#define SYS_ALARM   27
#define SYS_BRK     45

typedef uint32_t (*syscall_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

#define MAX_SYSCALLS 256

void syscall_init(void);
void syscall_register(uint8_t num, syscall_t func);
void syscall_register_all(void);
void syscall_handler(regs_t *r);

#endif // SYSCALL_H