/**
 * @author Sean Hobeck
 * @date 2026-07-27
 */
#include "arch.h"

/** @return the current architecture pair for capstone, gen. at compile time. */
arch_t
get_arch(void) {
#if defined(__amd64__) || defined(_M_AMD64)
    return (arch_t) { .arch = CS_ARCH_X86, .mode = CS_MODE_64 };
#endif
#if defined(__i386__) || defined(_M_IX86)
    return (arch_t) { .arch = CS_ARCH_X86, .mode = CS_MODE_32 };
#endif
#if defined(__aarch64__) || defined(_M_ARM64) 
    return (arch_t) { .arch = CS_ARCH_AARCH64, .mode = CS_MODE_ARM };
#endif
#if defined(__arm__) || defined(_M_ARM)
    return (arch_t) { .arch = CS_ARCH_ARM, .mode = CS_MODE_ARM };
#endif
}