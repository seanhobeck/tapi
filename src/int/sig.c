/**
 * @author Sean Hobeck
 * @date 2026-05-19
 */
#include "sig.h"

/*! @uses fprintf. */
#include <stdio.h>

/*! @uses calloc, free. */
#include <stdlib.h>

/*! @uses strlen, snprintf. */
#include <string.h>

/*! @uses internal. */
#include "intt.h"

/**
 * @brief format a single instruction into a string for display.
 *
 * @param insn the instruction to format.
 * @return an allocated formatted string, or 0x0 on failure.
 */
internal char*
format_insn(cs_insn* insn) {
    /* allocate buffer for formatted line. */
    char* line = calloc(1u, 192u); /* 160 is max for op_str, and 32 is max for mnemonic in capstone. */
    if (!line) {
        fprintf(stderr, "tapi, format_insn; calloc failed; could not allocate memory for line.\n");
        return 0x0;
    }

    /* if there exists an opcode for the given instruction, just append. */
    if (insn->op_str[0]) snprintf(line, 192u, "%s %s", insn->mnemonic, insn->op_str);
    else snprintf(line, 32u, "%s", insn->mnemonic);
    return line;
}

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
sig_compare(const char* signature, cs_insn* insn) {
    /* get formatted string. */
    char* fmtd_insn = format_insn(insn);

    /* iterate the format string pointer as well as the signature string pointer. */
    size_t s = 0, f = 0;
    while (signature[s] && fmtd_insn[f]) {
        if (signature[s] == '?') {
            /* iterate through the extra question marks. */
            while (signature[s] == '?')
                s++;

            /* we have consumed the signature string, we are good. */
            if (signature[s] == '\0') {
                free(fmtd_insn);
                return true;
            }

            /* if we have hit a '?' then we can increase our format string ptr. */
            while (fmtd_insn[f] && fmtd_insn[f] != signature[s])
                f++;

            /* the formatted string was consumed, we are done. */
            if (fmtd_insn[f] == '\0') {
                free(fmtd_insn);
                return false;
            }
        }
        else {
            /* compare the characters of the format and signature when we are
                not iterating through the '?' characters. */
            if (signature[s] != fmtd_insn[f]) {
                free(fmtd_insn);
                return false;
            }

            /* increment as per usual. */
            s++;
            f++;
        }
    }
    while (signature[s] == '?') s++; /* consume until we are done reading the extra '?' */

    /* have we fully consumed the strings? */
    char last = fmtd_insn[f];
    free(fmtd_insn);
    return signature[s] == '\0' && last == '\0';
};

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
sig_aarch64_chk(cs_insn* insn) {
    /* create our list of signatures (prologues and epilogues). */
    const static char* prologues[] = { AARCH64_P1, AARCH64_P2, AARCH64_P3, AARCH64_P4 };
    const static char* epilogues[] = { AARCH64_E1, AARCH64_E2, AARCH64_E3 };

    /* iterate over our list of signatures. */
    for (size_t i = 0; i < sizeof(prologues) / sizeof(char*); i++) {
        if (sig_compare(prologues[i], insn)) return SIG_AARCH64_PROLOGUE;
    }
    for (size_t i = 0; i < sizeof(epilogues) / sizeof(char*); i++) {
        if (sig_compare(epilogues[i], insn)) return SIG_AARCH64_EPILOGUE;
    }
    return SIG_UNRECOGNIZED;
};

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
sig_armhf_chk(cs_insn* insn) {
    /* create our list of signatures (prologues and epilogues). */
    const static char* prologues[] = {
        ARMHF_P1, ARMHF_P2, ARMHF_P3, ARMHF_P4, ARMHF_P5,
        ARMHF_P6, ARMHF_P7, ARMHF_P8, ARMHF_P9, ARMHF_P10,
        ARMHF_P11, ARMHF_P12, ARMHF_P13, ARMHF_P14, ARMHF_P15,
        ARMHF_P16, ARMHF_P17, ARMHF_P18
    };
    const static char* epilogues[] = {
        ARMHF_E1, ARMHF_E2, ARMHF_E3, ARMHF_E4, ARMHF_E5,
        ARMHF_E6, ARMHF_E7, ARMHF_E8, ARMHF_E9, ARMHF_E10,
        ARMHF_E11, ARMHF_E12, ARMHF_E13, ARMHF_E14, ARMHF_E15,
        ARMHF_E16, ARMHF_E17, ARMHF_E18
    };

    /* iterate over our list of signatures. */
    for (size_t i = 0; i < sizeof(prologues) / sizeof(char*); i++) {
        if (sig_compare(prologues[i], insn)) return SIG_ARMHF_PROLOGUE;
    }
    for (size_t i = 0; i < sizeof(epilogues) / sizeof(char*); i++) {
        if (sig_compare(epilogues[i], insn)) return SIG_ARMHF_EPILOGUE;
    }
    return SIG_UNRECOGNIZED;
};