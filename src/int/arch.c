/**
 * @author Sean Hobeck
 * @date 2026-01-21
 */
#include "arch.h"

/** @return the current architecture pair for capstone, gen. at compile time. */
arch_t
get_arch(void) {
#ifdef __amd64__
    return (arch_t) { .arch = CS_ARCH_X86, .mode = CS_MODE_64 };
#endif
#ifdef __i386__
    return (arch_t) { .arch = CS_ARCH_X86, .mode = CS_MODE_32 };
#endif
#ifdef __aarch64__
    return (arch_t) { .arch = CS_ARCH_AARCH64, .mode = CS_MODE_ARM };
#endif
#ifdef __arm__
    return (arch_t) { .arch = CS_ARCH_ARM, .mode = CS_MODE_ARM };
#endif
}