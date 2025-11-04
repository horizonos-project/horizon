#include <stdint.h>
#include <stdbool.h>
#include "io.h"
#include "pic.h"     // pic_eoi()
#include "idt.h"     // idt_set_gate() if you need it here
#include "tty.h"


#define KBD_DATA 0x60
#define KBD_STAT 0x64
#define KBD_CMD  0x64
#define IRQ1_VEC 0x21

// Ring buffer, is easy!
#define KBD_BUF_SZ 256
static volatile char buf[KBD_BUF_SZ];
static volatile uint8_t head=0, tail=0;

static inline void push(char c)
{
    uint8_t n = (uint8_t)((head+1) % KBD_BUF_SZ);
    if (n != tail)
    {
        buf[head] = c; 
        head = n;
    } // drop char on overflow
}

bool kbd_getchar(char *out)
{
    if (head == tail) 
        return false;

    *out = buf[tail];
    tail = (uint8_t)((tail+1) % KBD_BUF_SZ);

    return true;
}

// Modifiers
static bool shift_l=false, shift_r=false, ctrl=false, alt=false;
static bool e0 = false;

// US-EN keyboard keymap (a printable subset of)
static const char keymap[128] = {
/*0x00*/ 0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
/*0x0F*/ '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
/*0x1E*/ 'a','s','d','f','g','h','j','k','l',';','\'','`', 0, '\\',
/*0x2C*/ 'z','x','c','v','b','n','m',',','.','/', 0,    '*', 0, ' ', 0, 0,
/*0x3C*/ 0,0,0,0,0,0,0,0,0,0,0,   '-', 0,0,0,  '+',
/*0x4C*/ 0,0,0,0,0,0,0,0,0,0, 0,  0,   0,0,0,  0,
/*0x5C*/ 0,0,0,0,0,0,0,0,0,0, 0,  0,   0,0,0,  0,
/*0x6C*/ 0,0,0,0,0,0,0,0,0,0, 0,  0,   0,0,0,  0
};

// Shifted variants
static const char keymap_shift[128] = {
/*0x00*/ 0,  27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
/*0x0F*/ '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
/*0x1E*/ 'A','S','D','F','G','H','J','K','L',':','"','~', 0,  '|',
/*0x2C*/ 'Z','X','C','V','B','N','M','<','>','?', 0,    '*', 0, ' ', 0, 0,
/*...*/ 0
};

static inline uint8_t in_status(void)
{
    return inb(KBD_STAT);
}

static inline uint8_t in_data(void)
{ 
    return inb(KBD_DATA);
}

static inline void out_cmd(uint8_t v)
{ 
    outb(KBD_CMD, v); 
}

// ------------ untested partly AI non-compiling code, FIX ASAP. --------------

static void kbd_isr_impl(void){
    // Read scancode if available
    if ((in_status() & 1) == 0) goto done; // no data
    uint8_t sc = in_data();

    if (sc == 0xE0){ e0 = true; goto done; }

    bool released = sc & 0x80;
    uint8_t code = sc & 0x7F;

    // Modifiers (left/right shift, ctrl, alt; ignoring e0 detail for now)
    if (!e0){
        if (code==0x2A) { shift_l = !released; goto done; } // LShift
        if (code==0x36) { shift_r = !released; goto done; } // RShift
        if (code==0x1D) { ctrl    = !released; goto done; }
        if (code==0x38) { alt     = !released; goto done; }
    }
    e0 = false;

    if (!released){
        char ch = (shift_l||shift_r) ? keymap_shift[code] : keymap[code];
        if (ctrl){
            // crude Ctrl mapping for A–Z
            if (ch>='A' && ch<='Z') ch = ch - 'A' + 1;
            else if (ch>='a' && ch<='z') ch = ch - 'a' + 1;
        }
        if (ch) push(ch);
    }

done:
    pic_eoi(1); // EOI for IRQ1
}

// ISR trampoline (ASM side calls this)
void keyboard_isr(void){ kbd_isr_impl(); }

static void kbd_enable(void){
    // Unmask IRQ1 on PIC
    pic_clear_mask(1);

    // Enable scanning (controller cmds are finicky; this is often enough in QEMU)
    // 0xF4 to device via 0x60 after writing 0xD4 to 0x64 selects second port on some ctrls.
    // For classic single PS/2, just ensure clock enabled:
    // out_cmd(0xAE); // enable keyboard (optional)
}

void keyboard_init(void){
    // Hook ISR: IDT[0x21] = keyboard_isr stub (you likely have a macro for this)
    idt_set_irq_handler(1 /*IRQ1*/, keyboard_isr);
    kbd_enable();
}