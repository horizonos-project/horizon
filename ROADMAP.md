# Horizon 0.03 roadmap

## Milestone 1: Jump to ring3

The main goal of milestone 1 is just to consistently get from booting into a userland smoothly on both a raw boot and from the ISO. This is already partly possible but has yet to be finalized since it's fairly complex all things considered. Right now the GDT is ready but we still need a TSS and to actually jump to userspace and setup a userstack.

## Milestone 2: Syscalls working

Syscalls are mega important, and desperately need to work. Basic syscalls like write, exit, getpid will be fully implemented and should be usable from at least inline usermode asm by this point.

```c
asm("mov $4, %eax; mov $1, %ebx; ...; int $0x80");
```

## Milestone 3: Load User Binary from initramfs (3-5 days)

**Tasks:**
1. Write a simple userspace program in C
2. Compile it as flat binary (no ELF yet)
3. Add it to initramfs
4. Write `load_binary()` function:
   - Read from VFS
   - Allocate user memory
   - Copy binary to user address space
   - Set up entry point
5. Jump to loaded binary

As long as this can work properly and with more than one binary, things are smooth.

## Milestone 4: Process Structure

Multiple processes is vital to an OS doing anything besides the eternal spin of nothingness.

```c
typedef struct {
    uint32_t pid;
    uint32_t esp;         // User stack
    uint32_t eip;         // Entry point
    uint32_t *page_dir;   // Page directory
    uint8_t state;        // RUNNING, READY, etc.
} process_t;
```
2. Implement `create_process(char *path)`
3. Implement process state tracking
4. Simple process list (no scheduling yet)

## Milestone 5: Userspace library basics

POSIX-like wrappers for compatibility, not super expansive, enough to not explode.

- `write()`, `read()`, `open()`, `close()`
- `exit()`, `getpid()`

We also need a libc.a and crt0.s for the sake of startup and important functions.

## Bonuses for 0.03: ELF Loader, Scheduling

A basic, partially complete ELF loader that can take a plain elf32 binary (an actual ELF, not flat bin) and unpack it, and execute it properly! This is more of a stretch goal and may not be 100% finished by the time 0.03 is released and ready.

And scheduling, a round-robin like scheduler for tasks has been decided on for the sake of sanity.