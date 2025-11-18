#include <stddef.h>
#include "kernel/log.h"
#include "syscall.h"
#include "../../libk/kprint.h"
#include "../../drivers/video/vga.h"
#include "../../drivers/serial/serial.h"
#include "mm/mm.h"

uint32_t sys_exit(uint32_t status, uint32_t u2, uint32_t u3, uint32_t u4, uint32_t u5) {
    // Immediately dumping all of this for just the status
    (void)u2; (void)u3; (void)u4; (void)u5;

    klogf("[proc] Process exited with code %u\n", status);

    // TODO for when processes are real:
    // - clean up proc resources
    // - free mem
    // - close fd(s)
    // - switch to next proc

    kprintf_both("Process exited with code %u\n", status);
    kprintf_both("System has been halted. (We don't have a scheduler)\n");
    while (1) { 
        __asm__ volatile("hlt");
    }

    return 0; // We will never get here
}

uint32_t sys_getpid(uint32_t u1, uint32_t u2, uint32_t u3, uint32_t u4, uint32_t u5) {
    (void)u1; (void)u2; (void)u3; (void)u4; (void)u5;
    
    // We only have one "process" (the kernel)
    // Always return PID 1
    return 1;
}

uint32_t sys_write(uint32_t fd, uint32_t buf, uint32_t count, uint32_t u4, uint32_t u5) {
    (void)u4; (void)u5;
    
    // Validate pointer (basic check)
    if (buf == 0) {
        klogf("[syscall] sys_write: NULL buffer\n");
        return -1;  // EFAULT
    }
    
    char *str = (char*)buf;
    
    // Handle stdout (fd 1) and stderr (fd 2)
    if (fd == 1 || fd == 2) {
        for (uint32_t i = 0; i < count; i++) {
            vga_putc(str[i]);
            serial_putc(str[i]);
        }
        return count;  // Success: wrote all bytes
    }
    
    // TODO: Handle file writes via VFS
    
    klogf("[syscall] sys_write: invalid fd %u\n", fd);
    return -1;  // EBADF (bad file descriptor)
}

uint32_t sys_read(uint32_t fd, uint32_t buf, uint32_t count, uint32_t u4, uint32_t u5) {
    (void)u4; (void)u5;
    
    // Validate pointer
    if (buf == 0) {
        klogf("[syscall] sys_read: NULL buffer\n");
        return -1;  // EFAULT
    }
    
    char *buffer = (char*)buf;
    
    // Handle stdin (fd 0)
    if (fd == 0) {
        // TODO: When you have keyboard input working:
        // Read from keyboard buffer
        klogf("[syscall] sys_read: stdin not yet implemented\n");
        return 0;  // EOF for now
    }
    
    // TODO: Handle file reads via VFS
    
    klogf("[syscall] sys_read: invalid fd %u\n", fd);
    return -1;  // EBADF
}

uint32_t sys_open(uint32_t pathname, uint32_t flags, uint32_t mode, uint32_t u4, uint32_t u5) {
    (void)u4; (void)u5;
    
    if (pathname == 0) {
        klogf("[syscall] sys_open: NULL pathname\n");
        return -1;  // EFAULT
    }
    
    char *path = (char*)pathname;
    
    klogf("[syscall] sys_open: path='%s', flags=0x%x, mode=0x%x\n", path, flags, mode);
    
    // TODO: Implement via VFS when ready
    // For now, just log and fail
    
    klogf("[syscall] sys_open: VFS not yet implemented\n");
    return -1;  // ENOENT (no such file)
}

uint32_t sys_close(uint32_t fd, uint32_t u2, uint32_t u3, uint32_t u4, uint32_t u5) {
    (void)u2; (void)u3; (void)u4; (void)u5;
    
    // Don't allow closing std streams
    // This is comically bad
    if (fd <= 2) {
        klogf("[syscall] sys_close: cannot close std stream %u\n", fd);
        return -1;  // EBADF
    }
    
    klogf("[syscall] sys_close: fd=%u\n", fd);
    
    // TODO: Implement via VFS and process fd table
    
    klogf("[syscall] sys_close: VFS not yet implemented\n");
    return -1;  // EBADF
}

uint32_t sys_fork(uint32_t u1, uint32_t u2, uint32_t u3, uint32_t u4, uint32_t u5) {
    (void)u1; (void)u2; (void)u3; (void)u4; (void)u5;
    
    klogf("[syscall] sys_fork: not yet implemented (need scheduler)\n");
    
    // TODO: When processes:
    // 1. Allocate new PCB
    // 2. Copy parent's address space (CoW if fancy)
    // 3. Copy parent's fd table
    // 4. Set up new stack
    // 5. Return 0 to child, child PID to parent
    
    return -1;  // EAGAIN (resource temporarily unavailable)
}

uint32_t sys_execve(uint32_t filename, uint32_t argv, uint32_t envp, uint32_t u4, uint32_t u5) {
    (void)u4; (void)u5;
    
    if (filename == 0) {
        klogf("[syscall] sys_execve: NULL filename\n");
        return -1;  // EFAULT
    }
    
    char *path = (char*)filename;
    char **args = (char**)argv;
    char **env = (char**)envp;
    
    klogf("[syscall] sys_execve: path='%s'\n", path);
    
    // Log argv if present
    if (args != NULL) {
        klogf("[syscall] sys_execve: argv:\n");
        for (int i = 0; args[i] != NULL; i++) {
            klogf("  [%d] = '%s'\n", i, args[i]);
        }
    }
    
    // TODO: When ELF loader and VFS:
    // 1. Load executable from VFS
    // 2. Parse ELF header
    // 3. Map sections into memory
    // 4. Set up new stack with argv/envp
    // 5. Jump to entry point
    // NOTE: execve doesn't return on success!
    
    klogf("[syscall] sys_execve: not yet implemented\n");
    return -1;  // ENOENT
}

uint32_t sys_brk(uint32_t addr, uint32_t u2, uint32_t u3, uint32_t u4, uint32_t u5) {
    static uint32_t heap_start = 0;
    static uint32_t current_brk = 0;

    // Init on first call
    if (current_brk == 0) {
        heap_start = 0x40000000;
        current_brk = heap_start;
        klogf("[brk] Initalized heap at 0x%x", heap_start);
    }

    // Query current brk
    if (addr == 0) {
        return current_brk;
    }

    // Validate the request
    if (addr < heap_start) {
        klogf("[brk] Request 0x%x is below heap start!\n", addr);
        return current_brk;
    }

    // TODO: Check against stack once we have per-process stacks

    uint32_t old_brk_aligned = (current_brk + 0xFFF) & ~0xFFF;
    uint32_t new_brk_aligned = (addr + 0xFFF) & ~0xFFF;

    if (new_brk_aligned > old_brk_aligned) {
        // Growing heap - allocate pages
        uint32_t num_pages = (new_brk_aligned - old_brk_aligned) / 0x1000;
        
        klogf("[brk] Growing heap by %u pages\n", num_pages);
        
        for (uint32_t i = 0; i < num_pages; i++) {
            uint32_t vaddr = old_brk_aligned + (i * 0x1000);
            void *phys_page = pmm_alloc_frame();
            
            if (!phys_page) {
                klogf("[brk] Failed to allocate page!\n");
                return current_brk;
            }
            
            // Map page into current address space
            // Flags: Present, User, Read/Write
            vmm_map_page(vaddr, (uint32_t)phys_page, 
                     PAGE_PRESENT | PAGE_USER | PAGE_RW);
        }
    }
    else if (new_brk_aligned < old_brk_aligned) {
        // Oh hey we can shrink the heap!
        uint32_t num_pages = (old_brk_aligned - new_brk_aligned) / 0x1000;

        for (uint32_t i = 0; i < num_pages; i++) {
            uint32_t vaddr = new_brk_aligned + (i * 0x1000);
            
            // Get physical address and free it
            uint32_t phys_addr = vmm_get_physical(vaddr);
            if (phys_addr) {
                pmm_mark_free((void*)phys_addr);
            }
            
            // Unmap the page
            vmm_unmap_page(vaddr);
        }

        __asm__ volatile(
            "mov %%cr3, %%eax"
            "mov %%eax, %%cr3"
            : : : "eax"
        );
    }

    current_brk = addr;
    return current_brk;
}

uint32_t sys_alarm(uint32_t seconds, uint32_t u2, uint32_t u3, uint32_t u4, uint32_t u5) {
    (void)u2; (void)u3; (void)u4; (void)u5;
    
    klogf("[syscall] sys_alarm: %u seconds\n", seconds);
    
    // TODO:
    // - Timer interrupts
    // - Signal handling
    // - Process management
    // Then implement this properly
    
    // For now, just stub it out
    // alarm(0) disables any pending alarm
    // Returns number of seconds remaining on previous alarm
    
    return 0;  // No previous alarm
}