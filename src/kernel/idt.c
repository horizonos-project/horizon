#include <stdint.h>
#include "../libk/string.h"
#include "idt.h"
#include "isr.h"

// ISRs from isr.asm
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

static idt_entry_t idt[256];
static idt_ptr_t   idt_ptr;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

// lidt instruction
static inline void idt_load(idt_ptr_t *idtp) {
    __asm__ volatile("lidt (%0)" :: "r"(idtp));
}

void idt_init(void) {
    // 256 entries, zeroed
    memset(idt, 0, sizeof(idt));

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint32_t)&idt;

    // Present, ring 0, 32-bit interrupt gate: 0x8E
    uint8_t flags = 0x8E;
    uint16_t kcode = 0x08; // kcode is 0x08

    idt_set_gate(0,  (uint32_t)isr0,  kcode, flags);
    idt_set_gate(1,  (uint32_t)isr1,  kcode, flags);
    idt_set_gate(2,  (uint32_t)isr2,  kcode, flags);
    idt_set_gate(3,  (uint32_t)isr3,  kcode, flags);
    idt_set_gate(4,  (uint32_t)isr4,  kcode, flags);
    idt_set_gate(5,  (uint32_t)isr5,  kcode, flags);
    idt_set_gate(6,  (uint32_t)isr6,  kcode, flags);
    idt_set_gate(7,  (uint32_t)isr7,  kcode, flags);
    idt_set_gate(8,  (uint32_t)isr8,  kcode, flags);
    idt_set_gate(9,  (uint32_t)isr9,  kcode, flags);
    idt_set_gate(10, (uint32_t)isr10, kcode, flags);
    idt_set_gate(11, (uint32_t)isr11, kcode, flags);
    idt_set_gate(12, (uint32_t)isr12, kcode, flags);
    idt_set_gate(13, (uint32_t)isr13, kcode, flags);
    idt_set_gate(14, (uint32_t)isr14, kcode, flags);
    idt_set_gate(15, (uint32_t)isr15, kcode, flags);
    idt_set_gate(16, (uint32_t)isr16, kcode, flags);
    idt_set_gate(17, (uint32_t)isr17, kcode, flags);
    idt_set_gate(18, (uint32_t)isr18, kcode, flags);
    idt_set_gate(19, (uint32_t)isr19, kcode, flags);
    idt_set_gate(20, (uint32_t)isr20, kcode, flags);
    idt_set_gate(21, (uint32_t)isr21, kcode, flags);
    idt_set_gate(22, (uint32_t)isr22, kcode, flags);
    idt_set_gate(23, (uint32_t)isr23, kcode, flags);
    idt_set_gate(24, (uint32_t)isr24, kcode, flags);
    idt_set_gate(25, (uint32_t)isr25, kcode, flags);
    idt_set_gate(26, (uint32_t)isr26, kcode, flags);
    idt_set_gate(27, (uint32_t)isr27, kcode, flags);
    idt_set_gate(28, (uint32_t)isr28, kcode, flags);
    idt_set_gate(29, (uint32_t)isr29, kcode, flags);
    idt_set_gate(30, (uint32_t)isr30, kcode, flags);
    idt_set_gate(31, (uint32_t)isr31, kcode, flags);

    idt_load(&idt_ptr);
}
