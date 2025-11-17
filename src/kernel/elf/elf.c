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
// p_vaddr  = virtual address
// p_offset = file offset
// p_filesz = size in file
// p_memsz  = size in memory
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

    for (uint32_t addr = page_start; addr < page_end; addr += 0x1000) {
        void *phys = pmm_alloc_frame();
        if (!phys) {
            klogf("[elf] Out of physical memory mapping segment!\n");
            kprintf_both("[mem] Out of physical memory!\n");
            return -1;
        }

        vmm_map_page(addr, (uint32_t)phys, PAGE_PRESENT | PAGE_RW | PAGE_USER);
    }

    // Copy segment data from file into vmem
    uint8_t *dest = (uint8_t *)vaddr;
    const uint8_t *src = data + offset;

    if (filesz > 0) {
        memcpy(dest, src, filesz);
    }
}