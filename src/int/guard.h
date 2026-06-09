/**
 * @author Sean Hobeck
 * @date 2026-06-09
 */
#ifndef PGUARD_H
#define PGUARD_H

/*! @uses size_t. */
#include <stddef.h>

/*! @uses atomic_size_t. */
#include <stdatomic.h>

/*! @uses tapi_context_t. */
#include <tapi/tapi.h>

/** a data structure for a memory protection guard. */
typedef struct {
    void* address; /* address in memory to be written to. */
    size_t length, flags; /* length and old flags for memory (winapi). */
    atomic_size_t ref_count; /* a reference count for the number of times the guard has been called. */
} guard_t;

/**
 * @brief create a write-protect guard for an address in memory.
 *
 * @param context the tapi context to be used.
 * @param address the address to be given write protection in memory.
 * @param length the length of bytes to be protected.
 */
void
guard_create(tapi_context_t* context, void* address, size_t length);

/**
 * @brief clean up the guard list in the tapi context.
 *
 * @param context the tapi context to be used.
 */
void
guard_cleanup(tapi_context_t* context);
#endif /* PGUARD_H */