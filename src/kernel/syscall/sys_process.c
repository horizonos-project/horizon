// src/kernel/syscall/sys_process.c
#include <stdint.h>
#include <stddef.h>

#include "kernel/log.h"
#include "kernel/errno.h"

#include "../../drivers/video/vga.h"
#include "../../drivers/serial/serial.h"
#include "../../drivers/vfs/vfs.h"
#include "../../drivers/keyboard/keyboard.h"

#include "mm/mm.h"
#include "sys_process.h"

// ----------------------------------------------------------------------------
// SYS_EXIT (1)
// ----------------------------------------------------------------------------
int32_t sys_exit(uint32_t status, uint32_t u2, uint32_t u3,
                 uint32_t u4, uint32_t u5, uint32_t u6) {
    (void)u2; (void)u3; (void)u4; (void)u5; (void)u6;

    klogf("[proc] Process exited with code %u\n", status);

    // TODO (when you have real processes):
    // - close fds
    // - free address space
    // - free kernel resources
    // - schedule next runnable task

    kprintf_both("Process exited with code %u\n", status);
    kprintf_both("System halted (no scheduler yet)\n");

    for (;;) {
        __asm__ volatile("hlt");
    }

    // Unreachable
    return 0;
}

// ----------------------------------------------------------------------------
// SYS_GETPID (20)
// ----------------------------------------------------------------------------
int32_t sys_getpid(uint32_t u1, uint32_t u2, uint32_t u3,
                   uint32_t u4, uint32_t u5, uint32_t u6) {
    (void)u1; (void)u2; (void)u3; (void)u4; (void)u5; (void)u6;

    // Single "process" environment for now.
    return 1;
}

// ----------------------------------------------------------------------------
// SYS_WRITE (4)
// ----------------------------------------------------------------------------
int32_t sys_write(uint32_t fd, uint32_t buf, uint32_t count,
                  uint32_t u4, uint32_t u5, uint32_t u6) {
    (void)u4; (void)u5; (void)u6;

    if (buf == 0) {
        klogf("[syscall] write: NULL buffer\n");
        return SYSCALL_ERR(EFAULT);
    }

    const char *str = (const char *)buf;

    // stdout/stderr -> VGA + serial
    if (fd == 1 || fd == 2) {
        for (uint32_t i = 0; i < count; i++) {
            vga_putc(str[i]);
            serial_putc(str[i]);
        }
        return (int32_t)count;
    }

    // TODO: once you have writable FS / vfs_write
    // int n = vfs_write((int)fd, str, (size_t)count);
    // if (n < 0) return SYSCALL_ERR(EBADF or EIO...);

    klogf("[syscall] write: fd %u not supported (no vfs_write yet)\n", fd);
    return SYSCALL_ERR(EBADF);
}

// ----------------------------------------------------------------------------
// SYS_READ (3)
// ----------------------------------------------------------------------------
int32_t sys_read(uint32_t fd, uint32_t buf, uint32_t count,
                 uint32_t u4, uint32_t u5, uint32_t u6) {
    (void)u4; (void)u5; (void)u6;

    if (buf == 0) {
        klogf("[syscall] read: NULL buffer\n");
        return SYSCALL_ERR(EFAULT);
    }

    if (count == 0) {
        return 0;
    }

    char *out = (char *)buf;

    // stdin -> keyboard stream
    if (fd == 0) {
        uint32_t i = 0;

        // Block until at least one byte
        for (;;) {
            int ch = keyboard_getchar();
            if (ch >= 0) {
                out[i++] = (char)ch;
                break;
            }
            __asm__ volatile("hlt");
        }

        // Drain any additional available bytes (non-blocking)
        while (i < count) {
            int ch = keyboard_getchar();
            if (ch < 0) break;
            out[i++] = (char)ch;
        }

        return (int32_t)i;
    }

    // Disallow reading from stdout/stderr
    if (fd == 1 || fd == 2) {
        return SYSCALL_ERR(EBADF);
    }

    int n = vfs_read((int)fd, out, (size_t)count);
    if (n < 0) {
        klogf("[syscall] read: vfs_read failed for fd %u\n", fd);
        // If your VFS returns negative errno already, you can forward it.
        // Otherwise use a generic errno for now:
        return SYSCALL_ERR(EIO);
    }

    return (int32_t)n;
}

// ----------------------------------------------------------------------------
// SYS_OPEN (5)
// ----------------------------------------------------------------------------
int32_t sys_open(uint32_t pathname, uint32_t flags, uint32_t mode,
                 uint32_t u4, uint32_t u5, uint32_t u6) {
    (void)mode; (void)u4; (void)u5; (void)u6;

    if (pathname == 0) {
        klogf("[syscall] open: NULL pathname\n");
        return SYSCALL_ERR(EFAULT);
    }

    const char *path = (const char *)pathname;

    int fd = vfs_open((char *)path, (int)flags);
    if (fd < 0) {
        klogf("[syscall] open: failed '%s'\n", path);
        // If vfs_open returns -errno, forward it:
        // return fd;
        return SYSCALL_ERR(ENOENT);
    }

    return (int32_t)fd;
}

// ----------------------------------------------------------------------------
// SYS_CLOSE (6)
// ----------------------------------------------------------------------------
int32_t sys_close(uint32_t fd, uint32_t u2, uint32_t u3,
                  uint32_t u4, uint32_t u5, uint32_t u6) {
    (void)u2; (void)u3; (void)u4; (void)u5; (void)u6;

    // Temporary policy: don't allow closing stdio
    if (fd <= 2) {
        return SYSCALL_ERR(EBADF);
    }

    int ret = vfs_close((int)fd);
    if (ret < 0) {
        // If vfs_close returns -errno, forward it:
        // return ret;
        return SYSCALL_ERR(EBADF);
    }

    return 0;
}

// ----------------------------------------------------------------------------
// SYS_FORK (2) - stub
// ----------------------------------------------------------------------------
int32_t sys_fork(uint32_t u1, uint32_t u2, uint32_t u3,
                 uint32_t u4, uint32_t u5, uint32_t u6) {
    (void)u1; (void)u2; (void)u3; (void)u4; (void)u5; (void)u6;

    klogf("[syscall] fork: stub (need scheduler/process model)\n");
    return SYSCALL_ERR(EAGAIN);
}

// ----------------------------------------------------------------------------
// SYS_EXECVE (11) - stub
// ----------------------------------------------------------------------------
int32_t sys_execve(uint32_t filename, uint32_t argv, uint32_t envp,
                   uint32_t u4, uint32_t u5, uint32_t u6) {
    (void)u4; (void)u5; (void)u6;

    if (filename == 0) {
        klogf("[syscall] execve: NULL filename\n");
        return SYSCALL_ERR(EFAULT);
    }

    const char *path = (const char *)filename;
    char **args = (char **)argv;
    (void)envp; // avoid unused warning until you use it

    klogf("[syscall] execve: path='%s'\n", path);

    if (args != NULL) {
        klogf("[syscall] execve: argv:\n");
        for (int i = 0; args[i] != NULL; i++) {
            klogf("  [%d] = '%s'\n", i, args[i]);
        }
    }

    klogf("[syscall] execve: stub (need ELF loader + userspace)\n");
    return SYSCALL_ERR(ENOENT);
}

// ----------------------------------------------------------------------------
// SYS_BRK (45)
// ----------------------------------------------------------------------------
int32_t sys_brk(uint32_t addr, uint32_t u2, uint32_t u3,
                uint32_t u4, uint32_t u5, uint32_t u6) {
    (void)u2; (void)u3; (void)u4; (void)u5; (void)u6;

    static uint32_t heap_start = 0;
    static uint32_t current_brk = 0;

    if (current_brk == 0) {
        heap_start = 0x40000000;
        current_brk = heap_start;
        klogf("[brk] Initialized heap at 0x%x\n", heap_start);
    }

    // Query current brk
    if (addr == 0) {
        return (int32_t)current_brk;
    }

    // Linux-ish: reject below heap start by returning current brk.
    if (addr < heap_start) {
        klogf("[brk] Request 0x%x below heap start\n", addr);
        return (int32_t)current_brk;
    }

    uint32_t old_aligned = (current_brk + 0xFFF) & ~0xFFF;
    uint32_t new_aligned = (addr + 0xFFF) & ~0xFFF;

    if (new_aligned > old_aligned) {
        uint32_t num_pages = (new_aligned - old_aligned) / 0x1000;
        klogf("[brk] Growing heap by %u pages\n", num_pages);

        for (uint32_t i = 0; i < num_pages; i++) {
            uint32_t vaddr = old_aligned + (i * 0x1000);
            void *phys = pmm_alloc_frame();
            if (!phys) {
                klogf("[brk] Out of memory while growing heap\n");
                return (int32_t)current_brk;
            }

            vmm_map_page(vaddr, (uint32_t)phys, PAGE_PRESENT | PAGE_USER | PAGE_RW);
        }
    } else if (new_aligned < old_aligned) {
        uint32_t num_pages = (old_aligned - new_aligned) / 0x1000;
        klogf("[brk] Shrinking heap by %u pages\n", num_pages);

        for (uint32_t i = 0; i < num_pages; i++) {
            uint32_t vaddr = new_aligned + (i * 0x1000);
            uint32_t phys = vmm_get_physical(vaddr);
            if (phys) {
                pmm_mark_free(phys);
            }
            vmm_unmap_page(vaddr);
        }

        // Flush TLB
        __asm__ volatile(
            "mov %%cr3, %%eax;"
            "mov %%eax, %%cr3;"
            : : : "eax"
        );
    }

    current_brk = addr;
    return (int32_t)current_brk;
}

// ----------------------------------------------------------------------------
// SYS_ALARM (27) - stub
// ----------------------------------------------------------------------------
int32_t sys_alarm(uint32_t seconds, uint32_t u2, uint32_t u3,
                  uint32_t u4, uint32_t u5, uint32_t u6) {
    (void)u2; (void)u3; (void)u4; (void)u5; (void)u6;

    klogf("[syscall] alarm: stub (%u seconds)\n", seconds);
    return 0;
}

// ----------------------------------------------------------------------------
// Horizon syscall: CLEAR_VGA (500)
// ----------------------------------------------------------------------------
int32_t sys_clear_vga(uint32_t a, uint32_t b, uint32_t c,
                      uint32_t d, uint32_t e, uint32_t f) {
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f;
    vga_clear();
    return 0;
}
