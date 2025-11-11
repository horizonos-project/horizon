#include <stdint.h>
#include "kernel/idt.h"
#include "kernel/isr.h"
#include "libk/kprint.h"

typedef uint32_t (*syscall_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

#define MAX_SYSCALLS 256
static syscall_t syscalls[MAX_SYSCALLS];

void syscall_register(uint8_t num, syscall_t func)
{
    syscalls[num] = func;
}

void syscall_init(void)
{
    extern void isr_syscall_stub(void);
    idt_set_gate(0x80, (uint32_t)isr_syscall_stub, 0x08, 0xEE);
    kprintf("[syscall] Interface initialized at vector 0x80\n");
}

void syscall_handler(regs_t *r)
{
    uint32_t num = r->eax;
    if (num < MAX_SYSCALLS && syscalls[num]) {
        r->eax = syscalls[num](r->ebx, r->ecx, r->edx, r->esi, r->edi);
    } else {
        kprintf("[syscall] Invalid syscall: %u\n", num);
    }
}
