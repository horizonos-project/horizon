/**
 * @file isr.h
 * @brief Interrupt Service Routines (ISR) and IRQ Handlers
 * 
 * Defines interrupt service routines for CPU exceptions (ISR 0-31) and
 * hardware interrupt requests (IRQ 0-15). Each interrupt vector has a
 * corresponding assembly stub that saves CPU state and calls the C handler.
 * 
 * CPU Exceptions (ISR 0-31):
 * - 0: Divide by Zero
 * - 6: Invalid Opcode
 * - 13: General Protection Fault
 * - 14: Page Fault
 * - etc.
 * 
 * Hardware IRQs (32-47, remapped from 0-15):
 * - IRQ 0: System Timer (PIT)
 * - IRQ 1: Keyboard
 * - IRQ 14: Primary ATA Hard Disk
 * - etc.
 */

#ifndef ISR_H
#define ISR_H

#include <stdint.h>

/** @cond INTERNAL */
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
/** @endcond */

/** @cond INTERNAL */
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);
/** @endcond */

/**
 * @brief CPU register state at time of interrupt
 * 
 * Saved by the assembly interrupt stub and passed to C handlers.
 * Contains all general-purpose registers, segment registers, and
 * CPU state at the moment the interrupt occurred.
 * 
 * This structure allows handlers to:
 * - Inspect what the CPU was doing when interrupted
 * - Modify register state before returning (e.g., for syscalls)
 * - Debug crashes by examining register values
 */
typedef struct regs {
    uint32_t gs, fs, es, ds;       /**< Segment registers */
    uint32_t edi, esi, ebp, esp;   /**< General purpose registers */
    uint32_t ebx, edx, ecx, eax;   /**< General purpose registers */
    uint32_t int_no, err_code;     /**< Interrupt number and error code (if applicable) */
    uint32_t eip, cs, eflags;      /**< Pushed by CPU on interrupt */
    uint32_t useresp, ss;          /**< User stack (only if privilege change) */
} regs_t;

/**
 * @brief ISR handler function type
 * 
 * @param r Pointer to saved CPU register state
 */
typedef void (*isr_t)(regs_t* r);

/**
 * @brief IRQ handler function type
 * 
 * @param r Pointer to saved CPU register state
 */
typedef void (*irq_handler_t)(regs_t *r);

/**
 * @brief Install ISR handlers in the IDT
 * 
 * Sets up IDT entries 0-31 for CPU exception handlers.
 * Each entry points to the corresponding assembly stub (isr0-isr31).
 * 
 * @note Must be called after idt_init()
 */
void isr_install(void);

/**
 * @brief Install IRQ handlers and remap the PIC
 * 
 * Sets up IDT entries 32-47 for hardware interrupts and remaps
 * the Programmable Interrupt Controller (PIC) so IRQs don't
 * conflict with CPU exception vectors.
 * 
 * Default PIC mapping (IRQ 0-15 → INT 0x08-0x0F) conflicts with
 * CPU exceptions, so we remap to INT 0x20-0x2F (32-47).
 * 
 * @note Must be called after idt_init() and isr_install()
 */
void irq_install(void);

/**
 * @brief Main ISR dispatcher (called from assembly)
 * 
 * Called by assembly interrupt stubs when a CPU exception occurs.
 * Logs the exception and halts the system (for now - eventually
 * this should handle recoverable exceptions).
 * 
 * @param r Pointer to saved CPU register state
 * @internal
 */
void isr_handler(regs_t* r);

/**
 * @brief Register a handler for a specific IRQ
 * 
 * Allows drivers to install custom handlers for hardware interrupts.
 * When the specified IRQ fires, the registered handler will be called
 * with the CPU register state.
 * 
 * @param irq IRQ number (0-15)
 * @param handler Function to call when IRQ occurs
 * 
 * Example:
 * @code
 * void timer_handler(regs_t *r) {
 *     ticks++;
 *     // Handle timer tick
 * }
 * irq_register_handler(0, timer_handler);  // IRQ 0 = PIT timer
 * @endcode
 */
void irq_register_handler(uint8_t irq, irq_handler_t handler);

/**
 * @brief Main IRQ dispatcher (called from assembly)
 * 
 * Called by assembly interrupt stubs when a hardware IRQ occurs.
 * Dispatches to the registered handler (if any) and sends EOI to PIC.
 * 
 * @param r Pointer to saved CPU register state
 * @internal
 */
void irq_handler(regs_t *r);

#endif // ISR_H
