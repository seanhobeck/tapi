/**
 * @author Sean Hobeck
 * @date 2026-06-28
 */
#include "pltr.h"

/*! uses fprintf. */
#include <stdio.h>

/*! uses strcmp. */
#include <string.h>

/*! uses calloc, free. */
#include <stdlib.h>

#ifdef __gnu_linux__
/*! uses EM_..., ELF32/64_R_SYM/TYPE. */
#include <linux/elf.h>
#include <link.h>
#endif

/*! uses elf_t, elf_parse, ... */
#include "elf.h"

/*! uses internal. */
#include "intt.h"

/* an internal list for all of the plt entries. */
internal tapi_dyna_t* plt_table;

/** @brief fill the hashmap corresponding a name of a procedure to its stub within the plt. */
void
pltr_init(void) {
    /* only needs to be done once per process, not a per context type of thing. */
#ifdef __gnu_linux__
    elf_t* elf = elf_parse("/proc/self/exe");
    plt_table = tapi_dyna_create();

    /* find the .symtab, .strtab, etc... section header locations and read them. */
    elf_shdr_t* symtab, *strtab, *rela, *plt;
    dyna_foreach(elf->shdrs, elf_shdr_t*, iter)
        if (!strcmp(".dynsym", elf_shdr_name(elf, iter))) symtab = iter;
        if (!strcmp(".dynstr", elf_shdr_name(elf, iter))) strtab = iter;
        if (!strcmp(".rel.plt", elf_shdr_name(elf, iter)) \
            || !strcmp(".rela.plt", elf_shdr_name(elf, iter))) rela = iter;
        if (!strcmp(".plt", elf_shdr_name(elf, iter))) plt = iter;
    dyna_endforeach(elf->shdrs);

    /* if we didn't find the section headers then we can't continue. */
    if (!symtab || !strtab || !rela || !plt) {
        fprintf(stderr, "tapi, pltr_init; could not find section headers, are you in debug?\n");
        return;
    }

    /* allocate for the symbols. */
    void* symtab_data = calloc(1u, symtab->sh_size);
    void* strtab_data = calloc(1u, strtab->sh_size);
    void* rela_data = calloc(1u, rela->sh_size);
    void* plt_data = calloc(1u, plt->sh_size);
    if (!symtab_data || !strtab_data || !rela_data || !plt_data) {
        free(symtab_data);
        free(strtab_data);
        fprintf(stderr, "tapi, pltr_init; could not allocate memory for section hdr data.\n");
        return;
    }

    /* open the elf executable, read the .symtab and .strtab data at once. */
    FILE* fptr = fopen(elf->path, "rb");
    if (!fptr) {
        free(symtab_data);
        free(strtab_data);
        fprintf(stderr, "pltr_init; could not open file %s!\n", elf->path);
        return;
    }
    fseek(fptr, symtab->sh_offset, SEEK_SET);
    fread(symtab_data, 1u, symtab->sh_size, fptr);
    fseek(fptr, strtab->sh_offset, SEEK_SET);
    fread(strtab_data, 1u, strtab->sh_size, fptr);
    fseek(fptr, rela->sh_offset, SEEK_SET);
    fread(rela_data, 1u, rela->sh_size, fptr);
    fseek(fptr, plt->sh_offset, SEEK_SET);
    fread(plt_data, 1u, plt->sh_size, fptr);
    fclose(fptr);

    /* each architecture has a different size in entry for each stub within .plt,
     *  in order to iterate through the entire section we have to know the size of
     *  the first index (plt0) and the size of the succeeding entries, this varies
     *  and if you want to learn more about it, i suggest reading some of the articles
     *  linked below, they are slightly helpful. there are some edge cases for these
     *  numbers though, ie. bti/pac for aarch64, or even armth and other arm32 eabi's
     *  (which we support) which makes this problem significantly more difficult.
     *
     * ref:
     *  https://maskray.me/blog/2021-09-19-all-about-procedure-linkage-table
     *  https://sourceware.org/pipermail/binutils/2002-July/020953.html
     *  https://github.com/espressif/binutils-esp32ulp/blob/master/bfd/elf32-i386.c#L546C1-L546C26
     *  https://chromium.googlesource.
     *  com/native_client/nacl-toolchain/%2B/6d9e9956805cda3fe461198675ee0bc7bd8ded4b/binutils/bfd/elf64-x86-64.c#380
     */
    size_t plt_size = 16u, plt0_size = 16u;
    switch (elf->machine) {
    /* i386 and x86_64 are the same. */
    case EM_X86_64:
    case EM_386:
        /* x86_64 plt0 can change apparently, not from what i have seen though.
         *  ref: https://groups.google.com/g/x86-64-abi/c/oi3i85b1uI0?pli=1. */
        plt_size = plt0_size = 16u;
        break;
    /* aarch64 is 32 for plt0 and 16. */
    case EM_AARCH64:
        plt0_size = 32u;
        plt_size = 16u;
        break;
    /* thumb is different like i said above. */
    case EM_ARM:
        plt0_size = 20u;
        if ((uintptr_t)&pltr_init & 1u) /* are we in thumb-mode. */ {
            plt_size = 20u;
        } else plt_size = 16u;
        break;
    default:
        fprintf(stderr, "tapi, pltr_init; unsupported arch.\n");
        break;
    }

    /* for each class. */
    if (elf->class == ELF_CLASS_64) {
        elf64_rela_t* rel = (elf64_rela_t*) rela_data;
        elf64_sym_t* sym = (elf64_sym_t*) symtab_data;
        size_t num_rel = rela->sh_size / rela->sh_entsize;

        /* iterate... */
        for (size_t i = 0u; i < num_rel; i++) {
            uint32_t sym_idx = ELF64_R_SYM(rel[i].r_info);
            uint32_t type = ELF64_R_TYPE(rel[i].r_info);
            if (sym_idx == 0) continue;

            /* calculate the name for the symbol. */
            char* name = (char*)strtab_data + sym[sym_idx].st_name;
            if (!name || name[0] == '\0' || name[0] == '_') continue;
            if (!strstr("tapi_", name)) continue;

            /* calculate the stub address. */
            void* plt_address = (void*)(plt->sh_addr + plt0_size + (plt_size * i));

            /* create a plt entry and add it. */
            pltr_asc_t* entry = calloc(1u, sizeof *entry);
            entry->name = name;
            entry->address = plt_address;
            tapi_dyna_push(plt_table, entry);
        }

    } else if (elf->class == ELF_CLASS_32) {
        elf32_rela_t* rel = (elf32_rela_t*) rela_data;
        elf32_sym_t* sym = (elf32_sym_t*) symtab_data;
        size_t num_rel = rela->sh_size / rela->sh_entsize;
        /* ... */
    }
    elf_free(elf);
#endif
};

/**
 * @brief resolve a library/system call's plt stub address. this is done via searching through
 *  the processes elf/peb/mach-o headers to read through a generated plt table.
 *
 * @param name the name of the given library/system call.
 * @return an address corresponding to the plt stub function, or 0x0 o.w.
 */
void*
pltr_resolve(const char* name) {

};