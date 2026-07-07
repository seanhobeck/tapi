/**
 * @author Sean Hobeck
 * @date 2026-07-07
 */
#define _GNU_SOURCE /*! needed for dl_iterate_phdr. */
#include "plt.h"

/*! uses fprintf. */
#include <stdio.h>

/*! uses strcmp. */
#include <string.h>

/*! uses calloc, free. */
#include <stdlib.h>

/*! uses bool, true, false. */
#include <stdbool.h>

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

#ifdef __gnu_linux__
/* the elf base address. */
internal uintptr_t elf_address;

/* first object enumerated is the main program; dlpi_addr is its load bias. */
internal int
elf_baddr_callback(struct dl_phdr_info* info, size_t size, void* data) {
    elf_address = info->dlpi_addr;
    return 1u; /* stop after the first. */
}
#endif

/** @brief fill the hashmap corresponding a name of a procedure to its stub within the plt. */
void
plt_init(void) {
#ifdef __gnu_linux__
    /* only needs to be done once per process, not a per context type of thing. */
    dl_iterate_phdr(elf_baddr_callback, 0x0); /* get the elf base address. */
    elf_t* elf = elf_parse("/proc/self/exe");
    plt_table = tapi_dyna_create();

    /* find the .dynstr, .dynsym, etc... section header locations and read them. */
    elf_shdr_t* dynsym = 0x0, *dynstr = 0x0, *rela = 0x0, *plt = 0x0;
    dyna_foreach(elf->shdrs, elf_shdr_t*, iter)
        /* for dynamic (PIE) symbols and string associations. */
        if (!strcmp(".dynsym", elf_shdr_name(elf, iter))) dynsym = iter;
        if (!strcmp(".dynstr", elf_shdr_name(elf, iter))) dynstr = iter;

        /* rela.plt and plt.sec section headers. */
        if (!strcmp(".rel.plt", elf_shdr_name(elf, iter)) \
            || !strcmp(".rela.plt", elf_shdr_name(elf, iter))) rela = iter;
        if (!strcmp(".plt.sec", elf_shdr_name(elf, iter)) \
            || !strcmp(".plt", elf_shdr_name(elf, iter))) plt = iter;
    dyna_endforeach(elf->shdrs);

    /* if we didn't find the section headers then we can't continue. */
    if (!dynsym || !dynstr || !rela || !plt) {
        fprintf(stderr, "tapi, pltr_init; could not find section headers, are you in debug?\n");
        return;
    }

    /* allocate for the symbols. */
    void* dynsym_data = calloc(1u, dynsym->sh_size);
    void* dynstr_data = calloc(1u, dynstr->sh_size);
    void* rela_data = calloc(1u, rela->sh_size);
    void* plt_data = calloc(1u, plt->sh_size);
    if (!dynsym_data || !dynstr_data || !rela_data || !plt_data) {
        free(dynsym_data);
        free(dynstr_data);
        free(rela_data);
        free(plt_data);
        fprintf(stderr, "tapi, pltr_init; could not allocate memory for section hdr data.\n");
        return;
    }

    /* open the elf executable, read the .dynstr and .dynsym data at once. */
    FILE* fptr = fopen(elf->path, "rb");
    if (!fptr) {
        free(dynsym_data);
        free(dynstr_data);
        free(rela_data);
        free(plt_data);
        fprintf(stderr, "pltr_init; could not open file %s!\n", elf->path);
        return;
    }
    fseek(fptr, dynsym->sh_offset, SEEK_SET);
    fread(dynsym_data, 1u, dynsym->sh_size, fptr);
    fseek(fptr, dynstr->sh_offset, SEEK_SET);
    fread(dynstr_data, 1u, dynstr->sh_size, fptr);
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
        if ((uintptr_t)&plt_init & 1u) /* are we in thumb-mode. */ {
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
        elf64_sym_t* sym = (elf64_sym_t*) dynsym_data;
        size_t num_rel = rela->sh_size / rela->sh_entsize;

        /* are we iterating over plt.sec? if so ignore plt0. */
        uintptr_t plt_address = plt->sh_addr;
        bool is_pltsec = !strcmp(".plt.sec", elf_shdr_name(elf, plt));

        /* iterate... */
        for (size_t i = 0u; i < num_rel; i++) {
            uint32_t sym_idx = ELF64_R_SYM(rel[i].r_info);
            if (sym_idx == 0) continue;

            /* calculate the name for the symbol. */
            char* name = (char*)dynstr_data + sym[sym_idx].st_name;
            if (!name || name[0] == '\0' || name[0] == '_') continue;
            if (strstr(name, "tapi_")) continue; /* we want to ignore tapi symbols. */

            /* calculate the associated stub address. */
            uintptr_t offset = plt0_size + plt_size * i;
            if (is_pltsec) offset -= plt0_size;
            void* assoc_addr = (void*)(elf_address + plt_address + offset);

            /* create a plt entry and add it. */
            pltr_asc_t* entry = calloc(1u, sizeof *entry);
            entry->name = name;
            entry->address = assoc_addr;
            tapi_dyna_push(plt_table, entry);
        }
    }
    else if (elf->class == ELF_CLASS_32) {
        elf32_rela_t* rel = (elf32_rela_t*) rela_data;
        elf32_sym_t* sym = (elf32_sym_t*) dynsym_data;
        size_t num_rel = rela->sh_size / rela->sh_entsize;
        uintptr_t plt_address = plt->sh_addr;

        /* ... */
        /* iterate... */
        for (size_t i = 0u; i < num_rel; i++) {
            uint32_t sym_idx = ELF32_R_SYM(rel[i].r_info);
            if (sym_idx == 0) continue;

            /* calculate the name for the symbol. */
            char* name = (char*)dynstr_data + sym[sym_idx].st_name;
            if (!name || name[0] == '\0' || name[0] == '_') continue;
            if (strstr(name, "tapi_")) continue; /* we want to ignore tapi symbols. */

            /* calculate the associated stub address. */
            uintptr_t offset = plt0_size + plt_size * i;
            void* assoc_addr = (void*)(elf_address + plt_address + offset);

            /* create a plt entry and add it. */
            pltr_asc_t* entry = calloc(1u, sizeof *entry);
            entry->name = name;
            entry->address = assoc_addr;
            tapi_dyna_push(plt_table, entry);
        }
    }

    /* free everything. */
    elf_free(elf);
    free(dynsym_data);
    free(dynstr_data);
    free(rela_data);
    free(plt_data);
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
plt_resolve(const char* name) {
    dyna_foreach(plt_table, pltr_asc_t*, iter)
        if (!strcmp(name, iter->name)) return iter->address;
    dyna_endforeach(plt_table)
    return 0x0;
};