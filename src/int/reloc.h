/**
 * @author Sean Hobeck
 * @date 2026-07-20
 */
#ifndef RELOC_H
#define RELOC_H

/*! uses size_t. */
#include <stddef.h>

/**
 * ...
 */
typedef struct {
    void* caller, *callee; /* address of caller and callee of relocation. */
    size_t size; /* instruction(s) size in bytes. */
    unsigned char* bytes; /* the bytes of the absolute jump. */
    void* region; /* the allocated region of the absolute jump. */
} reloc_t;

/**
 * @brief make a relocation from a short relative call to an absolute
 *  call to anywhere within the binary.
 *
 * @param address the address to relocate a relative call from.
 * @param target the target address to attempt to call.
 * @param base the address of the base function.
 * @return a relocation structure ready to be used.
 */
reloc_t*
reloc_make(void* address, void* target, void* base);

/**
 * @brief attempt to find an already made relocation structure,
 *  if not found one will be made.
 *
 * @param address the address to relocate a relative call from.
 * @param target the target address to attempt to call.
 * @param base the address of the base function.
 * @return a relocation structure if found, if not one will be made which can return 0x0.
 */
reloc_t*
reloc_find(void* address, void* target, void* base);

/**
 * @brief free a relocation structure (free the memory used as well).
 *
 * @param reloc the relocation structure to be freed.
 */
void
reloc_cleanup(reloc_t* reloc);
#endif /* RELOC_H */
