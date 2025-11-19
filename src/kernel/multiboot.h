/**
 * @file multiboot.h
 * @brief Multiboot Specification Structures
 * 
 * Defines structures for the Multiboot information passed by GRUB and other
 * compliant bootloaders. The bootloader provides critical system information
 * like memory maps, loaded modules, and boot parameters.
 * 
 * The Multiboot specification allows kernels to be loaded by any compliant
 * bootloader, making the kernel more portable and easier to test.
 * 
 * @see https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
 * @see mboot_mmap.h for memory map entry structures
 */

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

/**
 * @brief Multiboot magic number
 * 
 * Value passed in EAX by the bootloader to indicate successful Multiboot
 * protocol handoff. The kernel should verify this value at entry.
 */
#define MULTIBOOT_MAGIC        0x2BADB002

/** @brief Memory info (mem_lower, mem_upper) is valid */
#define MB_INFO_MEM            (1 << 0)

/** @brief Boot device info is valid */
#define MB_INFO_BOOT_DEVICE    (1 << 1)

/** @brief Kernel command line is present */
#define MB_INFO_CMDLINE        (1 << 2)

/** @brief Module list is present */
#define MB_INFO_MODS           (1 << 3)

/** @brief Memory map is present */
#define MB_INFO_MMAP           (1 << 6)

/**
 * @brief Multiboot information structure
 * 
 * Passed by the bootloader (in EBX register) to provide system information.
 * The flags field indicates which fields contain valid data.
 * 
 * Always verify flags before accessing optional fields to avoid reading
 * garbage data from unsupported bootloaders.
 */
typedef struct multiboot_info {
    uint32_t flags;         /**< Bitfield indicating which fields are valid */

    /* Memory Information (valid if MB_INFO_MEM flag set) */
    uint32_t mem_lower;     /**< Amount of lower memory in KB (0-640 KB) */
    uint32_t mem_upper;     /**< Amount of upper memory in KB (from 1 MB) */

    /* Boot Device (valid if MB_INFO_BOOT_DEVICE flag set) */
    uint32_t boot_device;   /**< BIOS boot device identifier */

    /* Command Line (valid if MB_INFO_CMDLINE flag set) */
    uint32_t cmdline;       /**< Physical address of kernel command line string */

    /* Modules (valid if MB_INFO_MODS flag set) */
    uint32_t mods_count;    /**< Number of modules loaded (e.g., initramfs) */
    uint32_t mods_addr;     /**< Physical address of module structures array */

    /* ELF Section Headers (valid if flags[5] set) */
    uint32_t syms[4];       /**< ELF symbol table info (unused for now) */

    /* Memory Map (valid if MB_INFO_MMAP flag set) */
    uint32_t mmap_length;   /**< Length of memory map in bytes */
    uint32_t mmap_addr;     /**< Physical address of memory map entries */
} __attribute__((packed)) multiboot_info_t;

/**
 * @brief Display Multiboot information
 * 
 * Logs all available information from the Multiboot structure to the
 * kernel console. Useful for debugging boot issues and verifying what
 * the bootloader provided.
 * 
 * Outputs:
 * - Total memory (lower + upper)
 * - Boot device
 * - Command line arguments
 * - Loaded modules (initramfs, etc.)
 * - Memory map regions
 * 
 * @param mb Pointer to Multiboot information structure from bootloader
 */
void display_mb_info(multiboot_info_t *mb);

#endif
