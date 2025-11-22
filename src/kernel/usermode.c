#include "../libk/string.h"
#include "kernel/usermode.h"
#include "kernel/log.h"
#include "kernel/panic.h"
#include "mm/heap.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "elf/elf.h"
#include "../drivers/vfs/vfs.h"

void usermode_entry(void);

#define USER_CODE_SIZE 4096

void jump_to_elf(const char *path) {
    klogf("\n[elf] === Loading ELF Binary ===\n");
    klogf("[elf] Path: %s\n", path);
    
    // Open file via VFS
    int fd = vfs_open(path, 0);  // flags = 0 (read-only)
    if (fd < 0) {
        klogf("[elf] Failed to open %s (fd=%d)\n", path, fd);
        panicf("ELF LOAD FAILED (VFS)");
    }

    // Get file size via stat
    stat_t st;
    if (vfs_stat(path, &st) < 0) {
        klogf("[elf] Failed to stat %s\n", path);
        vfs_close(fd);
        panicf("ELF LOAD FAILED (STAT_T)");
    }

    uint32_t file_size = st.size;
    klogf("[elf] File size: %u bytes\n", file_size);

    // Allocate buffer for file
    uint8_t *data = (uint8_t *)kalloc(file_size);
    if (!data) {
        klogf("[elf] Failed to allocate %u bytes for %s\n", file_size, path);
        vfs_close(fd);
        panicf("ELF LOAD FAILED (BUFFER ALLOC)");
    }

    // Read entire file
    int bytes_read = vfs_read(fd, data, file_size);
    if (bytes_read < 0 || (uint32_t)bytes_read != file_size) {
        klogf("[elf] Failed to read %s (got %d bytes)\n", path, bytes_read);
        kfree(data);
        vfs_close(fd);
        panicf("ELF LOAD FAILED (FILE READ)");
    }

    vfs_close(fd);
    
    klogf("[elf] Read %u bytes successfully\n", file_size);

    // Load ELF
    elf_program_t prog;
    if (elf_load(data, file_size, &prog) < 0) {
        klogf("[elf] Failed to load ELF\n");
        kfree(data);
        panicf("ELF LOAD FAILED (ELF_PROGRAM_T)");
    }

    kfree(data);  // Don't need file buffer anymore

    klogf("[elf] Jumping to entry point: 0x%08x\n", prog.entry);
    klogf("[elf] Stack: 0x%08x\n", prog.stack_pointer);

    // Jump to ELF entry point
    __asm__ volatile(
        "cli\n"
        
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        
        "pushl $0x23\n"            // SS
        "pushl %0\n"               // ESP (from ELF)
        
        "pushf\n"
        "popl %%eax\n"
        "orl $0x200, %%eax\n"
        "pushl %%eax\n"            // EFLAGS
        
        "pushl $0x1B\n"            // CS
        "pushl %1\n"               // EIP (from ELF)
        
        "iret\n"
        :
        : "r"(prog.stack_pointer), "r"(prog.entry)
        : "eax"
    );

    klogf("USERMODE ESCAPE");
}

void jump_to_usermode(uint32_t user_stack) {
    klogf("[r3] === Jumping from r0 to r3 ===\n");
    klogf("[r3] User stack: 0x%08x\n", user_stack);
    klogf("[r3] Entrypoint: 0x%08x\n", (uint32_t)usermode_entry);
    klogf("[r3] Preparing iretd stack frame...\n");

    uint32_t user_code_phys = (uint32_t)pmm_alloc_frame();
    if (!user_code_phys) {
        kprintf_both("[stack] Failed to allocate physical frame for user code\n");
        for(;;);
    }
    
    // Step 2: Choose a virtual address in user space (0x400000 is conventional)
    uint32_t user_code_virt = 0x00400000;

    klogf("[r3] Allocated user code frame at phys: 0x%08x\n", user_code_phys);
    klogf("[r3] Mapping to virtual address: 0x%08x\n", user_code_virt);

    vmm_map_page(user_code_virt, user_code_phys, PAGE_PRESENT | PAGE_RW | PAGE_USER);

    uint32_t kernel_code_addr = (uint32_t)usermode_entry;

    klogf("[r3] Copying code from 0x%08x to usr 0x%08x\n", kernel_code_addr, user_code_virt);
    memcpy((void*)user_code_virt, (void*)kernel_code_addr, USER_CODE_SIZE);

    // Gotta love that this moment is just inline asm lol
    __asm__ volatile(
        "cli\n"                     // Disable interrupts during transition
        
                                    // Set up data segments for user mode
        "mov $0x23, %%ax\n"         // User data segment (0x20 | 3)
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        
                                    // Push values for iretd (in reverse order)
        "pushl $0x23\n"             // SS (user data segment)
        "pushl %0\n"                // ESP (user stack)
        
                                    // Push EFLAGS with IF enabled
        "pushf\n"
        "popl %%eax\n"
        "orl $0x200, %%eax\n"       // Set IF (interrupts enabled in usermode)
        "pushl %%eax\n"
        
        "pushl $0x1B\n"             // CS (user code segment = 0x18 | 3)
        "pushl %1\n"                // EIP (entry point)
        
        "iret\n"                    // Full r3 jump!
        :
        : "r"(user_stack), "r"(user_code_virt)
        : "eax"
    );

    // If we are here, things have gone EXTREMELY BAD.
    kprintf_both("[fatal] Returned from usermode!\n");
    kprintf_both("[fatal] System halted!\n");
    for (;;);
}

void usermode_entry(void) {
    uint32_t counter = 0;
    
    while(1) {
        char c = '0' + (counter % 10);
        
        __asm__ volatile(
            "mov $4, %%eax\n"      // SYS_WRITE
            "mov $1, %%ebx\n"      // stdout
            "lea %0, %%ecx\n"      // address of 'c' (on stack)
            "mov $1, %%edx\n"      // length = 1
            "int $0x80\n"
            :
            : "m"(c)
            : "eax", "ebx", "ecx", "edx"
        );
        
        counter++;
        
        for (volatile int i = 0; i < 10000000; i++);
    }
}