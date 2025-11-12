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

    uint8_t val = inb(port);
    io_wait();
    
    val |= (1 << irq_line);
    outb(port, val);
    io_wait();
}

void pic_clear_mask(uint8_t irq_line)
{
    uint16_t port;
    uint8_t actual_irq = irq_line;
    
    if (irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }

    klogf("[pic] Unmasking IRQ%u (port 0x%x)\n", actual_irq, port);
    
    uint8_t current = inb(port);
    io_wait(); 
    
    klogf("[pic] Current mask: 0x%02x\n", current);
    
    uint8_t val = current & ~(1 << irq_line);
    outb(port, val);
    io_wait();
    
    uint8_t verify = inb(port);
    io_wait();
    
    klogf("[pic] New mask: 0x%02x\n", verify);
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
    io_wait();

    outb(0x40, (uint8_t)(divisor & 0xFF));
    io_wait();

    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
    io_wait();

    kprintf("[pit] freq=%u Hz, divisor=%u\n", freq, divisor);
}

void pit_check(void) {
    klogf("[pit] Checking PIT status...\n");
    
    // Read current count (channel 0, latch command)
    outb(0x43, 0x00);
    io_wait();
    
    uint8_t lo = inb(0x40);
    io_wait();
    uint8_t hi = inb(0x40);
    io_wait();
    
    uint16_t count = (hi << 8) | lo;
    klogf("[pit] Current count: %u\n", count);
    
    // Wait a bit and check again
    for (volatile int i = 0; i < 1000000; i++);
    
    outb(0x43, 0x00);
    io_wait();
    lo = inb(0x40);
    io_wait();
    hi = inb(0x40);
    io_wait();
    
    uint16_t count2 = (hi << 8) | lo;
    klogf("[pit] Count after delay: %u\n", count2);
    
    if (count == count2) {
        klogf("[pit] WARNING: Count didn't change, PIT may not be running!\n");
    } else {
        klogf("[pit] PIT appears to be counting\n");
    }
}
