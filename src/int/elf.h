/**
 * @author Sean Hobeck
 * @date 2026-09-05
 */
#ifndef ELF_H
#define ELF_H
/*! uses platform-specific macros. */
#include <tapi/platform.h>
#ifdef TAPI_LINUX
/*! uses tapi_dyna_t. */
#include <tapi/dyna.h>

/*! uses uint8_t, uint16_t, uint32_t, uint64_t. */
#include <stdint.h>

/*! uses ssize_t, pid_t. */
#include <sys/types.h>

/** @brief enum for the elf class. */
typedef enum {
    ELF_CLASS_NONE = 0u,
    ELF_CLASS_32 = 1u,
    ELF_CLASS_64 = 2u
} elf_class_t;

/** @brief enum for the type of elf data encoding. */
typedef enum {
    ELF_DATA_NONE = 0u,
    ELF_DATA_LSB = 1u, /* little endian. */
    ELF_DATA_MSB = 2u  /* big endian. */
} elf_data_t;

/** @brief enum for the elf type, is it a .so, executable, reloc? */
typedef enum {
    ELF_TYPE_NONE = 0u,
    ELF_TYPE_REL = 1u,  /* relocatable. */
    ELF_TYPE_EXEC = 2u, /* executable. */
    ELF_TYPE_DYN = 3u,  /* shared object. */
    ELF_TYPE_CORE = 4u  /* core file. */
} elf_type_t;

/** @brief enum for the elf machine type. */
typedef enum {
    ELF_MACH_NONE = 0u,
    ELF_MACH_X86 = 3u,
    ELF_MACH_ARM = 40u,
    ELF_MACH_X86_64 = 62u,
    ELF_MACH_AARCH64 = 183u
} elf_mach_t;

/* program header flags. */
#define ELF_PF_X 0x1 /* executable. */
#define ELF_PF_W 0x2 /* writable. */
#define ELF_PF_R 0x4 /* readable. */

/* section header flags. */
#define ELF_SHF_WRITE 0x1 /* writable. */
#define ELF_SHF_ALLOC 0x2 /* occupies memory. */
#define ELF_SHF_EXECINSTR 0x4 /* executable. */

/** @brief an elf32 program header. */
typedef struct {
    uint32_t p_type;   /* segment type. */
    uint32_t p_offset; /* file offset. */
    uint32_t p_vaddr;  /* virtual address. */
    uint32_t p_paddr;  /* physical address. */
    uint32_t p_filesz; /* size in file. */
    uint32_t p_memsz;  /* size in memory. */
    uint32_t p_flags;  /* segment flags. */
    uint32_t p_align;  /* segment alignment. */
} __attribute__((packed)) elf32_phdr_t;

/** @brief an elf64 program header. */
typedef struct {
    uint32_t p_type;   /* segment type. */
    uint32_t p_flags;  /* segment flags. */
    uint64_t p_offset; /* file offset. */
    uint64_t p_vaddr;  /* virtual address. */
    uint64_t p_paddr;  /* physical address. */
    uint64_t p_filesz; /* size in file. */
    uint64_t p_memsz;  /* size in memory. */
    uint64_t p_align;  /* segment alignment. */
} __attribute__((packed)) elf64_phdr_t, elf_phdr_t; /* we 'upcast' to this. */

/** @brief an elf32 section header. */
typedef struct {
    uint32_t sh_name;      /* section name (string tbl index). */
    uint32_t sh_type;      /* section type. */
    uint32_t sh_flags;     /* section flags. */
    uint32_t sh_addr;      /* virtual address. */
    uint32_t sh_offset;    /* file offset. */
    uint32_t sh_size;      /* section size. */
    uint32_t sh_link;      /* link to another section. */
    uint32_t sh_info;      /* additional info. */
    uint32_t sh_addralign; /* alignment. */
    uint32_t sh_entsize;   /* entry size if the section holds a table. */
} __attribute__((packed)) elf32_shdr_t;

/** @brief an elf64 section header. */
typedef struct {
    uint32_t sh_name;      /* section name (string tbl index). */
    uint32_t sh_type;      /* section type. */
    uint64_t sh_flags;     /* section flags. */
    uint64_t sh_addr;      /* virtual address. */
    uint64_t sh_offset;    /* file offset. */
    uint64_t sh_size;      /* section size. */
    uint32_t sh_link;      /* link to another section. */
    uint32_t sh_info;      /* additional info. */
    uint64_t sh_addralign; /* alignment. */
    uint64_t sh_entsize;   /* entry size if the section holds a table. */
} __attribute__((packed)) elf64_shdr_t, elf_shdr_t; /* we 'upcast' to this. */

/** @brief an elf32 symbol. */
typedef struct {
    uint32_t st_name;   /* symbol name, idx in string table. */
    uint32_t st_value;  /* value of the symbol. */
    uint32_t st_size;   /* associated symbol size. */
    uint8_t st_info;    /* type and binding attributes. */
    uint8_t st_other;   /* no def. meaning (0) */
    uint16_t st_shndx;  /* associated section index. */
} __attribute__((packed)) elf32_sym_t;

/** @brief an elf64 symbol. */
typedef struct {
    uint32_t st_name;   /* symbol name, idx in string table. */
    uint8_t st_info;    /* type and binding attributes. */
    uint8_t st_other;   /* no def. meaning (0) */
    uint16_t st_shndx;  /* associated section index. */
    uint64_t st_value;  /* value of the symbol. */
    uint64_t st_size;   /* associated symbol size. */
} __attribute__((packed)) elf64_sym_t;

/** @brief an elf32 rela? */
typedef struct {
    uint32_t r_offset;  /* location at which to apply the action. */
    uint32_t r_info;    /* index and type of relocation. */
    int32_t r_addend;   /* constant addend used to compute value. */
} __attribute__((packed)) elf32_rela_t;

/** @brief an elf32 rel? */
typedef struct {
    uint32_t r_offset;  /* location at which to apply the action. */
    uint32_t r_info;    /* index and type of relocation. */
} __attribute__((packed)) elf32_rel_t;

/** @brief an elf64 rela? */
typedef struct {
    uint64_t r_offset;  /* location at which to apply the action. */
    uint64_t r_info;    /* index and type of relocation. */
    int64_t r_addend;   /* constant addend used to compute value. */
} __attribute__((packed)) elf64_rela_t;

/** @brief a structure for an elf file that has been parsed. */
typedef struct {
    elf_class_t class; /* 32 or 64 bit. */
    elf_data_t data; /* endianness. */
    elf_type_t type; /* executable, relocatable, etc... */
    elf_mach_t machine; /* target architecture. */
    uint64_t entry; /* entry point address. */
    uint64_t phoff; /* program header table offset. */
    uint64_t shoff; /* section header table offset. */
    uint16_t phnum; /* number of program headers. */
    uint16_t shnum; /* number of section headers. */
    uint16_t shstrndx; /* section header string table index. */
    tapi_dyna_t* phdrs; /* dynamic array of elf_phdr_t*. */
    tapi_dyna_t* shdrs; /* dynamic array of elf_shdr_t*. */
    char* shstrtab; /* section header string table. */
    size_t shstrtab_size; /* size of shstrtab. */
    char* path; /* path to the elf file. */
} elf_t;

/**
 * @brief parse an elf file from a file path.
 *
 * @param path the path to the elf file.
 * @return a pointer to an allocated elf_t if successful, 0x0 o.w.
 */
elf_t*
elf_parse(const char* path);

/**
 * @brief free an elf structure and all of its data.
 *
 * @param elf the elf structure to be freed.
 */
void
elf_free(elf_t* elf);

/**
 * @brief get the name of a section header.
 *
 * @param elf the elf structure.
 * @param shdr the section header.
 * @return a pointer to the name if successful, 0x0 o.w.
 */
const char*
elf_shdr_name(elf_t* elf, elf_shdr_t* shdr);
#endif /* __linux__ */
#endif /* ELF_H */