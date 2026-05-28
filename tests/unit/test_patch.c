/**
* @author Sean Hobeck
 * @date 2026-05-28
 */
/*! @uses int. module to be tested. */
#include "int/patch.h"

/*! @uses arch_t, get_arch. */
#include "int/arch.h"

/*! @uses assert. */
#include <assert.h>

/*! @uses int32_t. */
#include <stdint.h>

/*! @uses printf. */
#include <stdio.h>

/*! @uses free. */
#include <stdlib.h>

/*! function we search for. */
int
add(int a, int b) {
    return a + b;
};

/*! stub/mock replacement for add. */
int
add_stub(int a, int b) {
    return a * b;
}

/*! example function to patch. */
void
function() {
    int a = 200;
    int b = 300;
    int c = add(a, b);
    printf("%d\n", c);
};

/*! another example function to patch. */
void
function2() {
    int a = getc(stdin);
    int b = getchar();
    int c = add(a, b);
    int d = add(a, c);
    printf("%d\n", d);
}

/*! another example function to patch (this time with inline assembly). */
void
function3() {
    int a = getchar();
    int b = getchar();
    int c;
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
    printf("%d\n", c);
}

/**
 * @brief test 'patch_call_target' with a singular, simple call instruction in a function.
 */
void
test_f1_success() {
    /* arrange. */
    det_call_t* call = det_call_target(&function, &add);

    /* act. */
    int32_t result = patch_call_target(call, &add_stub);

    /* arrange. */
    det_call_t* call2 = det_call_target(&function, &add);
    det_call_t* call3 = det_call_target(&function, &add_stub);
    assert(result == 0x1);
    assert(call2 == 0x0);
    assert(call3 != 0x0);
    free(call);
    free(call3);
    printf("successfully replaced a singular call!\n");
}

/**
 * @brief test 'patch_call_target' with two calls in a function with other external calls.
 */
void
test_f2_success() {
    /* arrange. */
    det_call_t* call = det_call_target(&function2, &add);

    /* act. */
    int32_t result = patch_call_target(call, &add_stub);
    det_call_t* call2 = det_call_target(&function2, &add);
    int32_t result2 = patch_call_target(call2, &add_stub);

    /* arrange. */
    det_call_t* call3 = det_call_target(&function2, &add);
    det_call_t* call4 = det_call_target(&function2, &add_stub);
    det_call_t* call5 = det_call_target(&function2, &add_stub);
    assert(result == 0x1);
    assert(result2 == 0x1);
    assert(call3 == 0x0);
    assert(call4 != 0x0);
    assert(call5 != 0x0);
    free(call);
    free(call2);
    free(call4);
    free(call5);
    printf("successfully replaced two calls!\n");
}

/**
 * @brief test 'patch_call_target' with two calls in a function with other external calls.
 */
void
test_f3_success() {
    /* arrange. */
    det_call_t* call = det_call_target(&function3, &add);

    /* act. */
    int32_t result = patch_call_target(call, &add_stub);

    /* arrange. */
    det_call_t* call2 = det_call_target(&function3, &add);
    det_call_t* call3 = det_call_target(&function3, &add_stub);
    assert(result == 0x1);
    assert(call2 == 0x0);
    assert(call3 != 0x0);
    free(call);
    free(call3);
    printf("successfully replaced a singular call within a inline assembly function!\n");
}

int main(int argc, char** argv) {
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