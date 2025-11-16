#include "../libk/kprint.h"
#include "drivers/video/vga.h"
#include "kernel/log.h"
#include "panic.h"

__attribute__((noreturn)) void panicf(const char* fmt, ...) {
    __asm__ volatile("cli");

    kclear();

    kprintf_both("\n\n====================\n");
    kprintf_both("Horizon Kernel Panic!\n");
    kprintf_both("====================\n");

    va_list args;
    va_start(args, fmt);
    kvprintf(fmt, args);
    va_end(args);

    for (;;) {
        __asm__ volatile("hlt");
    }
}
