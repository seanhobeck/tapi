/**
 * @author Sean Hobeck
 * @date 2026-09-05
 */
#define _GNU_SOURCE /*! needed for dl_iterate_phdr. */
#include "lnk.h"

/*! uses fprintf. */
#include <stdio.h>

/*! uses strcmp. */
#include <string.h>

/*! uses calloc, free. */
#include <stdlib.h>

/*! uses bool, true, false. */
#include <stdbool.h>

#ifdef TAPI_LINUX
/*! uses EM_..., ELF32/64_R_SYM/TYPE. */
#include <linux/elf.h>
#include <link.h>
#endif

/*! uses cs_insn, cs_disas. */
#include <capstone/capstone.h>

/*! uses elf_t, elf_parse, ... */
#include "elf.h"

/*! uses internal. */
#include "intt.h"

/*! uses map_t, ... */
#include "map.h"

/*! uses sig_compare. */
#include "sig.h"

/* an internal list for all the plt entries. */
internal map_t* table;

#ifdef TAPI_LINUX
/* the elf base address. */
internal uintptr_t elf_address;

/* first object enumerated is the main program; dlpi_addr is its load bias. */
internal int
elf_baddr_callback(struct dl_phdr_info* info, __attribute__((unused)) size_t size, \
    __attribute__((unused))  void* data) {
    elf_address = info->dlpi_addr;
    return 1u; /* stop after the first. */
}

/** information about the plt0 and plt entry sizes for each architecture. */
typedef struct {
    size_t entry, first;
} plti_t;

/**
 * @brief find more information about the plt (procedural linkage table).
 *
 * @param machine the elf machine type.
 * @param plt0_address the address of the first plt entry.
 * @param data the plt section data.
 * @return a struct containing information about the plt.
 */
internal plti_t*
find_plt_sizes(elf_mach_t machine, uintptr_t plt0_address, uint8_t* data) {
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
    plti_t* info = calloc(1u, sizeof *info);
    switch (machine) {
        /* i386 and x86_64 are the same. */
        case EM_X86_64:
        case EM_386:
            /* x86_64 plt0 can change apparently, not from what i have seen though.
             *  ref: https://groups.google.com/g/x86-64-abi/c/oi3i85b1uI0?pli=1. */
            info->entry = info->first = 16u;
            break;
            /* aarch64 is 32 for plt0 and 16. */
        case EM_AARCH64:
            info->first = 32u;
            info->entry = 16u;
            break;
        case EM_ARM: {
            info->first = 20u;

            /* the only way we can detect plt sizes is by inspecting the first entry, NOT plt0. */
            csh arm, thumb;
            cs_open(CS_ARCH_ARM, CS_MODE_ARM, &arm);
            cs_option(arm, CS_OPT_DETAIL, CS_OPT_ON);
            cs_open(CS_ARCH_ARM, CS_MODE_THUMB, &thumb);
            cs_option(thumb, CS_OPT_DETAIL, CS_OPT_ON);
            cs_insn* insn = 0x0, *thumb_insn = 0x0;
            size_t arm_count = 0u, thumb_count = 0u;

            /* disassemble the first 4 bytes for arm (checking for bx lr, nop), and then try arm-mode. */
            uintptr_t start = elf_address + plt0_address + info->first;
            thumb_count = cs_disasm(thumb, data, 4u, start, 0u, &thumb_insn);
            arm_count = cs_disasm(arm, data, 12u, start, 0u, &insn);
            if (thumb_count == 2u) {
                /* if bx lr, nop, then we are dealing with a 16-byte thumb entry. */
                if (sig_compare(ARMHF_PLT1, &thumb_insn[0]) && \
                    sig_compare(ARMHF_PLT2, &thumb_insn[1]))
                    info->entry = 16u;
            }
            /* if we are matching the arm literal entry type (ldr, add, .word). */
            if (arm_count == 2u) {
                if (sig_compare(ARM32L_PLT1, &insn[0]) && \
                    sig_compare(ARM32L_PLT2, &insn[1]))
                    info->entry = 16u;
                else fprintf(stderr, "find_plt_sizes; unknown arm plt entry, doesn't match rare arm32 literal?");
            }
            else if (arm_count == 3u) {
                if (sig_compare(ARM32_PLT1, &insn[0]) && \
                    sig_compare(ARM32_PLT2, &insn[1]) && \
                    sig_compare(ARM32_PLT3, &insn[2]))
                    info->entry = 12u;
                else fprintf(stderr, "find_plt_sizes; unknown arm plt entry, doesn't match arm32?");
            }
            else fprintf(stderr, "find_plt_sizes; unknown arm plt entry, doesn't match thumb?");
            cs_close(&arm);
            cs_close(&thumb);
            cs_free(insn, arm_count);
            cs_free(thumb_insn, thumb_count);
            break;
        }
        default:
            fprintf(stderr, "find_plt_sizes; unsupported arch.\n");
            break;
    }
    return info;
};
#endif

/** @brief fill the hashmap corresponding a name of a procedure to its stub within the plt. */
void
lnk_init(void) {
#ifdef TAPI_LINUX
    /* only needs to be done once per process, not a per context type of thing. */
    dl_iterate_phdr(elf_baddr_callback, 0x0); /* get the elf base address. */
    elf_t* elf = elf_parse("/proc/self/exe");
    table = map_make();

    /* find the .dynstr, .dynsym, etc... section header locations and read them. */
    bool is_rel = false;
    elf_shdr_t* dynsym = 0x0, *dynstr = 0x0, *rela = 0x0, *plt = 0x0;
    dyna_foreach(elf->shdrs, elf_shdr_t*, iter)
        /* for dynamic (PIE) symbols and string associations. */
        const char* shdr_name = elf_shdr_name(elf, iter);
        if (shdr_name == 0x0) continue;
        if (!strcmp(".dynsym", shdr_name))
            dynsym = iter;
        if (!strcmp(".dynstr", shdr_name))
            dynstr = iter;

        /* rela.plt and plt.sec section headers. */
        bool is_relplt = !strcmp(".rel.plt", shdr_name);
        if (is_relplt || !strcmp(".rela.plt", shdr_name)) {
            is_rel = is_relplt;
            rela = iter;
        }
        if (!strcmp(".plt.sec", shdr_name) || !strcmp(".plt", shdr_name))
            plt = iter;
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

    /* get more info about the plt. */
    plti_t* info = find_plt_sizes(elf->machine, plt->sh_addr, (uint8_t*)plt_data);

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
            uintptr_t offset = info->first + info->entry * i;
            if (is_pltsec) offset -= info->first;
            void* assoc_addr = (void*)(elf_address + plt_address + offset);

            /* create a plt entry and add it. */
            map_push(table, name, assoc_addr);
        }
    }
    else if (elf->class == ELF_CLASS_32) {
        elf32_sym_t* sym = (elf32_sym_t*) dynsym_data;
        size_t num_rel = rela->sh_size / rela->sh_entsize;
        uintptr_t plt_address = plt->sh_addr;

        /* iterate... */
        if (is_rel) {
            elf32_rel_t* rel = (elf32_rel_t*) rela_data;
            for (size_t i = 0u; i < num_rel; i++) {
                uint32_t sym_idx = ELF32_R_SYM(rel[i].r_info);
                if (sym_idx == 0) continue;

                /* calculate the name for the symbol. */
                char* name = (char*)dynstr_data + sym[sym_idx].st_name;
                if (!name || name[0] == '\0' || name[0] == '_') continue;
                if (strstr(name, "tapi_")) continue; /* we want to ignore tapi symbols. */

                /* calculate the associated stub address. */
                uintptr_t offset = info->first + info->entry * i;
                void* assoc_addr = (void*)(elf_address + plt_address + offset);

                /* create a plt entry and add it. */
                map_push(table, name, assoc_addr);
            }
        }
        else {
            elf32_rela_t* rel = (elf32_rela_t*) rela_data;
            for (size_t i = 0u; i < num_rel; i++) {
                uint32_t sym_idx = ELF32_R_SYM(rel[i].r_info);
                if (sym_idx == 0) continue;

                /* calculate the name for the symbol. */
                char* name = (char*)dynstr_data + sym[sym_idx].st_name;
                if (!name || name[0] == '\0' || name[0] == '_') continue;
                if (strstr(name, "tapi_")) continue; /* we want to ignore tapi symbols. */

                /* calculate the associated stub address. */
                uintptr_t offset = info->first + info->entry * i;
                void* assoc_addr = (void*)(elf_address + plt_address + offset);

                /* create a plt entry and add it. */
                map_push(table, name, assoc_addr);
            }
        }
    }

    /* free everything. */
    elf_free(elf);
    free(dynsym_data);
    free(dynstr_data);
    free(rela_data);
    free(plt_data);
#elif defined(TAPI_WINDOWS)
    /* create the map. */
    table = map_make();

    /* get the base module, dos and nt headers. */
    PBYTE base_module = (PBYTE) GetModuleHandleA(0x0);
    PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)(base_module);
    PIMAGE_NT_HEADERS nt_header = (PIMAGE_NT_HEADERS)(base_module + dos_header->e_lfanew);
    IMAGE_DATA_DIRECTORY directory = nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!directory.VirtualAddress) {
        /* NOLINTNEXTLINE */
        fprintf_s(stderr, "tapi, lnk_init; could not resolve iat virtual address\n");
        return;
    }

    /* iterate... */
    PIMAGE_IMPORT_DESCRIPTOR desc_iter = (PIMAGE_IMPORT_DESCRIPTOR)(base_module + directory.VirtualAddress);
    for (; desc_iter->Name; ++desc_iter) {

        /* we need to calculate the original thunk address, sometimes desc_iter->FirstThunk == 0. */
        PIMAGE_THUNK_DATA thunk_orig = (PIMAGE_THUNK_DATA)(base_module + desc_iter->OriginalFirstThunk),
            thunk_first = (PIMAGE_THUNK_DATA)(base_module + desc_iter->FirstThunk);
        if (!thunk_orig) thunk_orig = thunk_first;
        for (; thunk_orig->u1.AddressOfData; ++thunk_orig, ++thunk_first) {
            if (IMAGE_SNAP_BY_ORDINAL(thunk_orig->u1.Ordinal)) /* ordinal number instead of fun name. */
                continue;

            /* get the actual import, then make a entry within our lookup table. */
            PIMAGE_IMPORT_BY_NAME import = (PIMAGE_IMPORT_BY_NAME)(base_module + thunk_orig->u1.AddressOfData);
            map_push(table, import->Name, &thunk_first->u1.Function);
        }
    }
#elif defined(TAPI_UNIX)
#error "tapi does not support macos/apple for automock resolving."
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
lnk_resolve(const char* name) {
    entry_t* entry = map_lookup(table, name);
    if (entry == 0x0)
        return 0x0;
    return entry->value;
};

#ifdef TAPI_WINDOWS
/*! uses get_arch. */
#include "arch.h"

/**
 * @brief quickly resolve a windows thunk.
 *
 * @param address the address of the thunk.
 * @return the actual address of what is being called at the thunk.
 */
void*
lnk_qr_thunk(void* address) {
    /* to resolve these incremental thunks, we simply evaluate the address of the jmp. */
    csh handle;
    arch_t arch = get_arch(); /* we don't need to worry about arm32th (thank god). */
    cs_open(arch.arch, arch.mode, &handle);
    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
    cs_insn* insn = cs_malloc(handle);
    if (!insn) {
        cs_close(&handle);
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi, lnk_qr_thunk; cs_malloc failed; could not allocate memory for "
            "instructions.");
        return 0;
    }
    
    /* disassemble just the jump (e9 rel32). */
    const uint8_t* bytes = (uint8_t*)address;
    cs_disasm(handle, bytes, 5u, (uintptr_t)address, 1u, &insn);
    if (!insn->op_str) return 0x0;
    uintptr_t value = strtoll((char*)(insn->op_str + 2u), 0x0, 16u);
    return value;
};
#endif  

/** @brief clean up the internal plt_table. */
void
lnk_cleanup(void) {
    map_cleanup(table);
};