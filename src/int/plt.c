/**
 * @author Sean Hobeck
 * @date 2026-06-28
 */
#include "plt.h"

/*! uses fprintf. */
#include <stdio.h>

/*! uses strcmp. */
#include <string.h>

/*! uses calloc, free. */
#include <stdlib.h>

#ifdef __gnu_linux__
/*! uses Elf32_Shdr, Elf32_Phdr. */
#include <linux/elf.h>
#endif

/*! uses elf_t, elf_parse, ... */
#include "elf.h"

/*! uses internal. */
#include "intt.h"

/* elf/(mach-o/peb wip) */
#ifdef __gnu_linux__
internal elf_t* elf;
#endif

/* an internal list for all of the plt entries. */
internal tapi_dyna_t* plt_table;

/** @brief fill the hashmap corresponding a name of a procedure to its stub within the plt. */
void
plt_init(void) {
    /* only needs to be done once per process, not a per context type of thing. */
#ifdef __gnu_linux__
    elf = elf_parse("/proc/self/exe");
    plt_table = tapi_dyna_create();

    /* find the .symtab and .strtab section header locations and read them. */
    elf_shdr_t* symtab, *strtab;
    dyna_foreach(elf->shdrs, elf_shdr_t*, iter)
        if (!strcmp(".symtab", elf_shdr_name(elf, iter))) symtab = iter;
        if (!strcmp(".strtab", elf_shdr_name(elf, iter))) strtab = iter;
    dyna_endforeach(elf->shdrs);

    /* if we didn't find the symtab or strtab sections then we can't continue. */
    if (!symtab || !strtab) {
        fprintf(stderr, "tapi, plt_init; could not find .symtab and .strtab!\n");
        return;
    }

    /* allocate for the symbols. */
    void* symtab_data = calloc(1u, symtab->sh_size);
    void* strtab_data = calloc(1u, strtab->sh_size);
    if (!symtab_data || !strtab_data) {
        free(symtab_data);
        free(strtab_data);
        fprintf(stderr, "tapi, plt_init; could not allocate memory for .symtab/.strtab data.\n");
        return;
    }

    /* open the elf executable, read the .symtab and .strtab data at once. */
    FILE* fptr = fopen(elf->path, "rb");
    if (!fptr) {
        free(symtab_data);
        free(strtab_data);
        fprintf(stderr, "plt_init; could not open file %s!\n", elf->path);
        return;
    }
    fseek(fptr, symtab->sh_offset, SEEK_SET);
    fread(symtab_data, 1u, symtab->sh_size, fptr);
    fseek(fptr, strtab->sh_offset, SEEK_SET);
    fread(strtab_data, 1u, strtab->sh_size, fptr);
    fclose(fptr);

    /* iterate... */
    size_t number_symbols = symtab->sh_size / symtab->sh_entsize;
    if (elf->class == ELF_CLASS_32) {
        elf32_sym_t* sym = symtab_data;
        for (size_t i = 0u; i < number_symbols; i++) {
            uint8_t type = sym[i].st_info & 0xf; /* checking if it is a function symbol. */
            if (type == 2u && sym[i].st_value != 0u) {
                /* get the name, if it is null or a reserved name, ignore it. */
                char* name = strtab_data + sym[i].st_name;
                if (!name || name[0] == '\0' || name[0] == '_') continue;

                /* create a plt entry and add it. */
                plt_entry_t* entry = calloc(1u, sizeof *entry);
                entry->name = name;
                entry->address = (void*) sym[i].st_value;
            }
        }
    } else if (elf->class == ELF_CLASS_64) {
        elf64_sym_t* sym = symtab_data;
        for (size_t i = 0u; i < number_symbols; i++) {
            uint8_t type = sym[i].st_info & 0xf; /* checking if it is a function symbol. */
            if (type == 2u && sym[i].st_value != 0u) {
                /* get the name, if it is null or a reserved name, ignore it. */
                char* name = strtab_data + sym[i].st_name;
                if (!name || name[0] == '\0' || name[0] == '_') continue;

                /* create a plt entry and add it. */
                plt_entry_t* entry = calloc(1u, sizeof *entry);
                entry->name = name;
                entry->address = (void*) sym[i].st_value;
            }
        }
    }
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
plt_resolve_by_name(const char* name) {

};