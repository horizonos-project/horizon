#pragma once

#include <stdint.h>
#include <stdbool.h>

void keyboard_init(void);

// returns false when buffer is empty
bool kbd_getchar(char *out);   
