/**
 * @author Sean Hobeck
 * @date 2026-03-18
 */
#ifndef PGUARD_H
#define PGUARD_H

/*! @uses size_t. */
#include <stddef.h>

/** a data structure for a memory protection guard. */
typedef struct {
    void* address; /* address in memory to be written to. */
    size_t length, flags; /* length and old flags for memory (winapi). */
    size_t ref_count; /* a reference count for the number of times the guard has been called. */
} guard_t;

/**
 * @brief create a write-protect guard for an address in memory.
 *
 * @param address the address to be given write protection in memory.
 * @param length the length of bytes to be protected.
 */
void
guard_create(void* address, size_t length);

/** @brief clean up the internal guard list. */
void
guard_cleanup();
#endif /* PGUARD_H */