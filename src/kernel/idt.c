// IDT implementation file
// (c) 2025- HorizonOS Project

#include <stdint.h>

#include "idt.h"
#include "io.h"
#include "pic.h"

// 256 IDT entries
static idt_entry_t idt[256];
static idt_ptr_t idt_descriptor;

// Stubs to be defined in isr_stubs.asm

// CPU Execptions:
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

// Hardware related IRQs (post PIC remap, will live at 0x20..0x2F)
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

// Helper to set a gate in IDT
static void idt_set_gate(int vec, uint32_t handler, uint16_t selector, uint8_t type_attr)
{
    idt[vec].offset_low  = handler & 0xFFFF;
    idt[vec].selector    = selector;
    idt[vec].zero        = 0;
    idt[vec].type_attr   = type_attr;
    idt[vec].offset_high = (handler >> 16) & 0xFFFF;
}

void idt_init(void)
{
    // 1. Build the IDT pointer
    idt_descriptor.limit = sizeof(idt) - 1;
    idt_descriptor.base  = (uint32_t)&idt[0];

    // 2. Zero the table initially
    for (int i = 0; i < 256; i++)
    {
        idt_set_gate(i, 0, 0, 0);
    }

    // 3. CPU exception gates (0..31)
    // type_attr 0x8E = present, DPL=0, 32-bit interrupt gate
    uint8_t gate_flags = 0x8E;

     // we'll assume code segment selector = 0x08 in the GDT GRUB set up
    uint16_t kernel_cs = 0x08;

    idt_set_gate(0,  (uint32_t)isr0,  kernel_cs, gate_flags);
    idt_set_gate(1,  (uint32_t)isr1,  kernel_cs, gate_flags);
    idt_set_gate(2,  (uint32_t)isr2,  kernel_cs, gate_flags);
    idt_set_gate(3,  (uint32_t)isr3,  kernel_cs, gate_flags);
    idt_set_gate(4,  (uint32_t)isr4,  kernel_cs, gate_flags);
    idt_set_gate(5,  (uint32_t)isr5,  kernel_cs, gate_flags);
    idt_set_gate(6,  (uint32_t)isr6,  kernel_cs, gate_flags);
    idt_set_gate(7,  (uint32_t)isr7,  kernel_cs, gate_flags);
    idt_set_gate(8,  (uint32_t)isr8,  kernel_cs, gate_flags);
    idt_set_gate(9,  (uint32_t)isr9,  kernel_cs, gate_flags);
    idt_set_gate(10, (uint32_t)isr10, kernel_cs, gate_flags);
    idt_set_gate(11, (uint32_t)isr11, kernel_cs, gate_flags);
    idt_set_gate(12, (uint32_t)isr12, kernel_cs, gate_flags);
    idt_set_gate(13, (uint32_t)isr13, kernel_cs, gate_flags);
    idt_set_gate(14, (uint32_t)isr14, kernel_cs, gate_flags);
    idt_set_gate(15, (uint32_t)isr15, kernel_cs, gate_flags);
    idt_set_gate(16, (uint32_t)isr16, kernel_cs, gate_flags);
    idt_set_gate(17, (uint32_t)isr17, kernel_cs, gate_flags);
    idt_set_gate(18, (uint32_t)isr18, kernel_cs, gate_flags);
    idt_set_gate(19, (uint32_t)isr19, kernel_cs, gate_flags);
    idt_set_gate(20, (uint32_t)isr20, kernel_cs, gate_flags);
    idt_set_gate(21, (uint32_t)isr21, kernel_cs, gate_flags);
    idt_set_gate(22, (uint32_t)isr22, kernel_cs, gate_flags);
    idt_set_gate(23, (uint32_t)isr23, kernel_cs, gate_flags);
    idt_set_gate(24, (uint32_t)isr24, kernel_cs, gate_flags);
    idt_set_gate(25, (uint32_t)isr25, kernel_cs, gate_flags);
    idt_set_gate(26, (uint32_t)isr26, kernel_cs, gate_flags);
    idt_set_gate(27, (uint32_t)isr27, kernel_cs, gate_flags);
    idt_set_gate(28, (uint32_t)isr28, kernel_cs, gate_flags);
    idt_set_gate(29, (uint32_t)isr29, kernel_cs, gate_flags);
    idt_set_gate(30, (uint32_t)isr30, kernel_cs, gate_flags);
    idt_set_gate(31, (uint32_t)isr31, kernel_cs, gate_flags);

    // 4. IRQs from PIC after remap (32..47)
    idt_set_gate(32, (uint32_t)irq0,  kernel_cs, gate_flags);
    idt_set_gate(33, (uint32_t)irq1,  kernel_cs, gate_flags);
    idt_set_gate(34, (uint32_t)irq2,  kernel_cs, gate_flags);
    idt_set_gate(35, (uint32_t)irq3,  kernel_cs, gate_flags);
    idt_set_gate(36, (uint32_t)irq4,  kernel_cs, gate_flags);
    idt_set_gate(37, (uint32_t)irq5,  kernel_cs, gate_flags);
    idt_set_gate(38, (uint32_t)irq6,  kernel_cs, gate_flags);
    idt_set_gate(39, (uint32_t)irq7,  kernel_cs, gate_flags);
    idt_set_gate(40, (uint32_t)irq8,  kernel_cs, gate_flags);
    idt_set_gate(41, (uint32_t)irq9,  kernel_cs, gate_flags);
    idt_set_gate(42, (uint32_t)irq10, kernel_cs, gate_flags);
    idt_set_gate(43, (uint32_t)irq11, kernel_cs, gate_flags);
    idt_set_gate(44, (uint32_t)irq12, kernel_cs, gate_flags);
    idt_set_gate(45, (uint32_t)irq13, kernel_cs, gate_flags);
    idt_set_gate(46, (uint32_t)irq14, kernel_cs, gate_flags);
    idt_set_gate(47, (uint32_t)irq15, kernel_cs, gate_flags);

    // 5. Actually load IDT with lidt
    __asm__ volatile ("lidtl (%0)" :: "r"(&idt_descriptor));

    // 6. Remap the PIC so IRQs map to 32..47
    pic_remap();

    // 7. Enable interrupts globally
    __asm__ volatile ("sti");
}

// ----------------------------------------------------------------------------
// C-level handlers
// ----------------------------------------------------------------------------

// CPU Exception handler
void isr_handler(uint32_t int_no, uint32_t err_code)
{
    // Hanging so we can see faults in GDB
    // Should replace with a kprintf type func eventually
    (void)err_code;
    for (;;)
    {
        __asm__ volatile("hlt");
    }
}

// Hardware IRQ handler
void irq_handler(uint32_t irq_no)
{
    // End-of-interrupt to PIC
    if (irq_no >= 8)
    {
        // Slave PIC
        outb(0xA0, 0x20);
    }
    // Master PIC
    outb(0x20, 0x20);

    // For now just return.
}
