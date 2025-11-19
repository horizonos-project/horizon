/**
 * @file pic.h
 * @brief Programmable Interrupt Controller (PIC) Management
 * 
 * Controls an Intel 8259 PIC chip (or compatible) which manages hardware
 * interrupts from devices. The PC has two PICs cascaded together, providing
 * 15 usable IRQ lines (IRQ 2 is used for cascading).
 * 
 * The PIC must be remapped on boot because its default IRQ mapping (0-15)
 * conflicts with CPU exception vectors. We remap to 32-47 to avoid conflicts.
 * 
 * PIC also handles interrupt masking (enabling/disabling specific IRQs) and
 * End-Of-Interrupt (EOI) signaling to tell the PIC "I'm done with this IRQ".
 * 
 * @note Modern systems use the APIC, but PIC is required for initial boot
 *       and is sufficient for single-core hobby OS development
 */

#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/**
 * @brief Remap the PIC to new interrupt vectors
 * 
 * Changes the PIC's IRQ mapping to avoid conflicts with CPU exceptions.
 * By default, PICs map IRQs 0-15 to interrupts 0x08-0x17, which overlaps
 * with CPU exceptions. We remap them to 0x20-0x2F (32-47).
 * 
 * @param offset1 New base vector for master PIC (IRQ 0-7)
 * @param offset2 New base vector for slave PIC (IRQ 8-15)
 * 
 * Standard remapping:
 * @code
 * pic_remap(0x20, 0x28);  // Master: 32-39, Slave: 40-47
 * @endcode
 * 
 * @note Must be called during interrupt initialization, before enabling IRQs
 */
void pic_remap(int offset1, int offset2);

/**
 * @brief Mask (disable) a specific IRQ line
 * 
 * Prevents the specified IRQ from triggering interrupts. Useful for
 * disabling noisy or unhandled hardware interrupts.
 * 
 * @param irq_line IRQ number to mask (0-15)
 * 
 * Example:
 * @code
 * pic_set_mask(1);  // Disable keyboard IRQ during critical section
 * @endcode
 */
void pic_set_mask(uint8_t irq_line);

/**
 * @brief Unmask (enable) a specific IRQ line
 * 
 * Allows the specified IRQ to trigger interrupts. Must be called after
 * registering an IRQ handler to start receiving interrupts from that device.
 * 
 * @param irq_line IRQ number to unmask (0-15)
 * 
 * Example:
 * @code
 * irq_register_handler(0, timer_handler);
 * pic_clear_mask(0);  // Enable timer IRQ
 * @endcode
 */
void pic_clear_mask(uint8_t irq_line);

/**
 * @brief Send End-Of-Interrupt signal to PIC
 * 
 * Notifies the PIC that interrupt handling is complete. Must be called
 * at the end of every IRQ handler, or the PIC will not send any more
 * interrupts from that IRQ line (or lower priority IRQs).
 * 
 * For IRQs 0-7 (master PIC), only master needs EOI.
 * For IRQs 8-15 (slave PIC), both slave and master need EOI.
 * 
 * @param irq IRQ number that was just handled (0-15)
 * 
 * Example:
 * @code
 * void timer_handler(regs_t *r) {
 *     ticks++;
 *     pic_send_eoi(0);  // Tell PIC we're done with IRQ 0
 * }
 * @endcode
 * 
 * @note Forgetting to send EOI will freeze that IRQ line!
 */
void pic_send_eoi(uint8_t irq);

/**
 * @brief Initialize the Programmable Interval Timer (PIT)
 * 
 * Configures the PIT (Intel 8253/8254 timer chip) to generate periodic
 * interrupts at the specified frequency. Used for:
 * - System clock / uptime tracking
 * - Preemptive multitasking (when we have a scheduler)
 * - Timing delays and timeouts
 * 
 * @param freq Desired interrupt frequency in Hz (typically 100-1000 Hz)
 * 
 * Common frequencies:
 * - 100 Hz: Standard for many systems (10ms per tick)
 * - 1000 Hz: Linux default (1ms per tick, better granularity)
 * - 18.2 Hz: Original IBM PC frequency (maybe don't use this!)
 * 
 * Example:
 * @code
 * pit_init(100);  // 100 Hz = timer tick every 10ms
 * @endcode
 * 
 * @note PIT generates IRQ 0
 */
void pit_init(uint32_t freq);

/**
 * @brief Check PIT status (debugging/diagnostics)
 * 
 * Reads and displays current PIT configuration and tick count.
 * Useful for verifying timer is running correctly.
 */
void pit_check(void);

#endif
