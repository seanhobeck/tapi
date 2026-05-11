/**
 * @author Sean Hobeck
 * @date 2026-02-21
 */
#include "det.h"

/*! @uses bool, true, false. */
#include <stdbool.h>

/*! @uses memcpy. */
#include <string.h>

/*! @uses cs_insn, csh, cs_open. */
#include <capstone/capstone.h>

/*! @uses internal. */
#include "intt.h"

/*! @uses arch_t, get_arch. */
#include "arch.h"

/**
 * @brief is the instruction specified used to end a function?
 *
 * @param insn the instruction to be inspected.
 * @param handle the capstone handle used.
 * @return if the instruction is a function end type.
 */
internal bool
is_end_inst(cs_insn* insn, csh handle) {
    /* check by capstone groupings (not accurate). */
    if (cs_insn_group(handle, insn, CS_GRP_RET) || cs_insn_group(handle, insn, CS_GRP_IRET) \
        || cs_insn_group(handle, insn, CS_GRP_BRANCH_RELATIVE)) {
        return true;
    }
    return false;
}

/**
 * @brief is the instruction specified a call at the end of a function to somewhere else?
 *
 * @param insn the instruction to be inspected.
 * @param architecture the given architecture.
 * @return if the instruction is a tail call.
 */
internal bool
is_tail_call(cs_insn* insn, arch_t architecture) {
    /* unconditional jumps only are tail calls. */
    if (architecture.arch == CS_ARCH_X86) {
        return insn->id == X86_INS_JMP;
    }
    if (architecture.arch == CS_ARCH_ARM) {
        /* only unconditional branches, not conditional. */
        return (insn->id == ARM_INS_B || insn->id == ARM_INS_BX)
            && insn->detail->arm.cc == ARMCC_AL;
    }
    return false;
}

/**
 * @brief find the size of the function in memory.
 *
 * @param address the address of the function to analyze.
 * @param max_size the max size to search.
 * @return size of the function in memory.
 */
size_t
det_function_size(void* address, size_t max_size) {
    /* open capstone to reading at the specified address with the native architecture. */
    csh handle;
    size_t size = 0;
    arch_t architecture = get_arch();
    /* detect if we need to use thumb based on the thumb bit. */
    bool is_thumb = architecture.mode == CS_MODE_ARM && (uintptr_t)address & 1u;
    if (is_thumb) {
        address = (void*)((uintptr_t)address & ~1u);
        architecture.mode = CS_MODE_THUMB;
    }
    cs_open(architecture.arch, architecture.mode, &handle);
    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

    /* get the bytes at the address, and create an iterator. */
    const uint8_t* bytes = (unsigned char*) address;
    uint64_t iter = (uintptr_t) address;
    size_t code_size = max_size;
    cs_insn* insn = cs_malloc(handle);
    if (!insn) {
        cs_close(&handle);
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi, det_function_size; cs_malloc failed; could not allocate memory for "
                        "instructions.");
        return 0;
    }

    /* start iterating. */
    int32_t pad_count = 0;
    bool found_end = false;
    while (cs_disasm_iter(handle, &bytes, &code_size, &iter, insn)) {
        size += insn->size;

        /* are any of these function end instructions? */
        if (is_end_inst(insn, handle))
            found_end = !is_tail_call(insn, architecture);

        /* if we aren't at the start of a function, check if we are at a common prologue. */
        if (size != insn->size || found_end) {
            bool is_prologue = false;
            if (architecture.arch == CS_ARCH_ARM) {
                /* common arm/thumb prologues, push {r7, ...} or push {fp, ...}. */
                is_prologue = insn->id == ARM_INS_PUSH;
            }
            else if (architecture.arch == CS_ARCH_AARCH64) {
                /* common aarch64 prologues, stp x29, x30, [sp, ...]. */
                is_prologue = insn->id == AARCH64_INS_STP;
            }

            /* if we hit another function prologue, stop before it. */
            if (is_prologue) {
                size -= insn->size;
                break;
            }
        }

        /* have we already hit a function-ending instruction? */
        if (found_end) {
            /* we scan for nop padding sections. */
            bool is_padding = false;
            if (architecture.arch == CS_ARCH_X86)
                is_padding = insn->id == X86_INS_NOP || insn->id == X86_INS_INT3;
            else if (architecture.arch == CS_ARCH_ARM) {
                /* thumb and arm nops, also mov r0, r0/ mov r8, r8. */
                is_padding = insn->id == ARM_INS_ALIAS_NOP || (insn->id == ARM_INS_MOV &&
                    insn->detail->arm.operands[0].reg == insn->detail->arm.operands[1].reg);
            }
            else if (architecture.arch == CS_ARCH_AARCH64)
                is_padding = insn->id == AARCH64_INS_ALIAS_NOP;
            /* are we encountering padding? */
            if (is_padding) {
                pad_count++;
                if (pad_count > 2)
                    break;
            } else {
                pad_count = 0;
            }
        }

        /* sanity bounds. */
        if (size >= max_size) {
            /* NOLINTNEXTLINE */
            fprintf(stderr, "tapi, det_function_size; warning; hit max search size of %zu bytes\n",
                max_size);
            break;
        }
    }

    /* free and close. */
    cs_free(insn, 1);
    cs_close(&handle);
    return size;
}

/**
 * @brief search through an x86 instructions opt. info to find if it is a call to an
 *  immediate/ relative address within the memory; for x86|x86_64 only.
 *
 * @param target the target address to search within the instruction for.
 * @param call the call information structure to be used.
 * @param insn the instruction to be searched.
 * @param mode the mode of the architecture (x86 vs. x86_64).
 * @return ref. to intt.h for enum.
 */
internal e_intt_result_t
find_call_bx86(const void* target, det_call_t* call, const cs_insn* insn, const cs_mode mode) {
    /* we look for the target address. */
    uint64_t address = 0u;
    cs_x86* ops = &insn->detail->x86;
    for (size_t i = 0; i < ops->op_count; i++) {
        /* iterate until we find the immediate value used in the call. */
        cs_x86_op* op = &ops->operands[i];
        if (op->type == X86_OP_IMM) {
            if (mode == CS_MODE_64) {
                /* relative call. */
                if (op->imm <= UINT32_MAX && ops->disp == 0u) {
                    address = insn->address + insn->size + op->imm;
                    call->is_rel = true;
                    call->orig_off = (int32_t) op->imm;
                }
                else if (op->size == 8u) {
                    /* absolute call, we can still modify these as if they were rel. */
                    address = op->imm;
                    call->is_rel = true;
                }
                /* we currently don't support rip-rel calls. */
            }
            else {
                /* e8 call with 4-byte immediate. */
                if (op->size == 4u) {
                    address = op->imm;
                    call->is_rel = true;
                }
                else { /* unrecognized. */ }
            }

            /* check if it is our target. */
            bool is_target = address == (uint64_t) target;
            if (is_target) {
                call->call = (void*) insn->address;
                call->dest = (void*) target;
                call->size = insn->size;

                /* copy and return. */
                /* NOLINTNEXTLINE */
                memcpy(call->bytes, insn->bytes, insn->size < 32u ? insn->size : 32u);
                return E_INTT_RESULT_SUCCESS;
            }
        }
    }
    return E_INTT_RESULT_FAILURE;
}

/**
 * @brief search through an arm instructions opt. info to find if it is a call to an immediate/
 *  relative address within memory; for aarch32|thumb only.
 *
 * @param target the target address to search within the instruction for.
 * @param call the call information structure to be used.
 * @param insn the instruction to be searched.
 * @return ref. to intt.h for enum.
 */
internal e_intt_result_t
find_call_barm32(const void* target, det_call_t* call, const cs_insn* insn) {
    bool branch_link = insn->id == ARM_INS_BL;
    bool blx = insn->id == ARM_INS_BLX;
    if (branch_link || blx) {
        cs_arm* arm = &insn->detail->arm;

        /* iterate through every operand. */
        for (size_t i = 0x0; i < arm->op_count; i++) {
            cs_arm_op* op = &arm->operands[i];
            if (op->type == ARM_OP_IMM) {
                uint64_t target_addr = (uint64_t)op->imm;
                call->is_rel = true;

                /* is it a branch with link insn? */
                if (branch_link) {
                    /* calculate original offset. */
                    if (call->is_thumb) {
                        /* thumb bl/blx; 2-instruction sequence
                         *  bl encodes a 21-bit signed offset. */
                        call->orig_off = (int32_t)(op->imm - (insn->address + 4u)) >> 1u;
                    } else {
                        /* arm bl; encodes 24-bit signed offset. */
                        call->orig_off = (int32_t)(op->imm - (insn->address + 8u)) >> 2u;
                    }
                }

                /* is this the target address? */
                uint64_t target_compare = (uint64_t)target & ~1u;
                uint64_t addr_compare = target_addr & ~1u;
                if (target_compare == addr_compare) {
                    call->call = (void*)insn->address;
                    call->dest = (void*)target_addr;
                    call->size = insn->size;

                    /* is this a blx interworking call? */
                    if (blx) {
                        if (call->is_thumb)
                            call->orig_off = (int32_t)(op->imm - (insn->address + 4u)) >> 1u;
                        else
                            call->orig_off = (int32_t)(op->imm - (insn->address + 8u)) >> 2u;
                    }

                    /* copy bytes and return. */
                    /* NOLINTNEXTLINE */
                    memcpy(call->bytes, (void*)insn->address,
                           insn->size < 32u ? insn->size : 32u);
                    return E_INTT_RESULT_SUCCESS;
                }
            }
        }
    }
    return E_INTT_RESULT_FAILURE;
}

/**
 * @brief search through an aarch64 instructions opt. info to find if it is a call to an immediate/
 *  relative address within memory; for aarch64 only.
 *
 * @param target the target address to search within the instruction for.
 * @param call the call info structure to be used.
 * @param insn the instruction to be searched.
 * @return ref. to intt.h for enum.
 */
internal e_intt_result_t
find_call_baarch64(const void* target, det_call_t* call, const cs_insn* insn) {
    /* is this a branch with link insn? */
    if (insn->id == AARCH64_INS_BL) {
        cs_aarch64* aarch64 = &insn->detail->aarch64;

        /* iterate... */
        for (size_t i = 0; i < aarch64->op_count; i++) {
            cs_aarch64_op* op = &aarch64->operands[i];

            /* are we dealing with the immediate value? */
            if (op->type == AARCH64_OP_IMM) {
                uint64_t target_addr = op->imm;
                call->is_rel = true;

                /* calculate offset, target = pc + (imm << 2). */
                call->orig_off = (int32_t)(target_addr - insn->address);

                /* is this the target address? */
                if ((void*)target_addr == target) {
                    call->call = (void*)insn->address;
                    call->dest = (void*)target_addr;
                    call->size = insn->size;

                    /* copy bytes and return. */
                    /* NOLINTNEXTLINE */
                    memcpy(call->bytes, (void*)insn->address,
                           insn->size < 32u ? insn->size : 32u);
                    return E_INTT_RESULT_SUCCESS;
                }
            }
        }
    }
    /* blr for indirect calls via register. */
    if (insn->id == AARCH64_INS_BLR) {
        /* this is unsupported at the moment. */
        return E_INTT_RESULT_FAILURE;
    }
    return E_INTT_RESULT_FAILURE;
}

/**
 * @brief is this a call based on the architecture (manual for arm, and insn_group for CS_GRP_CALL).
 *
 * @param handle the capstone handle required.
 * @param insn the capstone instruction.
 * @param architecture the architecture we are using.
 * @return if this is a some type of a call instruction.
 */
internal e_intt_result_t
is_call_arch(csh handle, cs_insn* insn, arch_t architecture) {
    /* aarch64. */
    if (architecture.arch == CS_ARCH_AARCH64) {
        return insn->id == AARCH64_INS_BL || insn->id == AARCH64_INS_BLR;
    }
    /* arm32/ armth. */
    if (architecture.arch == CS_ARCH_ARM) {
        return insn->id == ARM_INS_BL || insn->id == ARM_INS_BLX;
    }
    /* x86 like. */
    return cs_insn_group(handle, insn, CS_GRP_CALL);
}

/**
 * @brief determine the call target within a function in memory.
 *
 * @param source the function in memory to search through.
 * @param target the target call to look for.
 * @return a pointer to a det_call_t structure, and 0 o.w.
 */
det_call_t*
det_call_target(void* source, const void* target) {
    /* open memory for the compile-time architecture. */
    csh handle;
    arch_t architecture = get_arch();

    /* detect if we need to use thumb based on the thumb bit. */
    bool is_thumb = architecture.mode == CS_MODE_ARM && (uint64_t) source & 1u;
    if (is_thumb)
        architecture.mode = CS_MODE_THUMB;
    cs_open(architecture.arch, architecture.mode, &handle);
    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

    /* allocate the pointer. */
    det_call_t* call = calloc(1u, sizeof *call);
    if (call == 0x0) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi, det_call_target; calloc failed; could not allocate memory for "
                        "det_call_t*.\n");
        return 0x0;
    }
    call->is_thumb = is_thumb;

    /* set up address and size for the iteration. */
    size_t size = det_function_size(source, 0x1000);
    if (is_thumb)
        source = (void*)((uintptr_t)source & ~1u);
    const uint8_t* bytes = source;
    uint64_t address = (uint64_t) source;
    cs_insn* insn = cs_malloc(handle);
    if (!insn) {
        cs_close(&handle);
        free(call);
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi det_call_target, cs_malloc failed; could not allocate memory for "
                        "instructions.\n");
        return 0x0;
    }

    /* iterating. */
    while (cs_disasm_iter(handle, &bytes, &size, &address, insn)) {
        /* is this a call instruction? */
        if (is_call_arch(handle, insn, architecture)) {
            /* get the target address */
            switch (architecture.arch) {
                case (CS_ARCH_X86): {
                    /* check if we can find the target in the insn. */
                    if e_intt_passed(find_call_bx86(target, call, insn, architecture.mode)) {
                        cs_free(insn, 1u);
                        cs_close(&handle);
                        return call;
                    }
                    break;
                }
                /* same for both arm32 and aarch64. */
                case (CS_ARCH_ARM): {
                    /* this is really aarch32 + thumb. */
                    if e_intt_passed(find_call_barm32(target, call, insn)) {
                        cs_free(insn, 1u);
                        cs_close(&handle);
                        return call;
                    }
                    break;
                }
                case (CS_ARCH_AARCH64): {
                    if e_intt_passed(find_call_baarch64(target, call, insn)) {
                        cs_free(insn, 1u);
                        cs_close(&handle);
                        return call;
                    }
                    break;
                }
                default: break;
            }
        }
    }

    /* free & return. */
    free(call);
    cs_free(insn, 1u);
    cs_close(&handle);
    return 0x0;
}