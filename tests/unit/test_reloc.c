/**
 * @author Sean Hobeck
 * @date 2026-09-05
 */
#define _DEFAULT_SOURCE
/*! uses det_call_t, det_call_target. */
#include "int/det.h"

/*! uses the module to be included. */
#include "int/reloc.h"

/*! uses assert. */
#include <assert.h>

/*! uses printf. */
#include <stdio.h>

/*! uses uint8/16/32/64_t. */
#include <stdint.h>

/*! uses true, false. */
#include <stdbool.h>

/*! uses memcmp. */
#include <string.h>

#ifdef __linux__
/*! uses htole16/32/64. */
#include <endian.h>
#elif defined(__APPLE__)
/*! uses OSSwapHostToLittleInt16/32/64. */
#include <libkern/OSByteOrder.h>

/* redef. */
#define htole16 OSSwapHostToLittleInt16
#define htole32 OSSwapHostToLittleInt32
#define htole64 OSSwapHostToLittleInt64
#define le16toh OSSwapLittleToHostInt16
#define le32toh OSSwapLittleToHostInt32
#define le64toh OSSwapLittleToHostInt64
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
/*! uses htole16/32/64. */
#include <sys/endian.h>
#elif defined(_WIN32)
/*! uses uint16/32/64_t. */
#include <stdint.h>

/* little endian on all archs. */
#define htole16(x) ((uint16_t)(x))
#define htole32(x) ((uint32_t)(x))
#define htole64(x) ((uint64_t)(x))
#define le16toh(x) ((uint16_t)(x))
#define le32toh(x) ((uint32_t)(x))
#define le64toh(x) ((uint64_t)(x))
#else
#endif

/*! noinline macro. */
#ifdef _WIN32
#define TEST_NOINLINE __declspec(noinline)
#define TEST_ADDRESSOF(function) lnk_qr_thunk(&function)
#else
#define TEST_NOINLINE __attribute__((noinline))
#define TEST_ADDRESSOF(function) &function
#endif

/* calculating some random value. */
TEST_NOINLINE int
some_other_function(int a, int b) {
    return a * b * 10u;
};

/* calculating the value for a function. */
TEST_NOINLINE int
calculate_value(int a, int b) {
    int c = a + b;
    int d = a * b * b;
    return some_other_function(c, d) / 4u;
};

/* a far target example function. */
TEST_NOINLINE void
function_far() {};

/* the first reloc made for the test. */
static reloc_t* first;

/**
 * @brief test 'reloc_find' for the first time with a call to a function.
 */
void
test_reloc_find_first(void) {
    /* arrange. */
    det_call_t* call = det_call_target(TEST_ADDRESSOF(calculate_value), \
        TEST_ADDRESSOF(some_other_function));

    /* act. */
    first = reloc_find(call->call, TEST_ADDRESSOF(function_far), call->is_thumb, false);

    /* assert. */
    assert(call != 0x0);
    assert(first != 0x0);
#if defined(__amd64__) || defined(_M_AMD64)
    /* endbr64 / movabs r11, imm64 / jmp r11. */
    uint8_t bytes[17u] = {
        0xf3, 0x0f, 0x1e, 0xfa, \
        0x49, 0xbb, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
        0x41, 0xff, 0xe3
    };
    *(uint64_t*)(bytes + 6u) = (uint64_t)TEST_ADDRESSOF(function_far);
    assert(memcmp(first->bytes, bytes, first->size) == 0);
#endif
#if defined(__aarch64__) || defined(_M_ARM64)
    /* ldr x17, #8 / br x17 / .quad target. */
    uint8_t bytes[8u] = {
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
    };
    *(uint32_t*)(bytes) = htole32(0x58000051);
    *(uint32_t*)(bytes + 4u) = htole32(0xd61f0220);
    assert(memcmp(first->bytes, bytes, 8u) == 0);
#endif
#if defined(__i386) || defined(_M_IX86)
    /* mov eax, imm32 / jmp eax. */
    uint8_t bytes[7u] = {
        0xb8, 0x0, 0x0, 0x0, 0x0, 0xff, 0xe0
    };
    *(uint32_t*)(bytes + 1u) = (uint32_t)TEST_ADDRESSOF(function_far);
    assert(memcmp(first->bytes, bytes, first->size) == 0);
#endif
#if defined(__arm__) || defined(_M_ARM)
    /* two different forms of shell-code for each. */
    if (call->is_thumb) {
        uint8_t bytes[8u] = {
            0xdf, 0xf8, 0x04, 0xc0, /* ldr.w ip, [pc, #4] / ... */
            0x60, 0x47, /* bx ip / ... */
            0x0, 0xbf, /* nop / .dword target. */
        };
        assert(memcmp(first->bytes, bytes, 8u) == 0);
    }
    else {
        /* ldr r12, [pc + 0x8] / bx r12 / .dword target. */
        uint8_t bytes[12u] = {
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
        };
        *(uint32_t*)(bytes) = htole32(0xe59fc000);
        *(uint32_t*)(bytes + 4u) = htole32(0xe12fff1c);
        assert(memcmp(first->bytes, bytes, 8u) == 0);
    }
#endif
    printf("correctly made a relocation with the correct bytes, and within relative range.\n");
};

/**
 * @brief test 'reloc_find' for the second time with a call to an identical function.
 */
void
test_reloc_find_second(void) {
    /* arrange. */
    det_call_t* call = det_call_target(TEST_ADDRESSOF(calculate_value), \
        TEST_ADDRESSOF(some_other_function));

    /* act & assert. */
    reloc_t* reloc = reloc_find(call->call, TEST_ADDRESSOF(function_far), call->is_thumb, false);
    assert(call != 0x0);
    assert(reloc != 0x0);
    assert(reloc == first);
    printf("correctly found a existing reloc.\n");
};

/* we use a singular main function per suite of testing done. */
#ifndef _WIN32
int main() {
#else
/*! for test_reloc. */
#include "test_reloc.h"

int test_reloc() {
#endif
    /* reloc_make tests. */
    printf("----src/int/reloc.c: 'make/find' unit tests----\n");
    test_reloc_find_first();
    test_reloc_find_second();

#ifdef _WIN32
    /* reloc_make_custom tests. */
    printf("\n\n----src/int/reloc.c:  'make_custom'  unit tests----\n");
    printf("n/a due to duplicated code.\n");
#endif
    return 0;
}