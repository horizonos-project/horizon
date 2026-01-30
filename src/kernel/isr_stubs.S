// ISR implementation in GNU assembly
// (c) HorizonOS Project 2025-

.code32

.global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
.global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
.global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
.global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31

.global irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7
.global irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15

.extern isr_handler
.extern irq_handler

// Macro for ISRs without error code
.macro ISR_NOERR num
isr\num:
    cli
    pushl $0            // fake error code
    pushl $\num         // interrupt number
    jmp isr_common_stub
.endm

// Macro for ISRs with error code
.macro ISR_ERR num
isr\num:
    cli
    pushl $\num         // interrupt number (error code already pushed by CPU)
    jmp isr_common_stub
.endm

// Exception handlers (0-31)
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

// Macro for IRQ handlers
.macro IRQ_STUB irq, vector
irq\irq:
    pushl $0            // fake error code
    pushl $\vector      // interrupt number (32 + IRQ)
    jmp irq_common_stub
.endm

// IRQ handlers (32-47)
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

// Common ISR handler
isr_common_stub:
    pusha

    // Save current segment registers (could be user segments if coming from ring3)
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs

    // Load kernel data segments for safe C execution
    movw $0x10, %ax          // KERNEL_DS
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    // Pass &regs_t (points at gs,fs,es,ds,...) to C
    movl %esp, %eax
    pushl %eax
    call isr_handler
    addl $4, %esp

    // Restore original segment registers
    popl %gs
    popl %fs
    popl %es
    popl %ds

    popa

    addl $8, %esp            // drop int_no and err_code
    iret


// Common IRQ handler
irq_common_stub:
    pusha

    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs

    movw $0x10, %ax          // KERNEL_DS
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    movl %esp, %eax
    pushl %eax
    call irq_handler
    addl $4, %esp

    popl %gs
    popl %fs
    popl %es
    popl %ds

    popa

    addl $8, %esp
    iret
