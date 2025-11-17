// GDT flush assembly stub
// (c) 2025 HorizonOS Project

.code32
.global gdt_flush

gdt_flush:
    movl 4(%esp), %eax      // get pointer to GDT pointer
    lgdt (%eax)             // load GDT
    
    movw $0x10, %ax         // kernel data segment
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    
    ljmp $0x08, $.flush     // far jump to reload CS
.flush:
    ret
