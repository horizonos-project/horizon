// IDT.h --> Interrupt Descriptor Table
// (c) 2025- HorizonOS Project

#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// IDT Gate Descriptor (x86)
typedef struct __attribute__((packed)) {
    uint16_t offset_low;    // Bits 0..15 of handler addr
    uint16_t selector;      // Code segment selector in gdt
    uint8_t zero;           // Always zero, do not touch
    uint8_t type_attr;      // Type and Attributes
    uint16_t offset_high;   // Bits 16..31 of handler addr

} idt_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t limit;        // size of IDT - 1
    uint32_t base;         // address of first idt_entry_t
} idt_ptr_t;

// Installing / loading the IDT into our CPU, setting up handlers as well
void idt_init(void);

// These are called from assembly stubs

void isr_handler(uint32_t int_no, uint32_t err_code);
void irq_handler(uint32_t irq_no);

#endif // IDT_H