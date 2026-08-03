/**
 * @author Sean Hobeck
 * @date 2026-08-03
 */
/*! uses int. module to be tested. */
#include "int/patch.h"

/*! uses arch_t, get_arch. */
#include "int/arch.h"

/*! uses assert. */
#include <assert.h>

/*! uses int32_t. */
#include <stdint.h>

/*! uses printf. */
#include <stdio.h>

/*! uses free. */
#include <stdlib.h>

/*! uses lnk_qr_thunk. */
#include "lnk.h"

/*! noinline macro. */
#ifdef _WIN32
#define TEST_NOINLINE __declspec(noinline)
#define raddressof(function) lnk_qr_thunk(&function)
#else
#define TEST_NOINLINE __attribute__((noinline))
#define raddressof(function) &function
#endif

/*! function we search for. */
TEST_NOINLINE int
#ifdef _WIN32
__cdecl
#endif
add(int a, int b) {
    return a + b;
};

/*! stub/mock replacement for add. */
TEST_NOINLINE int
add_stub(int a, int b) {
    return a * b;
}

/*! example function to patch. */
TEST_NOINLINE void
function() {
    int a = 200;
    int b = 300;
    int c = add(a, b);
    printf("%d\n", c);
};

/*! another example function to patch. */
TEST_NOINLINE void
function2() {
    int a = getc(stdin);
    int b = getchar();
    int c = add(a, b);
    int d = add(a, c);
    printf("%d\n", d);
}

/*! another example function to patch (this time with inline assembly). */
TEST_NOINLINE void
function3() {
    int a = getchar();
    int b = getchar();
    int c;
#ifndef _WIN32
    __asm__ volatile(
#ifdef __amd64__
    "mov %1, %%edi\n\t"
    "mov %2, %%esi\n\t"
    "call add\n\t"
    "mov %%eax, %0"
    : "=r"(c)
    : "r"(a), "r"(b)
    : "edi", "esi", "eax"
#endif
#ifdef __i386__
    "push %2\n\t"
    "push %1\n\t"
    "call add\n\t"
    "add $8, %%esp\n\t"
    "mov %%eax, %0"
    : "=r"(c)
    : "r"(a), "r"(b)
    : "eax", "esp"
#endif
#ifdef __aarch64__
    "mov w0, %w1\n\t"
    "mov w1, %w2\n\t"
    "bl add\n\t"
    "mov %w0, w0"
    : "=r"(c)
    : "r"(a), "r"(b)
    : "x0", "x1"
#endif
#ifdef __arm__
    "mov r0, %1\n\t"
    "mov r1, %2\n\t"
    "bl add\n\t"
    "mov %0, r0"
    : "=r"(c)
    : "r"(a), "r"(b)
    : "r0", "r1"
#endif
    );
#else
#ifdef _M_IX86
    __asm {
        mov eax, b
        push eax
        mov eax, a
        push eax
        call add
        add esp, 8
        mov c, eax
    }
#else
   
#endif
#endif
    printf("%d\n", c);
}

/**
 * @brief test 'patch_call_target' with a singular, simple call instruction in a function.
 */
void
test_f1_success() {
    /* arrange. */
    det_call_t* call = det_call_target(raddressof(function), &add);
    tapi_context_t* context = tapi_init();

    /* act. */
    int32_t result = patch_call_target(context, call, &add_stub);

    /* assert. */
    det_call_t* call2 = det_call_target(raddressof(function), &add);
    det_call_t* call3 = det_call_target(raddressof(function), &add_stub);
    assert(result == 0x1);
    assert(call2 == 0x0);
    assert(call3 != 0x0);
    free(call);
    free(call3);
    free(context);
    printf("successfully replaced a singular call!\n");
}

/**
 * @brief test 'patch_call_target' with two calls in a function with other external calls.
 */
void
test_f2_success() {
    /* arrange. */
    det_call_t* call = det_call_target(raddressof(function2), &add);
    tapi_context_t* context = tapi_init();

    /* act. */
    int32_t result = patch_call_target(context, call, &add_stub);
    det_call_t* call2 = det_call_target(raddressof(function2), &add);
    int32_t result2 = patch_call_target(context, call2, &add_stub);

    /* assert. */
    det_call_t* call3 = det_call_target(raddressof(function2), &add);
    det_call_t* call4 = det_call_target(raddressof(function2), &add_stub);
    det_call_t* call5 = det_call_target(raddressof(function2), &add_stub);
    assert(result == 0x1);
    assert(result2 == 0x1);
    assert(call3 == 0x0);
    assert(call4 != 0x0);
    assert(call5 != 0x0);
    free(call);
    free(call2);
    free(call4);
    free(call5);
    free(context);
    printf("successfully replaced two calls!\n");
}

/**
 * @brief test 'patch_call_target' with two calls in a function with other external calls.
 */
void
test_f3_success() {
    /* arrange. */
    det_call_t* call = det_call_target(raddressof(function3), &add);
    tapi_context_t* context = tapi_init();

    /* act. */
    int32_t result = patch_call_target(context, call, &add_stub);

    /* assert. */
    det_call_t* call2 = det_call_target(raddressof(function3), &add);
    det_call_t* call3 = det_call_target(raddressof(function3), &add_stub);
    assert(result == 0x1);
    assert(call2 == 0x0);
    assert(call3 != 0x0);
    free(call);
    free(call3);
    free(context);
    printf("successfully replaced a singular call within a inline assembly function!\n");
}

#ifndef _WIN32
int main() {
#else
/*! for test_patch. */
#include "test_patch.h"

int test_patch() {
#endif
#ifdef __amd64__
    printf("----src/int/patch.c: 'patch_call_target'(amd64) partial tests----\n");
#endif
#ifdef __i386__
    printf("----src/int/patch.c: 'patch_call_target'(i386) partial tests----\n");
#endif
#ifdef __aarch64__
    printf("----src/int/patch.c: 'patch_call_target'(aarch64) partial tests----\n");
#endif
#ifdef __arm__
    printf("----src/int/patch.c: 'patch_call_target'(armhf) partial tests----\n");
#endif
    test_f1_success();
    test_f2_success();
    test_f3_success();
    return 0;
}