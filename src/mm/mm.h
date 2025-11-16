#ifndef MM_H
#define MM_H

// Unified mm header interface

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Common definitions
#define PAGE_SIZE 4096
#define FRAME_SIZE 4096

// Page/frame flags
#define PAGE_PRESENT  0x001
#define PAGE_RW       0x002
#define PAGE_USER     0x004

// Include all memory subsystems
#include "pmm.h"
#include "vmm.h"
#include "heap.h"

#endif // MM_H