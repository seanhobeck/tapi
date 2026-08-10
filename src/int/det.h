/**
 * @author Sean Hobeck
 * @date 2026-08-06
 */
#ifndef DET_H
#define DET_H

/*! uses tapi_dyna_t. */
#include <tapi/dyna.h>

/*! uses size_t. */
#include <stddef.h>

/*! uses uint8_t. */
#include <stdint.h>

/*! uses bool, true, false. */
#include <stdbool.h>

/**
 * @brief find the size of the function in memory.
 *
 * @param address the address of the function to analyze.
 * @param max_size the max size to search (0x200 usually is enough for small to mid-sized functions).
 * @return size of the function in memory.
 */
size_t
det_function_size(void* address, size_t max_size);

/**
 * @brief ...
 */
typedef enum {
    E_DET_CALLSPEC_UNDEF, /* general call. */
    E_DET_CALLSPEC_RIP_IND = 0x1, /* qword ptr[rip + disp32]. */
    E_DET_CALLSPEC_RAX_IND_32, /* qword ptr[rax + disp32] (requires trace). */
    E_DET_CALLSPEC_RAX_IND_8, /* qword ptr[rax + disp8] (requires trace). */
    E_DET_CALLSPEC_REG_IND, /* rax, rbx, r8-15, ... (requires trace). */
} e_det_call_spec_t;

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
    int32_t orig_off; /* the original offset size. */
    e_det_call_spec_t spec; /* the specific type of call. */
} det_call_t;

/**
 * @brief determine all call targets within a function in memory.
 *
 * @param source the function in memory to search through.
 * @param target the target call to look for.
 * @return a dynamic array of allocated det_call_t structures.
 */
tapi_dyna_t*
det_call_targets(void* source, const void* target);

/**
 * @brief determine a call target within a function in memory.
 *
 * @param source the function in memory to search through.
 * @param target the target call to look for.
 * @return a pointer to a det_call_t structure, and 0 o.w.
 */
det_call_t*
det_call_target(void* source, const void* target);
#endif /* DET_H */