#include "elf.h"
#include "../log.h"
#include "../../mm/vmm.h"
#include "../../mm/pmm.h"
#include "../../libk/string.h"

// Internal ELF header validation
static int elf_validate_header(const Elf32_Ehdr *hdr) {
    uint32_t magic =
        (hdr->e_ident[0]) |
        (hdr->e_ident[1] << 8) |
        (hdr->e_ident[2] << 16) |
        (hdr->e_ident[3] << 24);

    if (magic != ELF_MAGIC) {
        klogf("[elf] Invalid magic: 0x%x\n", magic);
        return -1;
    }

    if (hdr->e_ident[4] != ELFCLASS32) {
        klogf("[elf] Unsupported ELF class %u (expected 32)\n", hdr->e_ident[4]);
        return -1;
    }

    if (hdr->e_ident[5] != ELFDATA2LSB) {
        klogf("[elf] Unsupported endianness\n");
        return -1;
    }

    if (hdr->e_type != ET_EXEC) {
        klogf("[elf] Not an executable (e_type=%u)\n", hdr->e_type);
        return -1;
    }

    if (hdr->e_machine != EM_386) {
        klogf("[elf] Unsupported machine %u (expected x86)\n", hdr->e_machine);
        return -1;
    }

    return 0;
}

// Mapping a PT_LOAD segment
static int elf_load_segment(const uint8_t *data, const Elf32_Phdr *phdr) {
    uint32_t vaddr  = phdr->p_vaddr;
    uint32_t memsz  = phdr->p_memsz;
    uint32_t filesz = phdr->p_filesz;
    uint32_t offset = phdr->p_offset;

    if (phdr->p_type != PT_LOAD)
        return 0;
        // Skipping non load segments

    // Allocate + map mem for this segment
    uint32_t page_start = vaddr & ~0xFFF;
    uint32_t page_end   = (vaddr + memsz + 0xFFF) & ~0xFFF;

    klogf("[elf] Mapping pages: 0x%08x -> 0x%08x\n", page_start, page_end);

    for (uint32_t addr = page_start; addr < page_end; addr += 0x1000) {
        void *phys = pmm_alloc_frame();
        if (!phys) {
            klogf("[elf] Out of physical memory mapping segment!\n");
            return -1;
        }

        vmm_map_page(addr, (uint32_t)phys, PAGE_PRESENT | PAGE_RW | PAGE_USER);
    }

    // Copy segment data from file into vmem
    uint8_t *dest = (uint8_t *)vaddr;
    const uint8_t *src = data + offset;

    if (filesz > 0) {
        klogf("[elf] Copying %u bytes to 0x%08x\n", filesz, vaddr);
        memcpy(dest, src, filesz);
    }

    // Zero out the BSS
    if (memsz > filesz) {
        uint32_t bss_size = memsz - filesz;
        klogf("[elf] Zeroing BSS: %u bytes at 0x%08x\n", bss_size, vaddr + filesz);
        memset(dest + filesz, 0, bss_size);
    }

    return 0;
}

int elf_load(const uint8_t *data, uint32_t size, elf_program_t *out) {
    if (!data || !out) {
        klogf("[elf] Invalid parameters!\n");
        return -1;
    }

    if (size < sizeof(Elf32_Ehdr)) {
        klogf("[elf] File too small for ELF header\n");
        return -1;
    }

    // Parse the ELF32 header
    const Elf32_Ehdr *ehdr = (const Elf32_Ehdr *)data;
    if (elf_validate_header(ehdr) < 0) {
        return -1;
    }

    klogf("[elf] Valid elf32 executable!\n");
    klogf("[elf] Entry point: 0x%08x\n", ehdr->e_entry);
    klogf("[elf] Program headers: %u at offset %u\n", ehdr->e_phnum, ehdr->e_phoff);

    // Verify program header table is within bounds
    if (ehdr->e_phoff + (ehdr->e_phnum * ehdr->e_phentsize) > size) {
        klogf("[elf] Program headers extend past end of file\n");
        return -1;
    }

    // Load each PT_LOAD segment (i do not like how this looks)
    const Elf32_Phdr *phdr = (const Elf32_Phdr *)(data + ehdr->e_phoff);
    
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (elf_load_segment(data, &phdr[i]) < 0) {
            klogf("[elf] Failed to load segment %d\n", i);
            return -1;
        }
    }

    // Fill out program info
    out->entry = ehdr->e_entry;
    
    // Allocate user stack (conventional location: just below 3GB)
    // Top of user space - 4KB
    uint32_t user_stack_virt = 0xBFFFF000;  
    
    void *stack_phys = pmm_alloc_frame();
    if (!stack_phys) {
        klogf("[elf] Failed to allocate user stack\n");
        return -1;
    }

    vmm_map_page(user_stack_virt, (uint32_t)stack_phys, PAGE_PRESENT | PAGE_RW | PAGE_USER);

    // Stack grows down, so point to top (stop forgetting this)
    out->stack_pointer = user_stack_virt + 0x1000;

    klogf("[elf] User stack at 0x%08x\n", out->stack_pointer);
    klogf("[elf] ELF loaded successfully\n");

    return 0;

}