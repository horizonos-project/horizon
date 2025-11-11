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

    kprintf("Process exited with code %u\n", status);
    kprintf("System has been halted. (We don't have a scheduler)\n");
    while (1) { __asm__ volatile("hlt"); }

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