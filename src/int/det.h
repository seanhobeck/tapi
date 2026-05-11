/**
 * @author Sean Hobeck
 * @date 2026-02-21
 */
#ifndef DET_H
#define DET_H

/*! @uses size_t. */
#include <stddef.h>

/*! @uses uint8_t. */
#include <stdint.h>

/*! @uses bool, true, false. */
#include <stdbool.h>

/**
 * @brief find the size of the function in memory.
 *
 * @param address the address of the function to analyze.
 * @param max_size the max size to search.
 * @return size of the function in memory.
 */
size_t
det_function_size(void* address, size_t max_size);

/**
 * a data structure representing determined call instruction information read from capstone.
 * this data structure holds all the necessary information required to do a binary patch on that
 *  instruction, diverting addresses to stubs, which are then used by users in test_cases.
 */
typedef struct {
    void* call, *dest; /* pointer to both insn and dest. address. */
    size_t size; /* instruction size. */
    uint8_t bytes[32u]; /* bytes used in the call (max 32). */
    bool is_rel, is_thumb; /* is it a relative address?, are we arm thumb mode? */
    int32_t offset, orig_off; /* ... */
} det_call_t;

/**
 * @brief determine the call target within a function in memory.
 *
 * @param source the function in memory to search through.
 * @param target the target call to look for.
 * @return a pointer to a det_call_t structure, and 0 o.w.
 */
det_call_t*
det_call_target(void* source, const void* target);
#endif /* DET_H */