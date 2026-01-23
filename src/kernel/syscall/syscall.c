#include <stdint.h>
#include "syscall.h"
#include "kernel/idt.h"
#include "kernel/isr.h"
#include "kernel/log.h"
#include "kernel/errno.h"
#include "sys_process.h"

extern void isr_syscall_stub(void);

static syscall_t syscalls[MAX_SYSCALLS];

void syscall_register(uint8_t num, syscall_t func) {
    if (num >= MAX_SYSCALLS)
        return;

    syscalls[num] = func;
}

// Expand as needed since there's many many syscalls
void syscall_register_all(void) {
    syscall_register(SYS_EXIT,      sys_exit);
    syscall_register(SYS_GETPID,    sys_getpid);
    syscall_register(SYS_WRITE,     sys_write);
    syscall_register(SYS_READ,      sys_read);
    syscall_register(SYS_OPEN,      sys_open);
    syscall_register(SYS_CLOSE,     sys_close);
    syscall_register(SYS_FORK,      sys_fork);
    syscall_register(SYS_EXECVE,    sys_execve);
    syscall_register(SYS_BRK,       sys_brk);
    syscall_register(SYS_ALARM,     sys_alarm);
    syscall_register(SYS_CLEAR_VGA, sys_clear_vga);
}

void syscall_init(void) {
    idt_set_gate(0x80, (uint32_t)isr_syscall_stub, 0x08, 0xEF);
    klogf("[sysint] Interface created at vector 0x80.\n");
}

void syscall_handler(regs_t *r) {
    uint32_t num = r->eax;

    if (num >= MAX_SYSCALLS || syscalls[num] == SYSCALL_NULL) {
        klogf("[sysint] Unknown SYSCALL: %u\n", num);
        r->eax = (uint32_t)SYSCALL_ERR(ENOSYS);
        return;
    }

    uint32_t ret = syscalls[num](
        r->ebx, r->ecx, r->edx, r->esi, r->edi
    );

    r->eax = ret;
}
