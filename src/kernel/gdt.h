/**
 * @file gdt.h
 * @brief Global Descriptor Table (GDT) Management
 * 
 * The GDT defines memory segments and privilege levels for protected mode.
 * HorizonOS uses a flat memory model with separate code and data segments
 * for kernel (ring 0) and user (ring 3) modes.
 * 
 * The GDT is required for:
 * - Protected mode operation
 * - Privilege level enforcement (ring 0 vs ring 3)
 * - Task State Segment (TSS) for hardware task switching
 * 
 * @note x86 architecture requirement - must be set up early in boot
 */

#pragma once
#include <stdint.h>

/**
 * @brief GDT entry structure
 * 
 * Describes a single segment in the Global Descriptor Table.
 * Each segment defines a region of memory with specific access rights
 * and privilege levels.
 * 
 * The structure matches the x86 hardware format exactly and must
 * be packed to prevent compiler padding.
 */
struct gdt_entry {
    uint16_t limit_low;     /**< Lower 16 bits of segment limit */
    uint16_t base_low;      /**< Lower 16 bits of base address */
    uint8_t base_middle;    /**< Middle 8 bits of base address */
    uint8_t access;         /**< Access flags (present, privilege, type) */
    uint8_t granularity;    /**< Granularity and upper 4 bits of limit */
    uint8_t base_high;      /**< Upper 8 bits of base address */
} __attribute__((packed));
/**
 * @brief GDT pointer structure
 * 
 * Loaded into the GDTR register via the ldgt instruction.
 * Points to the GDT and specifies its size.
 */
struct gdt_ptr {
    uint16_t limit;  /**< Size of GDT in bytes minus 1 */
    uint32_t base;   /**< Linear address of the GDT */
} __attribute__((packed));

/**
 * @brief Set a GDT entry
 * 
 * Configures a single entry in the Global Descriptor Table with the
 * specified base address, limit, and access flags.
 * 
 * @param num GDT entry index (0-based)
 * @param base Segment base address (linear address)
 * @param limit Segment limit (size - 1)
 * @param access Access byte (present, DPL, type flags)
 * @param gran Granularity byte (4KB granularity, 32-bit, upper limit bits)
 */
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

/**
 * @brief Install and activate the GDT
 * 
 * Sets up the Global Descriptor Table with kernel and user segments,
 * then loads it into the CPU via the LGDT instruction. Also reloads
 * segment registers to use the new GDT.
 * 
 * Horizon GDT layout:
 * - Entry 0: Null descriptor (required by x86)
 * - Entry 1: Kernel code segment (ring 0)
 * - Entry 2: Kernel data segment (ring 0)
 * - Entry 3: User code segment (ring 3)
 * - Entry 4: User data segment (ring 3)
 * - Entry 5: TSS (Task State Segment)
 * 
 * @note Must be called early in kernel initialization
 */
void gdt_install(void);