/**
 * @author Sean Hobeck
 * @date 2026-01-21
 */
#ifndef ARCH_H
#define ARCH_H

/*! @uses cs_open. */
#include <capstone/capstone.h>

/** a data structure for pairing the cs_arch and cs_mode. */
typedef struct {
    cs_arch arch;
    cs_mode mode;
} arch_t;

/** @return the current architecture pair for capstone, gen. at compile time. */
arch_t
get_arch(void);
#endif /* ARCH_H */