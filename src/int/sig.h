/**
 * @author Sean Hobeck
 * @date 2026-05-19
 */
#ifndef SIG_H
#define SIG_H

/*! @uses bool, true, false. */
#include <stdbool.h>

/*! @uses cs_insn, etc... */
#include <capstone/capstone.h>

/**
 * @brief compare a disassembled capstone instruction to a signature
 *  template, treating '?' as an ignored character. a set of '?...?'
 *  with length k will be treated as a minimum of one ignored character
 *  and a max of k ignored characters; '?' is a min of 1, and a max of 1,
 *  '??' is a min of 1, and a max of 2, etc...
 *
 *  e.g:
 *      signature -> "stp ??, ??, [sp, #?????]
 *      insn -> stp x0, x1, [sp, #0x100]
 *  result -> match! return true.
 *
 *
 * @param signature the signature to compare the instruction to.
 * @param insn the disassembled instruction from capstone.
 * @return true if insn. is a match.
 */
bool
sig_compare(const char* signature, cs_insn* insn);

/**
 * an enum for the different types of signatures that can be compared against
 * for arm-based architectures.
 */
typedef enum {
    SIG_UNRECOGNIZED = 0x0,
    SIG_AARCH64_PROLOGUE, /* preceding contents of a function. */
    SIG_AARCH64_EPILOGUE, /* succeeding contents of a function. */
    SIG_ARMHF_PROLOGUE,
    SIG_ARMHF_EPILOGUE,
} e_sig_type_t;

/**
 * @brief check if the disassembled capstone instruction is considered
 *  an aarch64 function prologue (instruction that is preceding the
 *  contents of a function) or an epilogue (instruction succeeding the contents
 *  of a function).
 *
 * @param insn the disassembled instruction from capstone.
 * @return an enum corresponding to either recognized, prologue, or epilogue.
 */
e_sig_type_t
sig_aarch64_chk(cs_insn* insn);

/**
 * @brief check if the disassembled capstone instruction is considered
 *  an armhf function prologue (instruction that is preceding the
 *  contents of a function) or an epilogue (instruction succeeding the contents
 *  of a function).
 *
 * @param insn the disassembled instruction from capstone.
 * @return an enum corresponding to either recognized, prologue, or epilogue.
 */
e_sig_type_t
sig_armhf_chk(cs_insn* insn);


/*! @note common sequences for prologues & epilogues of functions for arm. */
#pragma region common sequences
#define AARCH64_P1 "sub sp, sp, #??????" /* sub. from the sp for new vars, max of #0xffff. */
#define AARCH64_P2 "stp ???, ???, [sp, #??????]" /* store pair of registers, max of #0xffff. */
#define AARCH64_P3 "stp ???, ???, [sp, #???????]!" /* max of #-0xffff. */
#define AARCH64_P4 "paciasp" /* enforcement for ROP attacks, PAC insns. */
#define AARCH64_E1 "add sp, sp, #??????" /* add to the sp to cleanup for vars, max of #0xffff. */
#define AARCH64_E2 "ldp ???, ???, [sp, #??????]" /* load a pair of registers, max of #0xffff. */
#define AARCH64_E3 "autiasp" /* enforcement for ROP attacks, PAC insns. */

/* there are various push pro/epilogues for armv7, most look like... */
#define ARMHF_P1 "push {r??}"
#define ARMHF_P2 "push {r??, lr}"
#define ARMHF_P3 "push {r??, r??}"
#define ARMHF_P4 "push {r??, r??, lr}"
#define ARMHF_P5 "push {r??, r??, r??}"
#define ARMHF_P6 "push {r??, r??, r??, lr}"
#define ARMHF_P7 "push {r??, r??, r??, r??}"
#define ARMHF_P8 "push {r??, r??, r??, r??, lr}"
#define ARMHF_P9 "push {r??, r??, r??, r??, r??}"
#define ARMHF_P10 "sub sp, sp, #??????" /* sub from the sp for new vars, max of $0xffff. */
#define ARMHF_P11 "stmdb sp!, {r??}"
#define ARMHF_P12 "stmdb sp!, {r??, lr}"
#define ARMHF_P13 "stmdb sp!, {r??, r??,}"
#define ARMHF_P14 "stmdb sp!, {r??, r??, lr}"
#define ARMHF_P15 "stmdb sp!, {r??, r??, r??}"
#define ARMHF_P16 "stmdb sp!, {r??, r??, r??, lr}"
#define ARMHF_P17 "stmdb sp!, {r??, r??, r??, r??}"
#define ARMHF_P18 "stmdb sp!, {r??, r??, r??, r??, lr}"
/* these need to encompass ^^ r4-11 as they are callee saved. */
#define ARMHF_E1 "pop {r??}"
#define ARMHF_E2 "pop {r??, pc}"
#define ARMHF_E3 "pop {r??, r??}"
#define ARMHF_E4 "pop {r??, r??, pc}"
#define ARMHF_E5 "pop {r??, r??, r??}"
#define ARMHF_E6 "pop {r??, r??, r??, pc}"
#define ARMHF_E7 "pop {r??, r??, r??, r??}"
#define ARMHF_E8 "pop {r??, r??, r??, r??, pc}"
#define ARMHF_E9 "pop {r??, r??, r??, r??, r??}"
#define ARMHF_E10 "add sp, sp, #??????" /* add to the sp to cleanup for vars, max of #0xffff. */
#define ARMHF_E11 "ldmia sp!, {r??}"
#define ARMHF_E12 "ldmia sp!, {r??, pc}"
#define ARMHF_E13 "ldmia sp!, {r??, r??,}"
#define ARMHF_E14 "ldmia sp!, {r??, r??, pc}"
#define ARMHF_E15 "ldmia sp!, {r??, r??, r??}"
#define ARMHF_E16 "ldmia sp!, {r??, r??, r??, pc}"
#define ARMHF_E17 "ldmia sp!, {r??, r??, r??, r??}"
#define ARMHF_E18 "ldmia sp!, {r??, r??, r??, r??, pc}"
/* for some reason armhf LOVES to use 'andeq' or 'lsls' as padding after an epilogue like ARMHF_E1-18? */
#define ARMHF_EP1 "lsls r??, r??, #??????"

/* for any x86 & x86_64 we aren't using any patterns since we can rely on capstones id's with CET. */
#pragma endregion
#endif /* SIG_H */