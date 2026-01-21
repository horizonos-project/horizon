#include "keyboard.h"
#include "kernel/isr.h"
#include "kernel/log.h"
#include "kernel/io.h"
#include "libk/kprint.h"
#include "drivers/video/vga.h"
#include "drivers/serial/serial.h"

#define KBD_DATA_PORT    0x60

// Scancode Set 1 mappings (US)
static const char scancode_to_ascii[128] = {
    0, 27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b','\t',
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s',
    'd','f','g','h','j','k','l',';','\'','`',0,'\\','z','x','c','v',
    'b','n','m',',','.','/',0,'*',0,' ',0,0,0,0,0,0,
    /* rest zero */ 0
};

static const char scancode_to_ascii_shift[128] = {
    0, 27,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,'A','S',
    'D','F','G','H','J','K','L',':','"','~',0,'|','Z','X','C','V',
    'B','N','M','<','>','?',0,'*',0,' ',0,0,0,0,0,0,
    /* rest zero */ 0
};

#define SC_LSHIFT   0x2A
#define SC_RSHIFT   0x36
#define SC_LCTRL    0x1D
#define SC_LALT     0x38
#define SC_CAPS     0x3A
#define SC_E0       0xE0

static int shift_pressed = 0;
static int ctrl_pressed  = 0;
static int alt_pressed   = 0;
static int caps_lock     = 0;
static int extended      = 0;

// **Tiny** ring buffer for typed characters
#define KBD_BUF_SIZE 128
static volatile char kbd_buf[KBD_BUF_SIZE];
static volatile uint32_t kbd_r = 0;
static volatile uint32_t kbd_w = 0;

static void kbd_push(char c) {
    uint32_t next = (kbd_w + 1) % KBD_BUF_SIZE;

    if (next == kbd_r) {
        // Buffer full, drop character
        return;
    }

    kbd_buf[kbd_w] = c;
    kbd_w = next;
}

int keyboard_getchar(void) {
    if (kbd_r == kbd_w) return -1;
    char c = kbd_buf[kbd_r];
    kbd_r = (kbd_r + 1) % KBD_BUF_SIZE;
    return (unsigned char)c;
}

static inline int is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static char translate_scancode(uint8_t sc) {
    if (sc >= 128) return 0;

    char base = shift_pressed ? scancode_to_ascii_shift[sc]
                              : scancode_to_ascii[sc];

    if (!base) return 0;

    // Caps lock affects letters only; Shift XOR Caps determines case
    if (caps_lock && is_letter(base)) {
        // If shift is NOT pressed, uppercase; if shift IS pressed, lowercase.
        if (!shift_pressed) {
            if (base >= 'a' && base <= 'z') base = (char)(base - 'a' + 'A');
        } else {
            if (base >= 'A' && base <= 'Z') base = (char)(base - 'A' + 'a');
        }
    }

    return base;
}

static void keyboard_irq(regs_t *r) {
    (void)r;

    uint8_t sc = inb(KBD_DATA_PORT);

    if (sc == SC_E0) {
        extended = 1;
        return;
    }
    if (extended) {
        extended = 0;
        return;
    }

    // Break code = key release
    if (sc & 0x80) {
        uint8_t make = sc & 0x7F;
        if (make == SC_LSHIFT || make == SC_RSHIFT) shift_pressed = 0;
        else if (make == SC_LCTRL) ctrl_pressed = 0;
        else if (make == SC_LALT)  alt_pressed  = 0;
        return;
    }

    // Make code = key press
    if (sc == SC_LSHIFT || sc == SC_RSHIFT) { shift_pressed = 1; return; }
    if (sc == SC_LCTRL)  { ctrl_pressed  = 1; return; }
    if (sc == SC_LALT)   { alt_pressed   = 1; return; }
    if (sc == SC_CAPS)   { caps_lock ^= 1;   return; }

    char c = translate_scancode(sc);
    if (!c) return;

    kbd_push(c);

    kputc(c);
}

void keyboard_init(void) {
    irq_register_handler(1, keyboard_irq);
    klogf("[kbd] Keyboard driver initialized (IRQ1)\n");
}