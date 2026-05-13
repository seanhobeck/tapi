/**
 * @author Sean Hobeck
 * @date 2026-05-11
 */
/*! @uses int. module to be tested. */
#include "int/det.h"

/*! @uses assert. */
#include <assert.h>

/*! @uses size_t. */
#include <stddef.h>

/*! @uses printf. */
#include <stdio.h>

/*!
 * this is just a random function that performs some computations and file i/o.
 */
long
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
int
inline_fun(int x) {
    __asm(
#ifdef __amd64__
    "\tmov -0x4(%rbp), %eax\n"
    "\txor $0xffffffff, %eax\n"
    "\tadd $0x1, %eax"
#endif
#ifdef __i386__
    "\txor $0xffffffff, %eax\n"
    "\tadd $0x1, %eax\n"
#endif
#ifdef __aarch64__
    "\tstr w0, [sp, #12]\n"
    "\tldr w0, [sp, #12]\n"
    "\tneg w0, w0\n"
#endif
#ifdef __arm__
    "\tstr w0, [sp, #12]\n"
    "\tldr w0, [sp, #12]\n"
    "\tneg w0, w0\n"
#endif
    );
}

void
test_function_size_general(void) {
    /* arrange, act, assert. */
    size_t size = det_function_size((void*) general_function, 0x100);
#ifdef __amd64__
    assert(size == 0xc1);
#endif
#ifdef __i386__
    assert(size == 0xac);
#endif
#ifdef __aarch64__
    /* todo; add this assertation. */
#endif
#ifdef __arm__
    /* todo; add this assertation. */
#endif
    printf("correctly determined a general functions size!\n");
}

void
test_function_size_inline(void) {
    /* arrange, act, assert. */
    size_t size = det_function_size((void*) inline_fun, 40u);
#ifdef __amd64__
    assert(size == 0x1b);
#endif
#ifdef __i386__
    assert(size == 0x1b);
#endif
#ifdef __aarch64__
    assert(size == 0x20);
#endif
#ifdef __arm__
    /* todo; add this assertation. */
#endif
    printf("correctly determined a inline asm. function size!\n");
}

int main(int argc, char** argv) {
    /* det_function_size unit tests. */
    printf("----src/int/det.c: 'det_function_size' unit tests----\n");
    test_function_size_general();
    test_function_size_inline();

    /* det_call_target unit tests. */
    return 0;
}