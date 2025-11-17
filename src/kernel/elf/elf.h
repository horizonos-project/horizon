#ifndef HORIZON_ELF_H
#define HORIZON_ELF_H

#include <stdint.h>

#define ELF_MAGIC 0x464C457F // \x7FELF

// ELF classes
#define ELFCLASS32 1
#define ELFCLASS64 2

// ELF Data Encoding
#define ELFDATA2LSB 1

// ELF types
#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3

// Machine types
#define EM_386    3

// PHeader types
#define PT_NULL   0
#define PT_LOAD   1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6

// Segment flags
#define PF_X 1
#define PF_W 2
#define PF_R 4

// ELF Header (32-bit)
typedef struct {
    unsigned char e_ident[16]; // Magic, ABI, class, etc.
    uint16_t e_type;           // Relocatable, executable, etc.
    uint16_t e_machine;        // Architecture
    uint32_t e_version;        // Always 1
    uint32_t e_entry;          // Entry point virtual address
    uint32_t e_phoff;          // Program header table offset
    uint32_t e_shoff;          // Section header table offset
    uint32_t e_flags;          // Processor flags
    uint16_t e_ehsize;         // ELF header size
    uint16_t e_phentsize;      // Program header entry size
    uint16_t e_phnum;          // Number of program headers
    uint16_t e_shentsize;      // Section header entry size
    uint16_t e_shnum;          // Number of section headers
    uint16_t e_shstrndx;       // Section header string table index
} Elf32_Ehdr;

// ELF32 Program Header
typedef struct {
    uint32_t p_type;   // PT_LOAD, PT_DYNAMIC, etc.
    uint32_t p_offset; // Offset of segment in file
    uint32_t p_vaddr;  // Virtual address to map
    uint32_t p_paddr;  // Physical addr (ignored for us)
    uint32_t p_filesz; // Size of segment data in file
    uint32_t p_memsz;  // Size of segment in memory (includes BSS)
    uint32_t p_flags;  // PF_X | PF_W | PF_R
    uint32_t p_align;  // Alignment (usually 4K)
} Elf32_Phdr;

typedef struct {
    uint32_t entry;
    uint32_t stack_pointer;
    // todo: more feilds as needed
} elf_program_t;

int elf_load(const uint8_t *data, uint32_t size, elf_program_t *out);

#endif // HORIZON_ELF_H