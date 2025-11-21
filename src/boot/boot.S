// Horizon Bootloader (Now in AT&T Syntax!)
//
// (c) 2025 HorizonOS Project
// This is Multiboot compliant for use with GRUB
//

.code32

.section .multiboot
.align 4
    .long 0x1BADB002            // multiboot magic
    .long 0x0                   // flags
    .long -(0x1BADB002 + 0x00)  // checksum

.section .text
.global _start
.extern kmain

_start:
    cli                          // disable interrupts
    movl $stack_top, %esp        // set up stack
    pushl %ebx                   // multiboot info struct ptr
    pushl %eax                   // multiboot magic num
    call kmain                   // jump into kernel main
.hang:
    hlt
    jmp .hang

.section .bss
.align 16
stack_bottom:
    .skip 16384                  // ~16KiB stack
stack_top:
