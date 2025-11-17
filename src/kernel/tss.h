#ifndef TSS_H
#define TSS_H

#include <stdint.h>

typedef struct {
    uint32_t prev_tss;   // Previous TSS (do we need this?)
    uint32_t esp0;       // Kernel stack pointer (super useful)
    uint32_t ss0;        // Kernel stack segment (0x10)
    uint32_t esp1;       // Unused :(
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;        // Page directory
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) tss_t;

void tss_install(uint32_t kernel_stack);
void tss_set_kernel_stack(uint32_t stack);

#endif // TSS_H