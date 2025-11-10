; Horizon bootloader
; (c) 2025- HorizonOS Project
;
; This is multiboot compliant for use with GRUB
; Everyones favourite bootloader thing!
;

BITS 32
SECTION .multiboot
align 4
    dd 0x1BADB002           ; magic
    dd 0x0                  ; flags
    dd -(0x1BADB002 + 0x00) ; checksum

SECTION .text
global _start
extern kmain

_start:
    cli                     ; disable interrupts
    mov esp, stack_top      ; temp stack
    push ebx                ; multiboot info struct ptr
    push eax                ; multiboot magic num
    call kmain              ; jmp into kernel main
.hang:                      ; Hang the CPU forever
    hlt
    jmp .hang

SECTION .bss
align 16
stack_bottom:
    resb 16384              ; ~16KiB stack
stack_top:
