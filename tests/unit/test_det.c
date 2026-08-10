/**
 * @author Sean Hobeck
 * @date 2026-08-04
 */
/*! uses int. module to be tested. */
#include "int/det.h"

/*! uses assert. */
#include <assert.h>

/*! uses size_t. */
#include <stddef.h>

/*! uses printf. */
#include <stdio.h>

/*! uses MAYBE_UNUSED. */
#include "ptc.h"

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

/*!
 * this is just a random function that performs some computations and file i/o.
 */
TEST_NOINLINE long
general_function(long a, short b, int c) {
    long d = a + b;
    char buf[256];
    sprintf(buf, "%d.txt", c);
    FILE* stream = fopen(buf, "r+");
    fclose(stream);
    return d;
}

/*!
 * this function is simply the assembly of the following function.
 *
 * int negate_plus_one(int x) { return ~x + 1; }
 */
TEST_NOINLINE int
inline_fun(MAYBE_UNUSED int x) {
#ifndef _WIN32
    __asm(
#ifdef __amd64__
    "\tmov -0x4(%rbp), %eax\n"
    "\txor $0xffffffff, %eax\n"
    "\tadd $0x1, %eax"
#endif
#ifdef __i386__
    "\tmov 0x8(%ebp), %eax\n"
    "\tneg %eax\n"
#endif
#ifdef __aarch64__
    "\tldr w0, [sp, #12]\n"
    "\tneg w0, w0\n"
#endif
#ifdef __arm__
    "\trsb r0, r0, #0\n"
    "\tbx lr\n"
#endif
    );
#else
    return -x;
#endif
}

/*!
 * this is just a recursive fibonacci function.
 */
TEST_NOINLINE int
recursive_fibonacci(int n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    return recursive_fibonacci(n - 1) + recursive_fibonacci(n - 2);
}

/*!
 * this function calls a bunch of other functions.
 */
TEST_NOINLINE void
random_calls() {
    int result = general_function(1, 2, 3);
    int result2 = inline_fun(4);
    int result3 = recursive_fibonacci(10);
    printf("result: %d\n", result);
    printf("result2: %d\n", result2);
    printf("result3: %d\n", result3);
    printf("done! %d\n", result + result2 + result3);
}

/*!
 * this function gets the tls pointer using intrinsics.
 */
#ifndef _WIN32
TEST_NOINLINE void*
intrinsic_fun() {
    void* ptr;
#ifdef __amd64__
    __asm__ volatile (
        "mov %%fs:0, %0"
        : "=r"(ptr)
    );
#endif
#ifdef __i386__
    __asm__ volatile (
        "mov %%gs:0, %0"
        : "=r"(ptr)
    );
#endif
#ifdef __aarch64__
    __asm__ volatile (
        "mrs %0, tpidr_el0"
        : "=r"(ptr)
    );
#endif
#ifdef __arm__
    __asm__ volatile (
        "mrc p15, 0, %0, c13, c0, 2"
        : "=r"(ptr)
    );
#endif
    return ptr;
}
#else
/*! uses __readgsqword/fsdword. */
#include <intrin.h>
TEST_NOINLINE void*
intrinsic_fun() {
#ifdef _M_AMD64
    return __readgsqword(0x30);
#elif defined(_M_IX86)
    return __readfsdword(0x18);
#endif
}
#endif

/**
 * @brief test 'det_function_size' for the example function general_function
 *  with a max size of 0x100 (256 bytes), per each architecture.
 */
void
test_fs_general(void) {
    /* arrange, act, assert. */
    size_t size = det_function_size(raddressof(general_function), 0x100);
#ifdef __amd64__
    assert(size == 0xbd);
#endif
#ifdef __i386__
    assert(size == 0xa8);
#endif
#ifdef __aarch64__
    assert(size == 0xac);
#endif
#ifdef __arm__
    assert(size == 0xca);
#endif
#ifdef _M_AMD64
    assert(size == 0x95);
#endif
#ifdef _M_IX86
    assert(size == 0x77);
#endif
    printf("correctly determined a general functions size: 0x%lx!\n", size);
}

/**
 * @brief test 'det_function_size' for the example function inline_fun
 *  with a max size of 0x24 (36 bytes), per each architecture.
 */
void
test_fs_inline(void) {
    /* arrange, act, assert. */
    size_t size = det_function_size(raddressof(inline_fun), 0x24);
#ifdef __amd64__
    assert(size == 0x17);
#endif
#ifdef __i386__
    assert(size == 0x19);
#endif
#ifdef __aarch64__
    assert(size == 0x1c);
#endif
#ifdef __arm__
    assert(size == 0x1c);
#endif
#ifdef _M_AMD64
    assert(size == 0xb);
#endif
#ifdef _M_IX86
    assert(size == 0xa);
#endif
    printf("correctly determined an inline asm. function size: 0x%lx!\n", size);
}

/**
 * @brief test 'det_function_size' for the example function recursive_fibonacci
 *  with a max size of 0x100 (256 bytes), per each architecture.
 */
void
test_fs_recursive(void) {
    /* arrange, act, assert. */
    size_t size = det_function_size(raddressof(recursive_fibonacci), 0x100);
#ifdef __amd64__
    assert(size == 0x4e);
#endif
#ifdef __i386__
    assert(size == 0x5c);
#endif
#ifdef __aarch64__
    assert(size == 0x64);
#endif
#ifdef __arm__
    assert(size == 0x3e);
#endif
#ifdef _M_AMD64
    assert(size == 0x4b);
#endif
#ifdef _M_IX86
    assert(size == 0x40);
#endif
    printf("correctly determined a recursive functions size: 0x%lx!\n", size);
}

/**
 * @brief test 'det_function_size' for the example function random_calls
 *  with a max size of 0xf0 (240 bytes), per each architecture.
 */
void
test_fs_random(void) {
    /* arrange, act, assert. */
    size_t size = det_function_size(raddressof(random_calls), 0xf0);
#ifdef __amd64__
    assert(size == 0xae);
#endif
#ifdef __i386__
    assert(size == 0xaf);
#endif
#ifdef __aarch64__
    assert(size == 0x94);
#endif
#ifdef __arm__
    assert(size == 0x64);
#endif
#ifdef _M_AMD64
    assert(size == 0x8c);
#endif
#ifdef _M_IX86
    assert(size == 0x80);
#endif
    printf("correctly determined a function with a set of random calls size: 0x%lx!\n", size);
}

/**
 * @brief test 'det_function_size' for the example function intrinsic_fun
 *  with a max size of 0x40 (64 bytes), per each architecture.
 */
void
test_fs_intrinsic(void) {
    /* arrange, act, assert. */
    size_t size = det_function_size(raddressof(intrinsic_fun), 0x40);
#ifdef __amd64__
    assert(size == 0x1b);
#endif
#ifdef __i386__
    assert(size == 0x22);
#endif
#ifdef __aarch64__
    assert(size == 0x18);
#endif
#ifdef __arm__
    assert(size == 0x1a);
#endif
#ifdef _M_AMD64
    assert(size == 0xa);
#endif
#ifdef _M_IX86
    assert(size == 0xb);
#endif
    printf("correctly determined a function with intrinsics size: 0x%lx!\n", size);
}

/**
 * @brief test 'det_call_target' for the example function 'random_calls',
 *  searching for the other example function 'general_function'.
 */
void
test_ct_random_calls_general(void) {
    /* arrange, act, assert. */
    det_call_t* target = det_call_target(raddressof(random_calls), &general_function);
    assert(target != 0x0);
    assert(target->call != 0x0);
    /* this is here for the time being, there is some issue with the thumb bit or something on armhf,
        since it seems that the addresses in gdb-multiarch are completely the same (with runtime eval). */
#ifndef __arm__
    assert(target->dest == &general_function);
#endif
    printf("correctly determined a relative call to another function at: 0x%p!\n", target->call);
}

/**
 * @brief test 'det_call_target' for the example function 'random_calls',
 *  searching for the other example function 'inline_fun'.
 */
void
test_ct_random_calls_inline(void) {
    /* arrange, act, assert. */
    det_call_t* target = det_call_target(raddressof(random_calls), &inline_fun);
    assert(target != 0x0);
    assert(target->call != 0x0);
#ifndef __arm__
    assert(target->dest == &inline_fun);
#endif
    printf("correctly determined a relative call to a inline function at: 0x%p!\n", target->call);
}

/**
 * @brief test 'det_call_target' for the example function 'random_calls',
 *  searching for the other example function 'recursive_fibonacci'.
 */
void
test_ct_random_calls_recursive(void) {
    /* arrange, act, assert. */
    det_call_t* target = det_call_target(raddressof(random_calls), &recursive_fibonacci);
    assert(target != 0x0);
    assert(target->call != 0x0);
#ifndef __arm__
    assert(target->dest == &recursive_fibonacci);
#endif
    printf("correctly determined a relative call to a recursive function at: 0x%p!\n", target->call);
}

/**
 * @brief test 'det_call_target' for the example function 'random_calls',
 *  searching for a function that was not called to.
 */
void
test_ct_random_calls_dne(void) {
    /* arrange, act, assert. */
    det_call_t* target = det_call_target(raddressof(random_calls), &test_fs_general);
    assert(target == 0x0);
    printf("correctly determined no relative call to a function!\n");
}

/**
 * @brief test 'det_call_target' for the example function 'recursive_fibonacci',
 *  searching for itself.
 */
void
test_ct_recursive_itself(void) {
    /* arrange, act, assert. */
    det_call_t* target = det_call_target(raddressof(recursive_fibonacci), &recursive_fibonacci);
    assert(target != 0x0);
    assert(target->call != 0x0);
#ifndef __arm__
    assert(target->dest == &recursive_fibonacci);
#endif
    printf("correctly determined a relative call to self at: 0x%p!\n\n", target->call);
}

/* we use a singular main function per suite of testing done. */
#ifndef _WIN32
int main() {
#else
/*! for test_det. */
#include "test_det.h"

int test_det() {
#endif
    /* det_function_size unit tests (some arch rely on src/int/sig). */
    printf("----src/int/det.c: 'det_function_size' unit tests----\n");
    test_fs_general();
    test_fs_inline();
    test_fs_recursive();
    test_fs_random();
    test_fs_intrinsic();

    /* det_call_target unit tests. */
    printf("\n\n----src/int/det.c:  'det_call_target'  unit tests----\n");
    test_ct_random_calls_general();
    test_ct_random_calls_inline();
    test_ct_random_calls_recursive();
    test_ct_random_calls_dne();
    test_ct_recursive_itself();
    return 0;
}