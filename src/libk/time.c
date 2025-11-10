#include <stdint.h>

// Busyloop since PIT does not exist yet
void sleep(volatile uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 10000; i++) {
        __asm__ volatile("outb %%al, $0x80" ::: "al");
    }
}
