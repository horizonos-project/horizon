#include <stdint.h>

#include "io.h"
#include "pic.h"

// PIC i/o ports
#define PIC1_CMD   0x20
#define PIC1_DATA  0x21
#define PIC2_CMD   0xA0
#define PIC2_DATA  0xA1

#define ICW1_INIT  0x10
#define ICW1_ICW4  0x01
#define ICW4_8086  0x01

void pic_remap(void)
{
    // Save masks
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    // Start the init seq
    outb(PIC1_DATA, 0x20); // master -> IRQs 0..7 map to ints 0x20..0x27
    outb(PIC2_DATA, 0x28); // slave  -> IRQs 8..15 map to ints 0x28..0x2F

    // ICW3: wiring things
    outb(PIC1_DATA, 0x04); // master: slave on IRQ2 (bitmask 00000100)
    outb(PIC2_DATA, 0x02); // slave: cascade identity is 2

    // 8086/88 outb mode
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // Restore masks
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}