/**
 * @author Sean Hobeck
 * @date 2026-06-09
 */
#include "guard.h"

/*! @uses calloc. */
#include <stdlib.h>

/*! @uses uintptr_t. */
#include <stdint.h>

/*! @uses sysconf, _SC_PAGE_SIZE. */
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/*! @uses mprotect. */
#include <sys/mman.h>

/*! @uses fprintf, stderr. */
#include <stdio.h>

/*! @uses internal, etc... */
#include "intt.h"

/*! @uses dyna_t, etc... */
#include "dyna.h"

/** @return page size on the given architecture, winapi and posix. */
internal size_t
get_page_size() {
#ifndef _WIN32
    long value = sysconf(_SC_PAGESIZE);
    return value > 0 ? (size_t)value : 4096u;
#else
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    return (size_t) si.dwPageSize;
#endif
}

/**
 * @brief align a page down and cast to an address for usage.
 *
 * @param page the page to be aligned down.
 * @return an address aligned down to the nearest page.
 */
internal void*
page_align_down(void* page, size_t size) {
    return (void*)((uintptr_t)page & ~(size - 1));
}

/**
 * @brief close/ restore the write-protect guard.
 *
 * @param guard the guard to be closed/ restored.
 */
internal void
guard_close(guard_t* guard) {
    /* remove write protection on the pages. */
#ifndef _WIN32
    if (mprotect(guard->address, guard->length, PROT_READ | PROT_EXEC) != 0x0) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi, guard_close; mprotect failed; could not close pguard.");
    }
#else
    DWORD tmp;
    if (VirtualProtect(guard->address, guard->length, guard->flags, &tmp) != 0x0) {
        /* we can actually use MSVCs "safe" version for fprintf. */
        fprintf_s(stderr, "tapi, guard_close; VirtualProtect failed; could not close pguard.");
    }
#endif
}

/**
 * @brief create a write-protect guard for an address in memory.
 *
 * @param context the tapi context to be used.
 * @param address the address to be given write protection in memory.
 * @param length the length of bytes to be protected.
 */
void
guard_create(tapi_context_t* context, void* address, size_t length) {
    /* check if it already exists in our context list of guards. */
    DYNA_FOREACH(context->guards, guard_t*, guard)
        if (guard->address == address && guard->length == length) {
            guard->ref_count++;
            return;
        }
    DYNA_ENDFOREACH(context->guards);

    /* if not found, then allocate. */
    guard_t* guard = calloc(1, sizeof *guard);
    guard->ref_count = 1;

    /* allow write protection on those pages. */
#ifndef _WIN32
    /* we find the relative page bounds. */
    guard->address = page_align_down(address, get_page_size());
    guard->length = length;
    /* NOLINTNEXTLINE */
    if (mprotect(guard->address, guard->length, PROT_READ | PROT_WRITE | PROT_EXEC) != 0x0) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi, guard_create; mprotect failed; could not allocate memory for pguard"
                        ".");
#else
    /* winapi doesn't care and does it for us. */
    guard->address = address;
    guard->length = length;
    if (VirtualProtect(guard->address, length, PAGE_EXECUTE_READWRITE, &guard->flags) !=
        0x0) {
        /* we can actually use MSVCs "safe" version for fprintf. */
        fprintf_s(stderr, "tapi, guard_create; VirtualProtect failed; could not allocate memory "
                          "for pguard.");
#endif
        free(guard);
        return;
    }

    /* push into our context list and return. */
    tapi_dyna_push(context->guards, guard);
}

/**
 * @brief clean up the guard list in the tapi context.
 *
 * @param context the tapi context to be used.
 */
void
guard_cleanup(tapi_context_t* context) {
    /* if an internal guard list was not made, don't worry about it! */
    if (!context->guards) return;

    /* iterate and return each guard. */
    DYNA_FOREACH(context->guards, guard_t*, guard)
        guard_close(guard);
        free(guard);
    DYNA_ENDFOREACH(context->guards);
    free(context->guards->data);
    free(context->guards);
};