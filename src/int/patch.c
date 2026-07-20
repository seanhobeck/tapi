/**
 * @author Sean Hobeck
 * @date 2026-07-17
 */
#include "patch.h"

/*! uses fprintf, stderr. */
#include <stdio.h>

/*! uses arch_t, get_arch. */
#include "arch.h"

/*! uses guard_create, guard_close. */
#include "guard.h"

/*! uses internal. */
#include "intt.h"

/*! uses reloc_t, reloc_find. */
#include "reloc.h"

/**
 * @brief flush the instruction cache to allow our patched instructions to be read properly.
 *
 * @param address the address in memory to start from.
 * @param size the size of instructions to clear the cache for.
 */
internal void
flush_insn_cache(void* address, size_t size) {
#ifdef _WIN32
    FlushInstructionCache(GetCurrentProcess(), address, size);
#else
    __builtin___clear_cache(address, address + size);
#endif
}

/**
 * @brief patch a relative call on x86 architectures (assuming it is an 0xe8 call).
 *
 * @param call the pointer to the call insn. within a function.
 * @param size the size of the insn.
 * @param new_target the pointer to the new call target.
 * @return ref. to intt.h for enum.
 */
internal e_intt_result_t
patch_relative_bx86(void* call, const size_t size, const void* new_target) {
    /* get the given byte code. */
    uint8_t* code = call;

    /* x64 rel. call is always 5 bytes. */
    if (size < 5u) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "bx86/64; instruction too small (%zu bytes).\n", size);
        return E_INTT_RESULT_FAILURE;
    }

    /* verify its actually an e8 rel. call */
    if (code[0] != 0xe8) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "bx86/64; not a rel. call (0x%02x).\n", code[0]);
        return E_INTT_RESULT_FAILURE;
    }

    /* calculate new offset. */
    int64_t offset64 = (int64_t)new_target - ((int64_t)call + 5u);

    /* check if the offset is 32-bit signed. */
    if (offset64 > INT32_MAX || offset64 < INT32_MIN) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "bx86/64; new target out of range (>2gb).\n");
        return E_INTT_RESULT_FAILURE;
    }
    int32_t offset = (int32_t)offset64;

    /* write to e8 ?? ?? ?? ??, and return. */
    *(int32_t*)(code + 1u) = offset;
    return E_INTT_RESULT_SUCCESS;
}

/**
 * @brief patch a relative call on aarch32 architecture.
 *
 * @param call the pointer to the call insn. within a function.
 * @param size the size of the insn.
 * @param new_target the pointer to the new call target.
 * @return ref. to intt.h for enum.
 */
internal e_intt_result_t
patch_relative_barm(void* call, const size_t size, const void* new_target) {
    /* we only expect 4 bytes. */
    if (size != 4u) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "barm32; expected 4-byte insn., got %zu bytes.\n", size);
        return E_INTT_RESULT_FAILURE;
    }

    /* check if its a bl instruction (opcode bits 31-24 = 0xeb). */
    uint32_t* insn = call;
    if ((*insn & 0xff000000) != 0xeb000000) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "barm32; not a bl insn. (0x%08x).\n", *insn);
        return E_INTT_RESULT_FAILURE;
    }

    /* offset calc. = (target - (pc + 8)) >> 2 (we do the shifting later). */
    uint64_t pc = (uint64_t)call;
    uint64_t target = (uint64_t)new_target;
    int32_t offset = (int32_t)(target - (pc + 8u));

    /* check 24-bit signed range (+32mb). */
    if (offset > 0x7fffff || offset < -0x800000) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "barm32; target out of range (+32mb).\n");
        return E_INTT_RESULT_FAILURE;
    }

    /* shift right by 2 (4-byte aligned). */
    offset = offset >> 2;

    /* construct new insn; preserve the top 8 bits, set new offset. */
    uint32_t new_insn = (*insn & 0xff000000) | (offset & 0x00ffffff);

    /* write the entire 4-byte insn. */
    *insn = new_insn;
    return E_INTT_RESULT_SUCCESS;
}

/**
 * @brief patch a relative call on arm thumb architecture.
 *
 * @param call the pointer to the call insn. within a function.
 * @param size the size of the insn.
 * @param new_target the pointer to the new call target.
 * @return ref. to intt.h for enum.
 */
internal e_intt_result_t
patch_relative_barmth(void* call, const size_t size, const void* new_target) {
    /* we only expect 4 bytes. */
    if (size != 4u) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "barmth; expected 4-byte insn., got %zu bytes.\n", size);
        return E_INTT_RESULT_FAILURE;
    }

    /* check if its a bl insn (first half = 0xf). */
    uint16_t* insn = call;
    if ((insn[0] & 0xf800) != 0xf000) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "barmth; not a bl insn. (0x%04x)\n", insn[0]);
        return E_INTT_RESULT_FAILURE;
    }

    /* offset calc. = (target - (pc + 4)). */
    uint64_t pc = (uint64_t)call;
    uint64_t target = (uint64_t)new_target & ~1u;
    int32_t offset = (int32_t)(target - (pc + 4u));

    /* check 24-bit signed range (+/-16mb). */
    if (offset > 0xffffff || offset < -0x1000000) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "barmth; target out of range (+/-16mb).\n");
        return E_INTT_RESULT_FAILURE;
    }

    /* thumb offset is split into imm11:imm10:sgn:i1:i2,
     * first halfword = 11110 sgn imm10.
     * second halfword = 11 j1 1 j2 imm11.
     * where j1 = ~(i1 ^ sgn), j2 = ~(i2 ^ sgn).
     * offset[24:1] -> {sgn, i1, i2, imm10, imm11}. */
    uint32_t sgn = (offset >> 24) & 0x1;
    uint32_t i1 = (offset >> 23) & 0x1;
    uint32_t i2 = (offset >> 22) & 0x1;
    uint32_t imm10 = (offset >> 12) & 0x3ff;
    uint32_t imm11 = (offset >> 1) & 0x7ff;

    /* calculate j1 and j2. */
    uint32_t j1 = (~(i1 ^ sgn)) & 0x1;
    uint32_t j2 = (~(i2 ^ sgn)) & 0x1;

    /* reconstruct instruction halves. */
    uint16_t new_first = 0xf000 | (sgn << 10) | imm10;
    uint16_t new_second = 0xd000 | (j1 << 13) | (j2 << 11) | imm11;

    /* write back the 4 bytes. */
    insn[0] = new_first;
    insn[1] = new_second;
    return E_INTT_RESULT_SUCCESS;
}

/**
 * @brief patch a relative call on a 64-bit arm architecture.
 *
 * @param call the pointer to the call insn. within a function.
 * @param size the size of the insn.
 * @param new_target the pointer to the new call target.
 * @return ref. to intt.h for enum.
 */
internal e_intt_result_t
patch_relative_barm64(void* call, const size_t size, const void* new_target) {
    /* we only expect 4 bytes. */
    if (size != 4u) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "barm64; expected 4-byte insn., got %zu bytes.\n", size);
        return E_INTT_RESULT_FAILURE;
    }
    uint32_t* instr = call;

    /* check if its a bl instruction (opcode bits 31-26 = 0x25). */
    if ((*instr & 0xfc000000) != 0x94000000) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "barm64; not a bl insn. (0x%08x).\n", *instr);
        return E_INTT_RESULT_FAILURE;
    }

    /* offset calc = (target - pc) >> 2. */
    uint64_t pc = (uint64_t)call;
    uint64_t target = (uint64_t)new_target;
    int64_t offset64 = (int64_t)(target - pc);

    /* check 26-bit signed range (+128mb). */
    if (offset64 > 0x3ffffff || offset64 < -0x4000000) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "barm64; target out of range (+128mb)\n");
        return E_INTT_RESULT_FAILURE;
    }

    /* shift right by 2. */
    int32_t offset = (int32_t)(offset64 >> 2u);

    /* construct new insn; preserve top 6 bits, set new offset. */
    uint32_t new_instr = (*instr & 0xfc000000) | (offset & 0x03ffffff);

    /* write the entire 4-byte ins. */
    *instr = new_instr;
    return E_INTT_RESULT_SUCCESS;
}

/**
 * @brief binary patch a call to a target using
 *
 * @param context the tapi context to be used.
 * @param call the call structure info representing the call to be patched.
 * @param new_target the new target address to set the new call to.
 * @param base the base function that is being called (mock->orig).
 * @return 1 if successful, and 0 o.w.
 */
int32_t
patch_call_target(tapi_context_t* context, const det_call_t* call, \
    const void* new_target, void* base) {
    /* create a write-protect guard for an entire page. */
    guard_create(context, call->call, call->size);

    /* patch for a specific backend. */
    arch_t architecture = get_arch();
    if (call->is_rel) {
        switch (architecture.arch) {
            case CS_ARCH_X86: {
                if e_intt_passed(patch_relative_bx86(call->call, call->size, new_target)) {
                    break;
                }
                /* attempt a reloc. */
                reloc_t* reloc = reloc_find(call->call, (void*)new_target, base);
                if (reloc != 0x0) {
                    if e_intt_passed(patch_relative_bx86(call->call, call->size, reloc->region)) {
                        break;
                    }
                }
                else return 0u;

                /* NOLINTNEXTLINE */
                fprintf(stderr, "bx86/64; patching relative call failed, attempting absolute reloc also failed.\n");
                break;
            }
            case CS_ARCH_ARM: {
                /* arm thumb? */
                if (call->is_thumb) {
                    if e_intt_passed(patch_relative_barmth(call->call, call->size, new_target)) {
                        break;
                    }

                    /* attempt a reloc. */
                    reloc_t* reloc = reloc_find(call->call, (void*)new_target, base);
                    if (reloc != 0x0) {
                        if e_intt_passed(patch_relative_barmth(call->call, call->size, reloc->region)) {
                            break;
                        }
                    }
                    else return 0u;
                }
                else if e_intt_passed(patch_relative_barm(call->call, call->size, new_target)) {
                    break;
                }
                else {
                    /* attempt a reloc. */
                    reloc_t* reloc = reloc_find(call->call, (void*)new_target, base);
                    if (reloc != 0x0) {
                        if e_intt_passed(patch_relative_barm(call->call, call->size, reloc->region)) {
                            break;
                        }
                    }
                    else return 0u;
                }

                /* NOLINTNEXTLINE */
                fprintf(stderr, "barm32/th; patching relative call failed, attempting absolute reloc also failed.\n");
                break;
            }
            case CS_ARCH_AARCH64: {
                if e_intt_passed(patch_relative_barm64(call->call, call->size, new_target)) {
                    break;
                }
                /* attempt a reloc. */
                reloc_t* reloc = reloc_find(call->call, (void*)new_target, base);
                if (reloc != 0x0) {
                    if e_intt_passed(patch_relative_barm64(call->call, call->size, reloc->region)) {
                        break;
                    }
                }
                else return 0u;

                /* NOLINTNEXTLINE */
                fprintf(stderr, "barm64; patching relative call failed, attempting absolute reloc also failed.\n");
                break;
            }
            default: {
                /* NOLINTNEXTLINE */
                fprintf(stderr, "unknown architecture; corrupted?");
                return 0u;
            };
        }
    } else {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "unknown architecture; cannot patch a non-relative call!\n"); /* not yet... indirect here we come. */
        return 0u;
    }

    /* close the guard and flush insn. cache. */
    flush_insn_cache(call->call, call->size);
    return 1u;
}