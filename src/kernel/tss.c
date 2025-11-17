#include "tss.h"
#include "gdt.h"
#include "kernel/log.h"
#include "../libk/string.h"

static tss_t tss;

void tss_install(uint32_t kernel_stack) {
    kprintf_both("[tss] Installing TSS...\n");

    memset(&tss, 0, sizeof(tss));

    // Setting up kernel stack for r3 -> r0 transition
    tss.ss0 = 0x10;
    tss.esp0 = kernel_stack;

    kprintf_both("[tss] Kernel stack: 0x%08x\n", kernel_stack);
    kprintf_both("[tss] Kernel 'SS0': 0x%08x\n", tss.ss0);

    // Set up segment registers
    tss.cs = 0x0b;  // User code segment (0x08 | 3)
    tss.ss = 0x13;  // User data segment (0x10 | 3)
    tss.ds = 0x13;
    tss.es = 0x13;
    tss.fs = 0x13;
    tss.gs = 0x13;

    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss) - 1;
    
    // gran = 0x0 for byte. granularity, not 4KB pages
    gdt_set_gate(5, base, limit, 0x89, 0x00);
    
    klogf("[tss] TSS descriptor added to GDT entry 5\n");
    klogf("[tss] Base: 0x%08x, Limit: 0x%08x\n", base, limit);

    // Loading tss via inline asm
    __asm__ volatile(
        "ltr %%ax" : : "a"(0x28)
    );

    kprintf_both("[ok] tss loaded (selector 0x28)\n");
}

// Updates the kernel_stack pointer
// when we have multiple processes
void tss_set_kernel_stack(uint32_t stack) {
    tss.esp0 = stack;
}
