;
; userland/hello.asm --> flat 32bit userland hello world
;

BITS 32
GLOBAL _start

SECTION .text
_start:
    mov eax, 4              ; SYS_WRITE
    mov ebx, 1              ; fd is stdout
    mov ecx, msg            ; buffer
    mov edx, msg_end - msg  ; length
    int 0x80

    mov eax, 1              ; SYS_EXIT
    mov ebx, 42             ; exit code
    int 0x80

SECTION .rodata
msg: db "Hello from ring 3 via int 0x80!", 10
msg_end: