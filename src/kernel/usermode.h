// src/kernel/usermode.h
#ifndef USERMODE_H
#define USERMODE_H

#include <stdint.h>

void jump_to_usermode(uint32_t user_stack);
void jump_to_elf(const char *path);

#endif