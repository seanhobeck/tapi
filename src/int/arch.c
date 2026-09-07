/**
 * @author Sean Hobeck
 * @date 2026-09-05
 */
#include "arch.h"

/*! uses platform macros. */
#include <tapi/platform.h>

/** @return the current architecture pair for capstone, gen. at compile time. */
arch_t
get_arch(void) {
#ifdef TAPI_AMD64
    return (arch_t) { .arch = CS_ARCH_X86, .mode = CS_MODE_64 };
#endif
#ifdef TAPI_X86
    return (arch_t) { .arch = CS_ARCH_X86, .mode = CS_MODE_32 };
#endif
#ifdef TAPI_AARCH64
    return (arch_t) { .arch = CS_ARCH_AARCH64, .mode = CS_MODE_ARM };
#endif
#ifdef TAPI_ARM32
    return (arch_t) { .arch = CS_ARCH_ARM, .mode = CS_MODE_ARM };
#endif
}