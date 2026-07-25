/**
 * @author Sean Hobeck
 * @date 2026-07-20
 */
#ifdef __gnu_linux__
#define _DEFAULT_SOURCE /* required for htole16/32/64. */
#endif
#include "reloc.h"

/*! uses assert. */
#include <assert.h>

#ifdef _WIN32
/*! uses virtualalloc. */
#include <windows.h>
#else
/*! uses mmap, munmap. */
#include <sys/mman.h>
#endif

/*! uses internal, get_page_size, etc... */
#include "intt.h"

/*! uses guard_make, etc... */
#include "guard.h"

/* max distance for a reloc. */
#if defined(__amd64__) || defined(__i386__)
#define MAX_DISTANCE 0x7fffffff
#else
#ifdef __aarch64__
#define MAX_DISTANCE 0x8000000
#elif __arm__
#define MAX_DISTANCE 0x2000000
#endif
#endif

/*! uses calloc, free, etc... */
#include <stdlib.h>

/*! uses snprintf. */
#include <stdio.h>

/*! uses memcpy. */
#include <string.h>

/*! uses map_t, etc... */
#include "map.h"

#ifdef __gnu_linux__
/*! uses htole16/32/64. */
#include <endian.h>
#elif defined(__APPLE__)
/*! uses OSSwapHostToLittleInt16/32/64. */
#include <libkern/OSByteOrder.h>

/* redef. */
#define htole16 OSSwapHostToLittleInt16
#define htole32 OSSwapHostToLittleInt32
#define htole64 OSSwapHostToLittleInt64
#define le16toh OSSwapLittleToHostInt16
#define le32toh OSSwapLittleToHostInt32
#define le64toh OSSwapLittleToHostInt64
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
/*! uses htole16/32/64. */
#include <sys/endian.h>
#elif defined(_WIN32)
/*! uses uint16/32/64_t. */
#include <stdint.h>

/* little endian on all archs. */
#define htole16(x) ((uint16_t)(x))
#define htole32(x) ((uint32_t)(x))
#define htole64(x) ((uint64_t)(x))
#define le16toh(x) ((uint16_t)(x))
#define le32toh(x) ((uint32_t)(x))
#define le64toh(x) ((uint64_t)(x))
#else
#endif

/**
 * @brief is it possible to reach the given address from the starting within the given
 *  bytes of a relative call?
 *
 * @param from the start of the call instruction from which to call from (ip/pc).
 * @param to the address we are trying to make a relative call to.
 * @return if its possible to make a call to the given address in the number '
 *  of bytes within the relative call instruction given per architecture.
 */
internal e_intt_result_t
rel_range_chk(const void* from, const uintptr_t to) {
#if defined(__amd64__) || defined(__i386__)
    /* calculate the displacement. */
    int64_t displacement = (int64_t)to - ((int64_t)from + 5ll); /*! assuming e8. */
    return displacement >= INT32_MIN && displacement<= INT32_MAX;
#else
    /* needs to be 4-byte aligned, not sure if mmap or virtualalloc handles this? */
    if (((uintptr_t)to & 3l) != 0x0) return false;
#ifdef __aarch64__
    /* calculate the displacement. */
    int64_t displacement = (int64_t)to - (int64_t)from;
    if (displacement & 3l != 0x0) return false;
    int64_t imm26 = displacement / 4ll;
    return imm26 >= -(1ll << 25ll) && imm26 <= (1ll << 25ll) - 1ll;
#elif __arm__
    /* calculate the displacement. */
    uintptr_t from_ptr = (uintptr_t)from;
    from_ptr &= ~(uintptr_t)1u;
    int64_t displacement = (int64_t)to - ((int64_t)from_ptr + 8ll);
    if ((displacement & 1ll) != 0x0) return false;
    return displacement >= -0x2000000 && displacement <= 0x1fffffc;
#endif
#endif
}

/**
 * @brief allocate a region of bytes near a target address.
 *
 * @param target the target address to allocate a region near.
 * @param size the size of the region needed to be allocated.
 * @return a region of size bytes ready to be used as a jump, 0x0 o.w.
 */
internal void*
alloc_region(const void* target, size_t size) {
    /* getting the page size and step. */
    size_t page_size = get_page_size();
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    size_t step = si.dwAllocationGranularity;
#else
    size_t step = page_size;
#endif
    size_t new_size = (size + page_size - 1u) & ~(page_size - 1u);

    /* iterate... */
    uintptr_t address = (uintptr_t) target;
    uintptr_t base = address & ~(step - 1u);
    const uintptr_t max_distance = MAX_DISTANCE;
    for (uintptr_t iter = step; iter < max_distance; iter += step) {
        uintptr_t upper = base + iter;
        uintptr_t lower = base - iter;

        /* check the upper and lower ranges. */
        if (e_intt_passed(rel_range_chk(target, upper))) {
#ifdef __gnu_linux__
            void* region = mmap((void*)upper, new_size, \
                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | \
                MAP_FIXED_NOREPLACE, -1, 0x0);
            if (region == (void*)upper) return region;
            /* if the allocation doesn't work then we simply unmap it. */
            if (region != MAP_FAILED) munmap(region, new_size);
#else
            void* region = VirtualAlloc((void*)upper, new_size, MEM_RESERVE | MEM_COMMIT, \
                PAGE_READWRITE);
            if (region) return region;
#endif
        }
        if (e_intt_passed(rel_range_chk(target, lower))) {
#ifdef __gnu_linux__
            void* region = mmap((void*)lower, new_size, \
                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | \
                MAP_FIXED_NOREPLACE, -1, 0x0);
            if (region == (void*)lower) return region;
            /* if the allocation doesn't work then we simply unmap it. */
            if (region != MAP_FAILED) munmap(region, new_size);
#else
            void* region = VirtualAlloc((void*)lower, new_size, MEM_RESERVE | MEM_COMMIT, \
                PAGE_READWRITE);
            if (region) return region;
#endif
        }
    }
    return 0x0; /* not found :( */
};

/**
 * @brief ...
 *
 * @param reloc ...
 */
internal void
free_region(reloc_t* reloc) {
#ifdef _WIN32
    VirtualFree(reloc->callee, reloc->size, MEM_DECOMMIT); /* we could use this again soon. */
#else
    munmap(reloc->callee, reloc->size);
#endif
};

/* an internal hashmap of relocs. */
internal map_t* reloc_table;

/**
 * @brief make a relocation from a short relative call to an absolute
 *  call to anywhere within the binary.
 *
 * @param address the address to relocate a relative call from.
 * @param target the target address to attempt to call.
 * @param thumb the target address is currently in thumb mode.
 * @return a relocation structure ready to be used.
 */
reloc_t*
reloc_make(void* address, void* target, bool thumb) {
    /* allocate the structure and push it to the internal table. */
    assert(address != 0x0 && target != 0x0);
    reloc_t* reloc = calloc(1u, sizeof *reloc);
    reloc->caller = address;
    reloc->callee = target;
#ifdef __amd64__
    reloc->size = 17u;
    reloc->bytes = calloc(1u,  reloc->size);

    /* endbr64 / movabs r11, imm64 / jmp r11. */
    uint8_t bytes[17u] = {
        0xf3, 0x0f, 0x1e, 0xfa, \
        0x49, 0xbb, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
        0x41, 0xff, 0xe3
    };
    *(uint64_t*)(bytes + 6u) = (uint64_t)reloc->callee;
    memcpy(reloc->bytes, bytes, reloc->size);
#endif
#ifdef __aarch64__
    reloc->size = 16u;
    reloc->bytes = calloc(1u,  reloc->size);

    /* ldr x17, #8 / br x17 / .quad target. */
    uint8_t bytes[16u] = {
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
    };
    *(uint32_t*)(bytes) = htole32(0x58000051);
    *(uint32_t*)(bytes + 4u) = htole32(0xd61f0220);
    *(uint64_t*)(bytes + 8u) = htole64((uint64_t)reloc->callee);
    memcpy(reloc->bytes, bytes, reloc->size);
#endif
#ifdef __i386
    /* todo; clobbering is possible with different calling conventions on win32, ie.
     *  cdecl vs. fastcall vs. stdcall. */
    reloc->size = 7u;
    reloc->bytes = calloc(1u,  reloc->size);

    /* mov eax, imm32 / jmp eax. */
    uint8_t bytes[7u] = {
        0xb8, 0x0, 0x0, 0x0, 0x0, 0xff, 0xe0
    };
    *(uint32_t*)(bytes + 1u) = (uint32_t)reloc->callee;
    memcpy(reloc->bytes, bytes, reloc->size);
#endif
#ifdef __arm__
    /* if we are in thumb-mode, we need to set the reloc address to have the thumb-bit enabled. */
    reloc->size = 12u;
    reloc->bytes = calloc(1u,  reloc->size);
    if (thumb) reloc->callee = (void*)((uintptr_t)reloc->callee | 1u);
    else reloc->callee = (void*)((uintptr_t)reloc->callee & ~1u);

    /* two different forms of shell-code for each. */
    if (thumb) {
        uint8_t bytes[12u] = {
            0xdf, 0xf8, 0x04, 0xc0, /* ldr.w ip, [pc, #4] / ... */
            0x60, 0x47, /* bx ip / ... */
            0x0, 0xbf, /* nop / .dword target. */
            0x0, 0x0, 0x0, 0x0, 0x0
        };
        *(uint32_t*)(bytes + 8u) = htole32((uint32_t)reloc->callee);
        memcpy(reloc->bytes, bytes, reloc->size);
    }
    else {
        /* ldr r12, [pc + 0x8] / bx r12 / .dword target. */
        uint8_t bytes[12u] = {
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
            0x0, 0x0, 0x0, 0x0,
        };
        *(uint32_t*)(bytes) = htole32(0xe59fc000);
        *(uint32_t*)(bytes + 4u) = htole32(0xe12fff1c);
        *(uint32_t*)(bytes + 8u) = htole32((uint32_t)reloc->callee);
        memcpy(reloc->bytes, bytes, reloc->size);
    }
#endif
    /* allocate the region and then copy it over. */
    reloc->region = alloc_region(address, reloc->size);
    if (reloc->region == 0x0) {
        fprintf(stderr, "tapi, reloc_make; unable to find region in given relative call range!\n");
        free(reloc->bytes);
        free(reloc);
        return 0x0;
    }
    uint8_t* region_bytes = reloc->region;
    memcpy(region_bytes, reloc->bytes, reloc->size);

    /* we now need to change this page to be executable. */
#ifndef _WIN32
    /* NOLINTNEXTLINE */
    if (mprotect(reloc->region, reloc->size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0x0) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi, reloc_make; mprotect failed; could not protect region!");
#else
        /* winapi doesn't care and does it for us. */
        DWORD old;
        if (VirtualProtect(reloc->region, reloc->size, PAGE_EXECUTE_READWRITE, &old) != 0x0) {
        /* we can actually use MSVCs "safe" version for fprintf. */
        fprintf_s(stderr, "tapi, reloc_make; mprotect failed; could not protect region!");
#endif
        reloc_cleanup(reloc);
        return 0x0; /* failure. */
    }

    /* and then flush the instruction cache. */
#ifdef _WIN32
    FlushInstructionCache(GetCurrentProcess(), reloc->region, reloc->size);
#else
    __builtin___clear_cache(reloc->region, reloc->region + reloc->size);
#endif

    /* we use the addresses as a key. */
    char buffer[256u];
    snprintf(buffer, 256u, "%p%p", address, target);

    /* push onto the internal table. */
    if (reloc_table == 0x0) reloc_table = map_make();
    map_push(reloc_table, buffer, reloc);

#ifdef __arm__
    /* this prevents a sigill. */
    if (thumb) reloc->region = (void*)((uintptr_t)reloc->region | 1u);
#endif
    return reloc;
};

/**
 * @brief attempt to find an already made relocation structure,
 *  if not found one will be made.
 *
 * @param address the address to relocate a relative call from.
 * @param target the target address to attempt to call.
 * @param thumb the target address is currently in thumb mode.
 * @return a relocation structure if found, if not one will be made which can return 0x0.
 */
reloc_t*
reloc_find(void* address, void* target, bool thumb) {
    /* attempt to get a pre-made value from the table, then simply check the distance. */
    assert(address != 0x0 && target != 0x0);
    char buffer[256u];
    snprintf(buffer, 256u, "%p%p", address, target);

    /* attempt to read, if found return it o.w. make one. */
    if (reloc_table == 0x0) reloc_table = map_make();
    void* found = map_lookup(reloc_table, buffer);
    if (found != 0x0) return (reloc_t*)found;
    return reloc_make(address, target, thumb);
};

/**
 * @brief free a relocation structure (free the memory used as well).
 *
 * @param reloc the relocation structure to be freed.
 */
void
reloc_cleanup(reloc_t* reloc) {
    /* free the region, then the bytes then the reloc. */
    assert(reloc != 0x0);
    free_region(reloc->region);
    free(reloc->bytes);
    free(reloc);
};