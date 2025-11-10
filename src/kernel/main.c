#include <stdint.h>
#include "utilities/assert.h"
#include "utilities/vga_print.h"
#include "../drivers/vfs/vfs.h"

extern void dummy_fs_init(void);

void kmain(void) {
    vga_clear();
    vga_puts("Horizon x86 booting...\n");

    vga_puts("Initalizing VFS...\n");
    if (vfs_init() < 0) {
        vga_puts("ERROR: VFS init failure!\n");
        goto hang;
    }

    // Register and mount dummy filesystem
    dummy_fs_init();
    if (vfs_mount("dummy", NULL, "/") < 0) {
        vga_puts("ERROR: Failed to mount root filesystem!\n");
        goto hang;
    }
    
    vga_puts("VFS initialized, dummy FS mounted at /\n\n");
    
    vga_puts("=== Test 1: Reading /hello.txt ===\n");
    int fd = vfs_open("/hello.txt", O_RDONLY);
    if (fd < 0) {
        vga_puts("ERROR: Could not open /hello.txt\n");
    } else {
        char buf[128];
        int n = vfs_read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            vga_puts("Contents: ");
            vga_puts(buf);
        } else {
            vga_puts("ERROR: bad read!");
        }
        vfs_close(fd);
    }
    
    // Test 2: Read test.txt
    vga_puts("\n=== Test 2: Reading /test.txt ===\n");
    fd = vfs_open("/test.txt", O_RDONLY);
    if (fd < 0) {
        vga_puts("ERROR: Could not open /test.txt\n");
    } else {
        char buf[128];
        int n = vfs_read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            vga_puts("Contents: ");
            vga_puts(buf);
        }
        vfs_close(fd);
    }
    
    // Test 3: Try non-existent file
    vga_puts("\n=== Test 3: Non-existent file ===\n");
    fd = vfs_open("/nonexistent.txt", O_RDONLY);
    if (fd < 0) {
        vga_puts("Correctly rejected non-existent file\n");
    } else {
        vga_puts("ERROR: Opened file that shouldn't exist!\n");
        vfs_close(fd);
    }
    
    // Test 4: stat()
    vga_puts("\n=== Test 4: stat() test ===\n");
    stat_t st;
    if (vfs_stat("/hello.txt", &st) == 0) {
        vga_puts("File size: ");
        // Poor man's integer printing (you'll want a real printf later)
        char size_str[16];
        int i = 0;
        uint32_t size = st.size;
        do {
            size_str[i++] = '0' + (size % 10);
            size /= 10;
        } while (size > 0);
        
        // Reverse the string
        for (int j = 0; j < i / 2; j++) {
            char tmp = size_str[j];
            size_str[j] = size_str[i - j - 1];
            size_str[i - j - 1] = tmp;
        }
        size_str[i] = '\0';
        
        vga_puts(size_str);
        vga_puts(" bytes\n");
    } else {
        vga_puts("ERROR: stat() failed\n");
    }
    
    vga_puts("\n=== All VFS tests complete! ===\n");
    vga_puts("HorizonOS is alive and breathing!\n");

hang:
    vga_puts("\nSystem halted automatically!\n");
    while(1) { 
        asm volatile("hlt"); 
    }
}
