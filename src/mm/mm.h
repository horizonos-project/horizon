/**
 * @file mm.h
 * @brief Unified Memory Management Interface
 * 
 * Provides a single header to access all memory management subsystems
 * in HorizonOS. This includes physical memory management (PMM),
 * virtual memory management (VMM), and kernel heap allocation.
 * 
 * Import this header to get access to the complete memory management API.
 * 
 * @defgroup mm Memory Management
 * @{
 */

#ifndef MM_H
#define MM_H

// Unified mm header interface

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/** @brief Size of a virtual memory page in bytes (4KB) */
#define PAGE_SIZE 4096

/** @brief Size of a physical memory frame in bytes (4KB) */
#define FRAME_SIZE 4096

/** @brief Page is present in memory */
#define PAGE_PRESENT  0x001

/** @brief Page is read/write (otherwise read-only) */
#define PAGE_RW       0x002

/** @brief Page is accessible from user mode (ring 3) */
#define PAGE_USER     0x004

#include "pmm.h"
#include "vmm.h"
#include "heap.h"

/** @} */

#endif // MM_H