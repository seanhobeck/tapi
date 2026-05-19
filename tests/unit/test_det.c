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
}

void
test_function_size_general(void) {
    /* arrange, act, assert. */
    size_t size = det_function_size((void*) general_function, 0x100);
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
    printf("correctly determined a general functions size: 0x%X!\n", size);
}

void
test_function_size_inline(void) {
    /* arrange, act, assert. */
    size_t size = det_function_size((void*) inline_fun, 0x24);
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
    assert(size == 0x1a);
#endif
    printf("correctly determined an inline asm. function size: 0x%X!\n", size);
}

int main(int argc, char** argv) {
    /* det_function_size unit tests. */
    printf("----src/int/det.c: 'det_function_size' unit tests----\n");
    test_function_size_general();
    test_function_size_inline();

    /* det_call_target unit tests. */
    return 0;
}