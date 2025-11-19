/**
 * @file elf.h
 * @brief ELF (Executable and Linkable Format) Binary Loader
 * 
 * Parses and loads ELF32 executables into memory. ELF is the standard
 * executable format for Unix-like systems, containing machine code,
 * data sections, and metadata about how to load the program.
 * 
 * The loader:
 * - Validates ELF magic number and architecture
 * - Parses program headers to find loadable segments
 * - Maps segments to their virtual addresses
 * - Handles BSS (uninitialized data) sections
 * - Returns the entry point for execution
 * 
 * @note This is a simplified loader - no dynamic linking, no relocations yet
 * @see https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
 */

#ifndef HORIZON_ELF_H
#define HORIZON_ELF_H

#include <stdint.h>

/** @brief ELF magic number: 0x7F followed by "ELF" */
#define ELF_MAGIC 0x464C457F

/** @brief 32-bit ELF format */
#define ELFCLASS32 1

/** @brief 64-bit ELF format (not supported yet) */
#define ELFCLASS64 2

/** @brief Little-endian data encoding (x86 uses this) */
#define ELFDATA2LSB 1

/** @brief No file type */
#define ET_NONE   0

/** @brief Relocatable file (.o object files) */
#define ET_REL    1

/** @brief Executable file (what we care about!) */
#define ET_EXEC   2

/** @brief Shared object file (.so libraries) */
#define ET_DYN    3

/** @brief Intel 80386 (x86 32-bit) */
#define EM_386    3

// PHeader types

/** @brief Unused program header entry */
#define PT_NULL   0

/** @brief Loadable segment (code, data, BSS) */
#define PT_LOAD   1

/** @brief Dynamic linking information */
#define PT_DYNAMIC 2

/** @brief Interpreter path (for dynamic linking) */
#define PT_INTERP  3

/** @brief Auxiliary information */
#define PT_NOTE    4

/** @brief Reserved (unused) */
#define PT_SHLIB   5

/** @brief Program header table itself */
#define PT_PHDR    6

// Segment flags

/** @brief Segment is executable */
#define PF_X 1

/** @brief Segment is writable */
#define PF_W 2

/** @brief Segment is readable */
#define PF_R 4

/**
 * @brief ELF32 file header
 * 
 * Located at the beginning of every ELF file. Contains metadata about
 * the binary including architecture, entry point, and locations of
 * program/section headers.
 * 
 * Always verify e_ident[0-3] == ELF_MAGIC before trusting other fields!
 */
typedef struct {
    unsigned char e_ident[16]; /**< Magic number, class, endianness, version, ABI */
    uint16_t e_type;           /**< Object file type (ET_EXEC for executables) */
    uint16_t e_machine;        /**< Target architecture (EM_386 for x86) */
    uint32_t e_version;        /**< ELF version (always 1) */
    uint32_t e_entry;          /**< Virtual address of entry point (where to jump!) */
    uint32_t e_phoff;          /**< File offset of program header table */
    uint32_t e_shoff;          /**< File offset of section header table */
    uint32_t e_flags;          /**< Processor-specific flags */
    uint16_t e_ehsize;         /**< Size of this ELF header in bytes */
    uint16_t e_phentsize;      /**< Size of one program header entry */
    uint16_t e_phnum;          /**< Number of program header entries */
    uint16_t e_shentsize;      /**< Size of one section header entry */
    uint16_t e_shnum;          /**< Number of section header entries */
    uint16_t e_shstrndx;       /**< Section header string table index */
} Elf32_Ehdr;

/**
 * @brief ELF32 program header
 * 
 * Describes a segment to be loaded into memory. PT_LOAD segments contain
 * the actual program code and data. Each segment has a file offset, virtual
 * address, size, and permission flags.
 * 
 * p_memsz > p_filesz indicates BSS (uninitialized data) that should be zeroed.
 */
typedef struct {
    uint32_t p_type;   /**< Segment type (PT_LOAD, PT_DYNAMIC, etc.) */
    uint32_t p_offset; /**< Offset of segment data in the ELF file */
    uint32_t p_vaddr;  /**< Virtual address where segment should be mapped */
    uint32_t p_paddr;  /**< Physical address (usually ignored/same as vaddr) */
    uint32_t p_filesz; /**< Size of segment data in the file */
    uint32_t p_memsz;  /**< Size of segment in memory (>= filesz; includes BSS) */
    uint32_t p_flags;  /**< Segment permissions (PF_X | PF_W | PF_R) */
    uint32_t p_align;  /**< Alignment requirement (usually 4096 for page alignment) */
} Elf32_Phdr;

/**
 * @brief Loaded ELF program information
 * 
 * Contains the essential info needed to start executing a loaded ELF binary.
 * Returned by elf_load() after successfully parsing and loading the program.
 */
typedef struct {
    uint32_t entry;          /**< Entry point virtual address (start here!) */
    uint32_t stack_pointer;  /**< Initial stack pointer for user mode */
    // TODO: Add more fields as needed (e.g., heap start, program break)
} elf_program_t;

/**
 * @brief Load an ELF binary into memory
 * 
 * Parses an ELF file, validates its format, and loads all PT_LOAD segments
 * into memory at their specified virtual addresses. Sets up BSS (zero-filled
 * uninitialized data) and returns the entry point.
 * 
 * Steps:
 * - 1. Validate ELF magic, class (32-bit), and architecture (x86)
 * - 2. Parse program headers
 * - 3. For each PT_LOAD segment:
 *   - Allocate pages at segment's virtual address
 *   - Copy segment data from file
 *   - Zero any BSS (p_memsz > p_filesz)
 *   - Set page permissions based on segment flags
 * - 4. Return entry point and stack pointer
 * 
 * @param data Pointer to ELF file data in memory
 * @param size Size of ELF file in bytes
 * @param out Output structure to fill with entry point and stack info
 * 
 * @return 0 on success, -1 on error (invalid ELF, unsupported format, etc.)
 * 
 * Example:
 * @code
 * uint8_t *elf_data = read_file("/bin/hello");
 * elf_program_t prog;
 * if (elf_load(elf_data, file_size, &prog) == 0) {
 *     jump_to_usermode(prog.stack_pointer);
 *     // Jump to prog.entry happens inside jump_to_usermode()
 * }
 * @endcode
 * 
 * @note Does not support dynamic linking, relocations, or shared libraries yet
 * @warning Assumes virtual memory is already set up for userspace!
 */
int elf_load(const uint8_t *data, uint32_t size, elf_program_t *out);

#endif // HORIZON_ELF_H