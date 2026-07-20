/**
 * @author Sean Hobeck
 * @date 2026-07-16
 */
#include "elf.h"

/*! uses fprintf, stderr, fopen, fclose, fread, fseek, ftell. */
#include <stdio.h>

/*! uses calloc, malloc, free. */
#include <stdlib.h>

/*! uses memcpy, strncpy. */
#include <string.h>

/*! uses bool, true, false. */
#include <stdbool.h>

/*! uses internal. */
#include "intt.h"

/** elf magic number. */
#define ELF_MAGIC 0x464c457fu /* 0x7f 'E' 'L' 'F' */

/** elf identification indices. */
#define EI_MAG0 0u      /* magic number byte 0. */
#define EI_MAG1 1u      /* magic number byte 1. */
#define EI_MAG2 2u      /* magic number byte 2. */
#define EI_MAG3 3u      /* magic number byte 3. */
#define EI_CLASS 4u     /* file class. */
#define EI_DATA 5u      /* data encoding. */
#define EI_VERSION 6u   /* file version. */
#define EI_NIDENT 16u   /* size of e_ident[]. */

/** @brief a generic elf32 header. */
typedef struct {
    uint8_t e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) elf32_ehdr_t;

/** @brief a generic elf64 header. */
typedef struct {
    uint8_t e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) elf64_ehdr_t;

/**
 * @brief check if an elf identifier is valid.
 *
 * @param ident the elf identifier.
 * @return if the identifier is valid.
 */
internal e_intt_result_t
elf_valid_ident(uint8_t* ident) {
    return ident[EI_MAG0] == 0x7fu && ident[EI_MAG1] == 'E' &&
           ident[EI_MAG2] == 'L' && ident[EI_MAG3] == 'F';
}

/**
 * @brief parse an elf32 file from a buffer.
 *
 * @param buffer the buffer containing the elf file.
 * @param size the size of the buffer.
 * @return a pointer to an allocated elf_t if successful, 0x0 o.w.
 */
internal elf_t*
elf_parse32(uint8_t* buffer, size_t size) {
    elf32_ehdr_t* ehdr = (elf32_ehdr_t*)buffer;
    
    if (size < sizeof *ehdr) {
        fprintf(stderr, "tapi, elf_parse32; buffer too small for elf32 header.\n");
        return 0x0;
    }

    /* allocate our elf structure. */
    elf_t* elf = calloc(1u, sizeof *elf);
    elf->class = ELF_CLASS_32;
    elf->data = ehdr->e_ident[EI_DATA];
    elf->type = ehdr->e_type;
    elf->machine = ehdr->e_machine;
    elf->entry = ehdr->e_entry;
    elf->phoff = ehdr->e_phoff;
    elf->shoff = ehdr->e_shoff;
    elf->phnum = ehdr->e_phnum;
    elf->shnum = ehdr->e_shnum;
    elf->shstrndx = ehdr->e_shstrndx;
    elf->phdrs = tapi_dyna_create();
    elf->shdrs = tapi_dyna_create();

    /* parse program headers. */
    if (elf->phnum > 0 && elf->phoff + elf->phnum * sizeof(elf32_phdr_t) <= size) {
        for (uint16_t i = 0; i < elf->phnum; i++) {
            elf32_phdr_t* phdr32 = (elf32_phdr_t*)(buffer + elf->phoff + i * sizeof(elf32_phdr_t));
            elf_phdr_t* phdr = calloc(1u, sizeof *phdr);
            phdr->p_type = phdr32->p_type;
            phdr->p_flags = phdr32->p_flags;
            phdr->p_offset = phdr32->p_offset;
            phdr->p_vaddr = phdr32->p_vaddr;
            phdr->p_paddr = phdr32->p_paddr;
            phdr->p_filesz = phdr32->p_filesz;
            phdr->p_memsz = phdr32->p_memsz;
            phdr->p_align = phdr32->p_align;
            tapi_dyna_push(elf->phdrs, phdr);
        }
    }

    /* parse section headers. */
    if (elf->shnum > 0 && elf->shoff + elf->shnum * sizeof(elf32_shdr_t) <= size) {
        for (uint16_t i = 0; i < elf->shnum; i++) {
            elf32_shdr_t* shdr32 = (elf32_shdr_t*)(buffer + elf->shoff + i * sizeof(elf32_shdr_t));
            elf_shdr_t* shdr = calloc(1u, sizeof *shdr);
            shdr->sh_name = shdr32->sh_name;
            shdr->sh_type = shdr32->sh_type;
            shdr->sh_flags = shdr32->sh_flags;
            shdr->sh_addr = shdr32->sh_addr;
            shdr->sh_offset = shdr32->sh_offset;
            shdr->sh_size = shdr32->sh_size;
            shdr->sh_link = shdr32->sh_link;
            shdr->sh_info = shdr32->sh_info;
            shdr->sh_addralign = shdr32->sh_addralign;
            shdr->sh_entsize = shdr32->sh_entsize;
            tapi_dyna_push(elf->shdrs, shdr);
        }

        /* parse section header string table. */
        if (elf->shstrndx < elf->shnum) {
            elf_shdr_t* shstrtab = dyna_get(elf->shdrs, elf_shdr_t*, elf->shstrndx);
            if (shstrtab && shstrtab->sh_offset + shstrtab->sh_size <= size) {
                elf->shstrtab_size = shstrtab->sh_size;
                elf->shstrtab = malloc(elf->shstrtab_size);
                memcpy(elf->shstrtab, buffer + shstrtab->sh_offset, elf->shstrtab_size);
            }
        }
    }
    return elf;
}

/**
 * @brief parse an elf64 file from a buffer.
 *
 * @param buffer the buffer containing the elf file.
 * @param size the size of the buffer.
 * @return a pointer to an allocated elf_t if successful, 0x0 o.w.
 */
internal elf_t*
elf_parse64(uint8_t* buffer, size_t size) {
    elf64_ehdr_t* ehdr = (elf64_ehdr_t*)buffer;
    if (size < sizeof *ehdr) {
        fprintf(stderr, "tapi, elf_parse64; buffer too small for elf64 header.\n");
        return 0x0;
    }

    /* allocate our elf structure. */
    elf_t* elf = calloc(1u, sizeof *elf);
    elf->class = ELF_CLASS_64;
    elf->data = ehdr->e_ident[EI_DATA];
    elf->type = ehdr->e_type;
    elf->machine = ehdr->e_machine;
    elf->entry = ehdr->e_entry;
    elf->phoff = ehdr->e_phoff;
    elf->shoff = ehdr->e_shoff;
    elf->phnum = ehdr->e_phnum;
    elf->shnum = ehdr->e_shnum;
    elf->shstrndx = ehdr->e_shstrndx;
    elf->phdrs = tapi_dyna_create();
    elf->shdrs = tapi_dyna_create();

    /* parse program headers. */
    if (elf->phnum > 0 && elf->phoff + elf->phnum * sizeof(elf64_phdr_t) <= size) {
        for (uint16_t i = 0; i < elf->phnum; i++) {
            elf64_phdr_t* phdr64 = (elf64_phdr_t*)(buffer + elf->phoff + i * sizeof(elf64_phdr_t));
            elf_phdr_t* phdr = calloc(1u, sizeof *phdr);
            memcpy(phdr, phdr64, sizeof *phdr);
            tapi_dyna_push(elf->phdrs, phdr);
        }
    }

    /* parse section headers. */
    if (elf->shnum > 0 && elf->shoff + elf->shnum * sizeof(elf64_shdr_t) <= size) {
        for (uint16_t i = 0; i < elf->shnum; i++) {
            elf64_shdr_t* shdr64 = (elf64_shdr_t*)(buffer + elf->shoff + i * sizeof(elf64_shdr_t));
            elf_shdr_t* shdr = calloc(1u, sizeof *shdr);
            memcpy(shdr, shdr64, sizeof *shdr);
            tapi_dyna_push(elf->shdrs, shdr);
        }

        /* parse section header string table. */
        if (elf->shstrndx < elf->shnum) {
            elf_shdr_t* shstrtab = dyna_get(elf->shdrs, elf_shdr_t*, elf->shstrndx);
            if (shstrtab && shstrtab->sh_offset + shstrtab->sh_size <= size) {
                elf->shstrtab_size = shstrtab->sh_size;
                elf->shstrtab = malloc(elf->shstrtab_size);
                memcpy(elf->shstrtab, buffer + shstrtab->sh_offset, elf->shstrtab_size);
            }
        }
    }
    return elf;
}

/**
 * @brief parse an elf file from a file path.
 *
 * @param path the path to the elf file.
 * @return a pointer to an allocated elf_t if successful, 0x0 o.w.
 */
elf_t*
elf_parse(const char* path) {
    /* open the file. */
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "tapi, elf_parse; could not open file %s.\n", path);
        return 0x0;
    }

    /* get the file size. */
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* read the file into a buffer. */
    uint8_t* buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "tapi, elf_parse; malloc failed; could not allocate memory for buffer.\n");
        fclose(file);
        return 0x0;
    }
    if (fread(buffer, 1u, size, file) != size) {
        fprintf(stderr, "tapi, elf_parse; fread failed; could not read file %s.\n", path);
        free(buffer);
        fclose(file);
        return 0x0;
    }
    fclose(file);

    /* check if the elf is valid. */
    if (size < EI_NIDENT || !e_intt_passed(elf_valid_ident(buffer))) {
        fprintf(stderr, "tapi, elf_parse; invalid elf file %s.\n", path);
        free(buffer);
        return 0x0;
    }

    /* parse the elf based on class. */
    elf_t* elf = 0x0;
    if (buffer[EI_CLASS] == ELF_CLASS_32) elf = elf_parse32(buffer, size);
    else if (buffer[EI_CLASS] == ELF_CLASS_64) elf = elf_parse64(buffer, size);
    else fprintf(stderr, "tapi, elf_parse; unsupported elf class %d.\n", buffer[EI_CLASS]);

    /* store path in elf structure. */
    if (elf) {
        size_t len = strlen(path);
        elf->path = calloc(1u, len + 1u);
        strncpy(elf->path, path, len);
    }

    free(buffer);
    return elf;
}

/**
 * @brief free an elf structure and all of its data.
 *
 * @param elf the elf structure to be freed.
 */
void
elf_free(elf_t* elf) {
    if (!elf) return;

    /* free program headers. */
    if (elf->phdrs) {
        dyna_foreach(elf->phdrs, elf_phdr_t*, phdr)
            free(phdr);
        dyna_endforeach(elf->phdrs);
        tapi_dyna_free(elf->phdrs);
    }

    /* free section headers. */
    if (elf->shdrs) {
        dyna_foreach(elf->shdrs, elf_shdr_t*, shdr)
            free(shdr);
        dyna_endforeach(elf->shdrs);
        tapi_dyna_free(elf->shdrs);
    }

    /* free string table. */
    free(elf->shstrtab);

    /* free path. */
    free(elf->path);
    free(elf);
}

/**
 * @brief get the name of a section header.
 *
 * @param elf the elf structure.
 * @param shdr the section header.
 * @return a pointer to the name if successful, 0x0 o.w.
 */
const char*
elf_shdr_name(elf_t* elf, elf_shdr_t* shdr) {
    if (!elf || !shdr || !elf->shstrtab|| !elf->shstrtab) return 0x0;
    if (elf->shstrtab_size == 0) return 0x0;
    if (shdr->sh_name >= elf->shstrtab_size) return 0x0;

    const char* s = elf->shstrtab + shdr->sh_name;
    size_t remaining = elf->shstrtab_size - shdr->sh_name;
    if (!memchr(s, 0, remaining)) return 0x0; /* no terminator in table. */
    return s;
}