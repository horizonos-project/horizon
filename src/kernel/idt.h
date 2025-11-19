/**
 * @file idt.h
 * @brief Interrupt Descriptor Table (IDT) Management
 * 
 * The IDT maps interrupt and exception vectors to their handler functions.
 * When a hardware interrupt occurs (keyboard, timer, etc.) or the CPU
 * encounters an exception (page fault, divide by zero), it uses the IDT
 * to find the appropriate handler routine.
 * 
 * This is how we know what's going on as it's happening, rather than
 * just assuming everything is going OK and crossing our fingers! 🤞
 * 
 * The IDT handles:
 * - CPU exceptions (0-31): divide by zero, page faults, general protection, etc.
 * - Hardware IRQs (32-47): timer, keyboard, disk, network, etc.
 * - Software interrupts (0x80): syscall interface
 * 
 * @note x86 architecture requirement - must be set up before enabling interrupts
 */

#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/**
 * @brief IDT entry structure
 * 
 * Describes a single interrupt/exception handler in the table.
 * Each entry points to an interrupt service routine (ISR) and
 * specifies its privilege level and type.
 * 
 * The structure matches the x86 hardware format exactly and must
 * be packed to prevent compiler padding.
 */
typedef struct {
    uint16_t base_low;   /**< Lower 16 bits of handler address */
    uint16_t sel;        /**< Kernel code segment selector (usually 0x08) */
    uint8_t  always0;    /**< Always zero (reserved by x86) */
    uint8_t  flags;      /**< Gate type, DPL, and present bit */
    uint16_t base_high;  /**< Upper 16 bits of handler address */
} __attribute__((packed)) idt_entry_t;

/**
 * @brief IDT pointer structure
 * 
 * Loaded into the IDTR register via the LIDT instruction.
 * Points to the IDT and specifies its size.
 */
typedef struct {
    uint16_t limit;  /**< Size of IDT in bytes minus 1 */
    uint32_t base;   /**< Linear address of the IDT */
} __attribute__((packed)) idt_ptr_t;

/**
 * @brief Initialize the Interrupt Descriptor Table
 * 
 * Sets up all 256 IDT entries and installs handlers for:
 * - CPU exceptions (vectors 0-31)
 * - Hardware IRQs (vectors 32-47, remapped from 0-15)
 * - Syscall interface (vector 0x80)
 * 
 * After initialization, the IDT is loaded into the CPU via LIDT.
 * Interrupts remain disabled until explicitly enabled with STI.
 * 
 * @note Must be called before enabling interrupts
 * @see idt_set_gate()
 */
void idt_init(void);

/**
 * @brief Set an IDT gate entry
 * 
 * Configures a single IDT entry to point to an interrupt handler.
 * 
 * @param num Interrupt vector number (0-255)
 * @param base Address of the interrupt handler function
 * @param sel Code segment selector (usually 0x08 for kernel code)
 * @param flags Gate flags:
 *              - Bit 7: Present (must be 1 for active gates)
 *              - Bits 5-6: DPL (privilege level: 0=kernel, 3=user)
 *              - Bits 0-4: Gate type (0x0E = 32-bit interrupt gate)
 * 
 * Common flag values:
 * - 0x8E: 32-bit interrupt gate, ring 0 (kernel interrupts)
 * - 0xEE: 32-bit interrupt gate, ring 3 (syscalls from userspace)
 * 
 * Example:
 * @code
 * idt_set_gate(0x80, (uint32_t)syscall_handler, 0x08, 0xEE);
 * @endcode
 */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

#endif // IDT_H