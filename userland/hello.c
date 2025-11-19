// This is a hello program that exists in Horizon's userland
// This is just a test binary, don't mind it :P
#define SYS_EXIT   1
#define SYS_READ   3
#define SYS_WRITE  4
#define SYS_OPEN   5
#define SYS_CLOSE  6
#define SYS_GETPID 20
#define SYS_BRK    45

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

static inline int syscall5(int num, int arg1, int arg2, int arg3, int arg4, int arg5) {
    int ret;
    __asm__ volatile("int $0x80" 
                     : "=a"(ret) 
                     : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5));
    return ret;
}

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

static inline int getpid(void) {
    int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_GETPID));
    return ret;
}

static inline unsigned int brk(unsigned int addr) {
    return syscall5(SYS_BRK, addr, 0, 0, 0, 0);
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
    
    // Test brk!
    print("\nTesting sys_brk()...\n");
    
    unsigned int initial = brk(0);
    print("Got initial brk\n");
    
    // Allocate 4KB
    unsigned int new_brk = brk(initial + 0x1000);
    
    if (new_brk == initial + 0x1000) {
        print("brk() allocated 4KB successfully!\n");
        
        // Actually write to the heap
        char *heap = (char*)initial;
        heap[0] = 'O';
        heap[1] = 'K';
        heap[2] = '!';
        heap[3] = '\n';
        
        write(1, "Heap test: ", 11);
        write(1, heap, 4);
    } else {
        print("brk() failed :(\n");
    }

    int motd_fd = open("/etc/motd", 0);
    if (motd_fd < 0) {
        print("File not found! (/etc/motd)\n");
    }

    char motdbuf[128];
    int n = read(motd_fd, motdbuf, sizeof(motdbuf) - 1);

    if (n > 0) {
        motdbuf[n] = '\0';
        print("MOTD contents:\n");
        write(1, motdbuf, n);
        print("\n");
    }

    close(motd_fd);

    print("Exiting cleanly...\n");
    exit(0);
}
