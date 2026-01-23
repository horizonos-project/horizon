.code32
.global isr_syscall_stub
.extern syscall_handler

isr_syscall_stub:
    cli                     // optional; int gate already clears IF

    pushl $0                // err_code (fake)
    pushl $0x80             // int_no (so regs_t lines up)

    pusha

    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs

    movw $0x10, %ax         // KERNEL_DS
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    movl %esp, %eax         // &regs_t
    pushl %eax
    call syscall_handler
    addl $4, %esp

    // --- IMPORTANT: preserve return value ---
    // syscall_handler wrote r->eax (in the struct) but popa would overwrite EAX.
    // At this point, the saved EAX in the pusha frame lives at:
    //   [esp + 16 + 28] if you count seg pushes, BUT easiest is: compute address from current esp layout.
    //
    // Layout right now (top at esp):
    //   gs fs es ds
    //   EDI ESI EBP ESP EBX EDX ECX EAX   <- pusha block (EAX is last in memory)
    //   int_no err_code
    //   (then CPU frame: eip cs eflags ...)
    //
    // So saved EAX is at [esp + 4*4 + 7*4] = [esp + 16 + 28] = [esp + 44]
    movl 44(%esp), %eax      // load saved eax slot (currently contains return if C wrote it correctly)
    // But: C wrote into struct field, so it's already at 44(%esp); we just need popa to restore it.
    // Therefore, do nothing here IF the struct alignment is correct.
    // (If you later change push order, revisit.)

    popl %gs
    popl %fs
    popl %es
    popl %ds

    popa

    addl $8, %esp           // drop int_no and err_code
    iret
