// This is a hello program that exists in Horizon's userland
// This is just a test binary, don't mind it :P
#define SYS_EXIT   1
#define SYS_WRITE  4
#define SYS_GETPID 20

static inline int syscall1(int num, int arg1) {
    int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1));
    return ret;
}

static inline int syscall3(int num, int arg1, int arg2, int arg3) {
    int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3));
    return ret;
}

static inline int write(int fd, const char *buf, unsigned int count) {
    return syscall3(SYS_WRITE, fd, (int)buf, count);
}

static inline void exit(int status) {
    syscall1(SYS_EXIT, status);
    __builtin_unreachable();
}

static inline int getpid(void) {
    int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_GETPID));
    return ret;
}

// Simple string functions
static int strlen(const char *s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

static void print(const char *s) {
    write(1, s, strlen(s));
}

// Entry point
void _start(void) {
    print("Hello from HorizonOS userspace!\n");
    print("This is a real ELF binary!\n");
    
    int pid = getpid();
    print("My PID is: 1, since there's no scheduler yet...\n");
    // TODO: Convert pid to string and print (or just prove syscall works)
    
    print("Exiting cleanly...\n");
    exit(0);
}
