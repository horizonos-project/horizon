// PS/2 Keyboard Scancode Set 1 - US Layout
//
// Scancodes 0x00-0x7F are key press (make codes)
//
// Scancodes with bit 7 set (0x80+) are key release (break codes)

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
int keyboard_getchar(void); // -1 if none

#endif // KEYBOARD_H