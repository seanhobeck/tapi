/**
 * @author Sean Hobeck
 * @date 2026-09-05
 */
#ifndef RELOC_H
#define RELOC_H

/*! uses size_t. */
#include <stddef.h>

/*! uses bool, true, false. */
#include <stdbool.h>

/*! uses uint8_t. */
#include <stdint.h>

/**
 * a data structure for representing a relocation call (sometimes referred to as a trampoline).
 * this is done for when relative calls cannot reach the original call target, ie. on x86_64,
 *  e8 rel32 can only hold 4-bytes with an address space of 8, thus if anything is not within
 *  a 4-byte range it cannot be called without using a reloc.
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
 * @param thumb the target address is currently in thumb mode.
 * @param disp32 if we should be checking with a rip-based call.
 * @return a relocation structure ready to be used.
 */
reloc_t*
reloc_make(void* address, void* target, bool thumb, bool disp32);

/**
 * @brief make a relocation from a short relative call to a an allocated
 *  space of bytes with a custom size.
 *
 * @param address the address to relocate a call from.
 * @param target the target address to attempt to call.
 * @param size the size of the region to allocate.
 * @param bytes the bytes to be written.
 * @param disp32 if we should be checking with a rip-based call.
 * @return a relocation structure ready to be used.
 */
reloc_t*
reloc_make_custom(void* address, void* target, size_t size, \
    uint8_t* bytes, bool disp32);

/**
 * @brief attempt to find an already made relocation structure,
 *  if not found one will be made.
 *
 * @param address the address to relocate a relative call from.
 * @param target the target address to attempt to call.
 * @param thumb the target address is currently in thumb mode.
 * @param disp32 if we should be checking with a rip-based call.
 * @return a relocation structure if found, if not one will be made which can return 0x0.
 */
reloc_t*
reloc_find(void* address, void* target, bool thumb, bool disp32);

/**
 * @brief free a relocation structure (free the memory used as well).
 *
 * @param reloc the relocation structure to be freed.
 */
void
reloc_cleanup(reloc_t* reloc);
#endif /* RELOC_H */
