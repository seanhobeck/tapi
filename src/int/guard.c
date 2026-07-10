/**
 * @author Sean Hobeck
 * @date 2026-07-09
 */
#include "guard.h"

/*! uses calloc. */
#include <stdlib.h>

/*! uses fprintf, stderr. */
#include <stdio.h>

#ifndef _WIN32
/*! uses mprotect. */
#include <sys/mman.h>
#endif

/*! uses internal, etc... */
#include "intt.h"

/*! uses dyna_t, etc... */
#include "dyna.h"

/*! uses page_align_down, get_page_size. */
#include "pgutl.h"

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
    dyna_foreach(context->guards, guard_t*, guard)
        if (guard->address == address && guard->length == length) {
            guard->ref_count++;
            return;
        }
    dyna_endforeach(context->guards);

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
    dyna_foreach(context->guards, guard_t*, guard)
        guard_close(guard);
        free(guard);
    dyna_endforeach(context->guards);
    free(context->guards->data);
    free(context->guards);
};