#include "../libk/kprint.h"

__attribute__((noreturn)) void panicf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kvprintf(fmt, args);
    va_end(args);
}
