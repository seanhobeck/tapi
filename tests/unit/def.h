/**
 * @author Sean Hobeck
 * @date 2026-10-23
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

#ifdef __linux__
#include <endian.h>
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>

/* redef. */
#define htole16 OSSwapHostToLittleInt16
#define htole32 OSSwapHostToLittleInt32
#define htole64 OSSwapHostToLittleInt64
#define le16toh OSSwapLittleToHostInt16
#define le32toh OSSwapLittleToHostInt32
#define le64toh OSSwapLittleToHostInt64
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <sys/endian.h>
#elif defined(_WIN32)
#define htole16(x) ((uint16_t)(x))
#define htole32(x) ((uint32_t)(x))
#define htole64(x) ((uint64_t)(x))
#define le16toh(x) ((uint16_t)(x))
#define le32toh(x) ((uint32_t)(x))
#define le64toh(x) ((uint64_t)(x))
#else
#endif
#endif /* DEF_H. */