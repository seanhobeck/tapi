/**
 * @author Sean Hobeck
 * @date 2026-10-20
 */
#ifndef DEF_H
#define DEF_H
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/* posix-test-compliance. */
#ifdef __gnu_linux__
#define test_maybe_unused __attribute__((unused))
#else
#define test_maybe_unused
#endif

/*! noinline macro. */
#ifdef _WIN32
#include <windows.h>
#include "int/lnk.h"
#define test_noinline __declspec(noinline)
#define test_addressof(function) lnk_qr_thunk(&function)
#else
#ifdef __linux__
#include <unistd.h>
#endif
#define test_noinline __attribute__((noinline))
#define test_addressof(function) &function
#endif
#endif /* DEF_H. */