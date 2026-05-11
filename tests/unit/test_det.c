/**
 * @author Sean Hobeck
 * @date 2026-05-11
 */
/*! @uses int. module to be tested. */
#include "../src/int/det.h"

/* todo; add winapi equiv (no inline for x64 unfortunately). */
int inline_fun(int x) {
    __asm(
#ifdef __amd64__
    "push %rbp"
    "mov %rsp, %rbp"
    "mov %edi, -0x4(%rbp)"
    "mov -0x4(%rbp), %eax"
    "xor $0xffffffff, %eax"
    "add $0x1, %eax"
    "pop %rbp"
    "ret"
#elifdef __i386__
    "push %ebp"
    "mov %esp, %ebp"
    "mov 0x8(%ebp), %eax"
    "xor $0xffffffff, %eax"
    "add $0x1, %eax"
    "pop %ebp"
    "ret"
#elifdef __aarch64__
    "sub sp, sp, #0x10"
    "str w0, [sp, #12]"
    "ldr w0, [sp, #12]"
    "neg w0, w0"
    "add sp, sp, #0x10"
    "ret"
#elifdef __arm__
    "sub sp, sp, #0x10"
    "ldr w8, [sp, #12]"
    "mvn w8, w8"
    "add w0, w8, #0x1"
    "add sp, sp, #0x10"
    "ret"
#endif
    );
}

static void
test_funsz_inlines_asm() {
    /* act, arrange, assert. */
}

int main(int argc, char** argv) {
    return 0;
}