# Horizon 0.04 roadmap

## Milestone 1: More syscalls

This is fairly self explanitory. This OS needs more syscalls in order to function properly.

## Milestone 2: Process Structure

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

## Milestone 3: Userspace library basics

POSIX-like wrappers for compatibility, not super expansive, enough to not explode.

- `write()`, `read()`, `open()`, `close()`
- `exit()`, `getpid()`

We also need a libc.a and crt0.s for the sake of startup and important functions.
