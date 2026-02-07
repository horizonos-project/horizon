* Load kernel segment selectors before calling C in syscall stub

  * **Where:** `src/kernel/syscall/syscall_asm.S`
  * **Do:** set `ds/es/fs/gs = 0x10` before calling `syscall_handler`

* Load kernel segment selectors before calling C in ISR/IRQ stubs

  * **Where:** `src/kernel/isr_stubs.S`
  * **Do:** in `isr_common_stub` and `irq_common_stub`, set `ds/es/fs/gs = 0x10` before calling C handler

* Fix/verify `regs_t` ↔ ASM push order alignment

  * **Where:** `src/kernel/isr_stubs.S` + `src/kernel/isr.h` (`regs_t`)
  * **Do:** confirm struct matches stack layout exactly for ISR/IRQ/syscall frames
  * **Do:** add a one-time debug assert/log using known register values to validate

* Fix blocking stdin under interrupt-gate syscalls (`0xEE`)

  * **Where:** `src/kernel/syscall/*` (your `sys_read`)
  * **Do:** replace `hlt` wait with `sti; hlt; cli` (interim) **or** stop blocking until scheduler exists
  * **Later:** replace with “sleep current process until input available”

* Remove echo-from-IRQ behavior (later but recommended)

  * **Where:** `src/drivers/keyboard/keyboard.c`
  * **Do:** remove `kputc(c);` from `keyboard_irq`; move echo to tty/console layer or userspace

* Unify selector constants (stop magic numbers)

  * **Where:** new header (e.g., `src/arch/x86/selectors.h`) and update usages in:

    * `src/kernel/usermode.c`
    * `src/kernel/tss.c`
    * `src/kernel/isr.c` / `src/kernel/idt.c` / syscall init
  * **Do:** define `KERNEL_CS/KERNEL_DS/USER_CS/USER_DS/TSS_SEL`

* Reconcile user segment selectors mismatch (TSS vs usermode jump)

  * **Where:** `src/kernel/tss.c` + `src/kernel/usermode.c`
  * **Do:** make both use the same `USER_CS/USER_DS` values

* Verify ring3→ring0 stack switching is actually using TSS `esp0`

  * **Where:** `src/kernel/isr.c` (page fault handler) + stubs if needed
  * **Do:** log `(r->cs & 3)`, handler `esp`, and `r->useresp` when CPL=3; confirm handler `esp` is kernel stack
  * **Do:** ensure `ltr` done once and `tss.esp0` valid

* Make `brk` per-process (not global static)

  * **Where:** `src/kernel/syscall/*` (`sys_brk`) + new proc struct
  * **Do:** move `heap_start/current_brk` into process state

* Implement minimal process system skeleton (struct + current)

  * **Where:** new `src/kernel/proc/*` (or similar) + integrate in `src/kernel/main.c`
  * **Do:** `proc_t` with pid, state, page dir, kstack top, user stack, fd table ptr, brk, regs/context

* Scheduler minimal round-robin

  * **Where:** new `src/kernel/sched/*` + hook PIT (`src/kernel/...` wherever PIT handler lives)
  * **Do:** context switch between at least idle + one user proc
  * **Do:** on switch, call `tss_set_kernel_stack(next->kstack_top)`

* Fix `sys_exit` to terminate process, not halt OS

  * **Where:** `src/kernel/syscall/*` (`sys_exit`)
  * **Do:** cleanup proc resources, mark ZOMBIE, reschedule

* Implement `waitpid` (minimal)

  * **Where:** `src/kernel/syscall/*` + proc system
  * **Do:** wait for child, return exit status, handle zombies

* Implement `fork` (initial: full copy)

  * **Where:** `src/kernel/syscall/*` + proc/mm
  * **Do:** duplicate address space + fd table, child returns 0 (edit saved regs), parent returns child pid

* Implement `execve` using ELF loader + ext2 VFS

  * **Where:** `src/kernel/syscall/*` (`sys_execve`) + `src/kernel/elf/elf.c` + VFS/ext2
  * **Do:** load ELF from ext2 via VFS, replace current address space, build new user stack (argv/envp), jump to entry

* Add per-process FD table + file object model (if not already)

  * **Where:** `src/drivers/vfs/*` + proc struct
  * **Do:** stdin/stdout/stderr as file objects; remove fd special-casing from syscalls

* Implement `pipe`

  * **Where:** new syscall + VFS/pipe backend
  * **Do:** create two fds (read/write ends) backed by ring buffer

* Implement `dup2`

  * **Where:** new syscall + fd table
  * **Do:** duplicate file object into target fd, close target first if open

* Repo/file organization (optional but recommended)

  * **Where:** move files into `arch/x86/`, `kernel/`, `drivers/`, `fs/`, `libk/`
  * **Do:** relocate `idt/gdt/tss/isr_stubs/syscall_asm` under `arch/x86/` and update includes/Makefile paths

