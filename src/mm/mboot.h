/**
 * @file mboot.h
 * @brief Multiboot Memory Map Structures
 * 
 * Defines structures for parsing the memory map provided by the bootloader
 * via the Multiboot specification. The memory map describes available and
 * reserved physical memory regions.
 * 
 * @see https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
 */
#ifndef MBOOT_MMAP_H
#define MBOOT_MMAP_H

#include <stdint.h>

/**
 * @brief Multiboot memory map entry
 * 
 * Describes a single region of physical memory. The bootloader provides
 * an array of these entries to inform the kernel about available RAM,
 * reserved regions, ACPI data, etc.
 * 
 * Memory types:
 * - Type 1: Available RAM (usable)
 * - Type 2: Reserved (unusable)
 * - Type 3: ACPI reclaimable
 * - Type 4: ACPI NVS
 * - Type 5: Bad memory
 */
typedef struct {
    uint32_t size;
    uint64_t addr;
    uint64_t length;
    uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

#endif