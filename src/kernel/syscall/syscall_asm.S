// Syscall handler stub
// (c) 2025 HorizonOS Project

.code32
.global isr_syscall_stub
.extern syscall_handler

isr_syscall_stub:
    pusha
    
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    
    movw $0x10, %ax     // kernel data segment
    movw %ax, %ds
    movw %ax, %es
    
    pushl %esp
    call syscall_handler
    addl $4, %esp
    
    popl %gs
    popl %fs
    popl %es
    popl %ds
    
    popa
    iret
