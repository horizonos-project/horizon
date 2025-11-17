;
; ISR implementation, has to be done in ASM.
; This won't be too bad, I'm just not a fan of ASM.
;
; (c) HorizonOS Project 2025-
;

[bits 32]

global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31

global irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7
global irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15

global isr_common_stub
extern isr_handler
extern irq_handler

; Helper macros:
; - For exceptions without errcode: push 0 then int_no
; - For exceptions with hardware error code: push int_no only
;   (since CPU pushed errcode)

%macro ISR_NOERR 1
isr%1:
    cli
    push dword 0
    push dword %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERR 1
isr%1:
    cli
    push dword %1         ; int_no (err_code already on stack)
    jmp isr_common_stub
%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
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

; IRQ stubs: map to 32–47
%macro IRQ_STUB 2
irq%1:
    push dword 0          ; fake error code
    push dword %2         ; int number (32 + IRQn)
    jmp irq_common_stub
%endmacro

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
IRQ_STUB 10, 42
IRQ_STUB 11, 43
IRQ_STUB 12, 44
IRQ_STUB 13, 45
IRQ_STUB 14, 46
IRQ_STUB 15, 47

; ISR_COMMON_STUB handler glue
isr_common_stub:
    ; On entry:
    ; [esp+0] ~> int_no
    ; [esp+4] ~> errcode
    ; CPU pushed EIP, CS, EFLAGS, ESP, SS (if usr)

    pusha
    push ds
    push es
    push fs
    push gs

    ; mov ax, 0x10        ; kernel data selector (assumes 0x10)
    ; mov ds, ax
    ; mov es, ax
    ; mov fs, ax
    ; mov gs, ax

    mov eax, esp        ; regs_t* argument
    push eax
    call isr_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds

    popa

    add esp, 8          ; drop int_no + err_code
    
    iretd

; I cannot believe we got this close to userland without this
; Damn, that's both horrifying and neat.
irq_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs

    ; mov ax, 0x10
    ; mov ds, ax
    ; mov es, ax
    ; mov fs, ax
    ; mov gs, ax

    mov eax, esp
    push eax
    call irq_handler    ; ← IRQs go here!
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa

    add esp, 8
    
    iretd