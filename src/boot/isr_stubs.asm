[BITS 32]
SECTION .text

; Exporting these so idt.c can ref them
%macro ISR_NOERR 1
global isr%1
isr%1:
    push dword 0            ; fake error code
    push dword %1           ; int number
    jmp isr_common
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    push dword %1         ; interrupt number
    jmp isr_common
%endmacro

%macro IRQ_STUB 2
    global irq%1
irq%1:
    push dword 0          ; dummy error code
    push dword %2         ; interrupt vector number (0x20+N)
    jmp irq_common
%endmacro

; CPU exceptions 0..31
; some exceptions push an error code automatically, some don't
; Treat them all uniformly: handler(int_no, err_code)

; 0: Divide Error (no err code)
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
; 8: Double Fault (err code pushed by CPU)
global isr8
isr8:
    push dword 8          ; int_no
    jmp isr_common_err

ISR_NOERR 9
ISR_ERR   10              ; invalid TSS
ISR_ERR   11              ; segment not present
ISR_ERR   12              ; stack fault
ISR_ERR   13              ; general protection
ISR_ERR   14              ; page fault
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

; IRQs after the PIC remap (32->47)
IRQ_STUB 0, 32
IRQ_STUB 1, 33
IRQ_STUB 2, 34
IRQ_STUB 3, 35
IRQ_STUB 4, 36
IRQ_STUB 5, 37
IRQ_STUB 6, 38
IRQ_STUB 7, 39
IRQ_STUB 8, 40
IRQ_STUB 9, 41
IRQ_STUB 10,42
IRQ_STUB 11,43
IRQ_STUB 12,44
IRQ_STUB 13,45
IRQ_STUB 14,46
IRQ_STUB 15,47

; Common ISR path (CPU exceptions)
; stack right now (top): [int_no] [err_code or fake] ...
extern isr_handler
extern irq_handler

; Generic entry
isr_common:
    pusha                 ; save general regs
    mov eax, [esp + 32]   ; int_no   (after pusha: pusha added 32 bytes)
    mov ebx, [esp + 36]   ; err_code
    push ebx
    push eax
    call isr_handler
    add esp, 8
    popa
    add esp, 8            ; pop fake err + int_no
    iretd

; Some exceptions that already pushed err_code need a slightly
; different stack cleanup. We special-cased isr8 jumping here.
isr_common_err:
    pusha
    mov eax, [esp + 32]   ; int_no
    mov ebx, [esp + 28]   ; err_code already on stack (layout differs)
    push ebx
    push eax
    call isr_handler
    add esp, 8
    popa
    add esp, 4            ; pop int_no only (err_code already consumed)
    iretd

; Common IRQ path (hardware interrupts)
irq_common:
    pusha
    mov eax, [esp + 32]   ; int_no
    sub eax, 32           ; convert vector (32..47) -> irq_no (0..15)
    push eax
    call irq_handler
    add esp, 4
    popa
    add esp, 8            ; pop fake err + int_no
    iretd
