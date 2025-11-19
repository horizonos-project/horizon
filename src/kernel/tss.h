/**
 * @file tss.h
 * @brief Task State Segment (TSS) Management
 * 
 * The TSS is an x86 structure used for hardware task switching and, more
 * importantly for us, storing the kernel stack pointer when switching from
 * user mode (ring 3) to kernel mode (ring 0).
 * 
 * When a syscall or interrupt happens while in userspace, the CPU needs to
 * switch to a kernel stack. The TSS tells it where that stack is (esp0/ss0).
 * 
 * We don't use the TSS for hardware task switching (that's ancient and slow),
 * but we DO need it for the privilege level transitions!
 * 
 * Key fields we actually use:
 * - esp0: Kernel stack pointer (updated on context switch)
 * - ss0: Kernel stack segment (always 0x10)
 * - iomap_base: I/O permission bitmap (set to sizeof(tss_t) = disabled)
 * 
 * Everything else is mostly ignored by modern OSes but required by x86.
 */

#ifndef TSS_H
#define TSS_H

#include <stdint.h>

/**
 * @brief Task State Segment structure
 * 
 * Hardware-defined structure that x86 uses for task management and
 * privilege level transitions. Most fields are unused in modern software
 * task switching, but esp0/ss0 are critical for userspace support.
 * 
 * The structure must be packed to match the exact hardware layout expected
 * by the CPU.
 */
typedef struct {
    uint32_t prev_tss;   /**< Link to previous TSS (for hardware task switching - unused) */
    uint32_t esp0;       /**< Kernel stack pointer - SUPER IMPORTANT for ring 3 → ring 0! */
    uint32_t ss0;        /**< Kernel stack segment (always 0x10 for us) */
    uint32_t esp1;       /**< Ring 1 stack (unused) */
    uint32_t ss1;        /**< Ring 1 segment (unused) */
    uint32_t esp2;       /**< Ring 2 stack (unused) */
    uint32_t ss2;        /**< Ring 2 segment (unused) */
    uint32_t cr3;        /**< Page directory physical address */
    uint32_t eip;        /**< Instruction pointer */
    uint32_t eflags;     /**< CPU flags register */
    uint32_t eax, ecx, edx, ebx;  /**< General purpose registers */
    uint32_t esp, ebp, esi, edi;  /**< Stack and index registers */
    uint32_t es, cs, ss, ds, fs, gs;  /**< Segment selectors */
    uint32_t ldt;        /**< Local Descriptor Table selector (unused) */
    uint16_t trap;       /**< Debug trap flag */
    uint16_t iomap_base; /**< I/O permission bitmap offset (set to sizeof(tss) = disabled) */
} __attribute__((packed)) tss_t;

/**
 * @brief Install the TSS into the GDT
 * 
 * Creates a TSS structure, initializes it with the kernel stack, and
 * installs it as a GDT entry. Also loads the TSS into the CPU using
 * the LTR (Load Task Register) instruction.
 * 
 * Must be called after GDT initialization and before jumping to userspace.
 * 
 * @param kernel_stack Physical address of the kernel stack top
 * 
 * Example:
 * @code
 * uint32_t kstack = (uint32_t)kalloc(0x4000) + 0x4000;  // 16KB stack
 * tss_install(kstack);
 * @endcode
 * 
 * @note Only needs to be called once during boot
 */
void tss_install(uint32_t kernel_stack);

/**
 * @brief Update the kernel stack pointer in the TSS
 * 
 * When switching between processes/tasks, each needs its own kernel stack.
 * This function updates esp0 in the TSS so that the next ring 3 → ring 0
 * transition uses the correct kernel stack.
 * 
 * @param stack Physical address of the new kernel stack top
 * 
 * Example (in hypothetical scheduler):
 * @code
 * void switch_task(task_t *next) {
 *     tss_set_kernel_stack(next->kernel_stack_top);
 *     // ... switch page directory, restore registers, etc.
 * }
 * @endcode
 * 
 * @note Critical for process management - wrong stack = instant crash!
 */
void tss_set_kernel_stack(uint32_t stack);

#endif // TSS_H