#include <stddef.h>
#include "kernel/log.h"
#include "syscall.h"
#include "../../libk/kprint.h"
#include "../../drivers/video/vga.h"
#include "../../drivers/serial/serial.h"

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
    (void)u2; (void)u3; (void)u4; (void)u5;
    
    static uint32_t current_brk = 0;
    
    // Initialize brk on first call
    if (current_brk == 0) {
        // Set initial program break (I think this is right)
        current_brk = 0x40000000;
        klogf("[syscall] sys_brk: initialized to 0x%x\n", current_brk);
    }
    
    // If addr is 0, just return current brk
    if (addr == 0) {
        return current_brk;
    }
    
    // TODO: Validate the new address:
    // - Must be above initial brk
    // - Must not overlap with stack
    // - Must be page-aligned (or align it)
    
    klogf("[syscall] sys_brk: requested 0x%x, current 0x%x\n", addr, current_brk);
    
    // For now, just accept any reasonable request
    if (addr > current_brk) {
        // Growing heap - would need to allocate pages
        klogf("[syscall] sys_brk: growing heap (TODO: allocate pages)\n");
    } else if (addr < current_brk) {
        // Shrinking heap - would need to free pages
        klogf("[syscall] sys_brk: shrinking heap (TODO: free pages)\n");
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