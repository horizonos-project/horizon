#include <stdint.h>
#include "pic.h"
#include "io.h"
#include "../libk/kprint.h"
#include "kernel/log.h"

#define PIC1_CMD   0x20
#define PIC1_DATA  0x21
#define PIC2_CMD   0xA0
#define PIC2_DATA  0xA1

#define ICW1_INIT  0x10
#define ICW1_ICW4  0x01
#define ICW4_8086  0x01

static inline void io_wait(void) {
    outb(0x80, 0);
}

void pic_remap(int offset1, int offset2)
{
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();

    outb(PIC1_DATA, 0x04); // tell master about slave at IRQ2
    io_wait();
    outb(PIC2_DATA, 0x02); // tell slave its cascade ID
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);

    kprintf("[pic] Remapped: master=%d, slave=%d\n", offset1, offset2);
}

void pic_set_mask(uint8_t irq_line)
{
    uint16_t port;
    if (irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }

    uint8_t val = inb(port) | (1 << irq_line);
    outb(port, val);
}

void pic_clear_mask(uint8_t irq_line)
{
    uint16_t port;
    if (irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }

    uint8_t val = inb(port) & ~(1 << irq_line);
    outb(port, val);
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)
        outb(PIC2_CMD, 0x20);
    outb(PIC1_CMD, 0x20);
}

void pit_init(uint32_t freq) {
    if (!freq)
        klogf("pit_init: frequency == 0");

    uint32_t divisor = 1193180 / freq;
    if (!divisor)
        klogf("pit_init: divisor == 0 (freq=%u)", freq);

    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));

    kprintf("[pit] freq=%u Hz, divisor=%u\n", freq, divisor);
}
