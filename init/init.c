/**
 * The technically immortal init process. This proc cannot be reaped
 * or otherwise stopped, doingo so will hang the kernel. This proc
 * should always be PID 0.
 * 
 * We aren't going to check that, that's on the actual kernel to
 * make that check. So what is init? Init is an interesting process.
 * It's responsible for initalizing core user services and keeping
 * the CPU busy with at least one process.
 * 
 */

#define SYS_EXIT   1
#define SYS_READ   3
#define SYS_WRITE  4
#define SYS_OPEN   5
#define SYS_CLOSE  6
#define SYS_BRK    45
#define SYS_CLEAR_VGA 500

static inline int syscall1(int num, int arg1) {
    int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1));
    return ret;
}

static inline int syscall3(int num, int arg1, int arg2, int arg3) {
    int ret;
    __asm__ volatile("int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3));
    return ret;
}

static inline int syscall5(int num, int arg1, int arg2, int arg3, int arg4, int arg5) {
    int ret;
    __asm__ volatile("int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5));
    return ret;
}

// --------------------------------------------------
// Syscall wrappers
// --------------------------------------------------

static inline int open(const char *path, int flags) {
    return syscall3(SYS_OPEN, (int)path, flags, 0);
}

static inline int close(int fd) {
    return syscall1(SYS_CLOSE, fd);
}

static inline int read(int fd, void *buf, unsigned int count) {
    return syscall3(SYS_READ, fd, (int)buf, count);
}

static inline int write(int fd, const char *buf, unsigned int count) {
    return syscall3(SYS_WRITE, fd, (int)buf, count);
}

static inline void exit(int status) {
    syscall1(SYS_EXIT, status);
    __builtin_unreachable();
}

static inline unsigned int brk(unsigned int addr) {
    return syscall5(SYS_BRK, addr, 0, 0, 0, 0);
}

static inline unsigned int clear() {
    return syscall5(SYS_CLEAR_VGA, 0, 0, 0, 0, 0);
}

static int strlen(const char *s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

static void print(const char *s) {
    write(1, s, strlen(s));
}

// --------------------------------------------------
// Entry point
// --------------------------------------------------

void _start(void) {
    clear();
    print("Horizon init online.\n");
    exit(0);
}

