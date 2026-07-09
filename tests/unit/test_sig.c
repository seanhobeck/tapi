/**
 * @author Sean Hobeck
 * @date 2026-06-26
 */
/*! uses int. module to be tested. */
#include "int/sig.h"

/*! uses arch_t, get_arch. */
#include "int/arch.h"

/*! uses assert. */
#include <assert.h>

/*! uses size_t. */
#include <stddef.h>

/*! uses printf. */
#include <stdio.h>

/*! uses strcpy. */
#include <string.h>

/*! uses cs_insn. */
#include <capstone/capstone.h>

/**
 * @brief test 'sig_compare' with an example cs_insn with a short mnemonic.
 */
void
test_strcmp_short(csh handle) {
    /* arrange. */
    char* a = "hello";
    cs_insn* b = cs_malloc(handle);
    memset(b->op_str, 0, sizeof(b->op_str));
    strcpy(b->mnemonic, "hello");

    /* act & assert. */
    assert(sig_compare(a, b));
    cs_free(b, 1);
    printf("correctly compared two strings without '?' of a short length!\n");
};

/**
 * @brief test 'sig_compare' with an example cs_insn with a medium mnemonic.
 */
void
test_strcmp_med(csh handle) {
    /* arrange. */
    char* a = "abcdefghijklmnop";
    cs_insn* b = cs_malloc(handle);
    memset(b->op_str, 0, sizeof(b->op_str));
    strcpy(b->mnemonic, "abcdefghijklmnop");

    /* act & assert. */
    assert(sig_compare(a, b));
    cs_free(b, 1);
    printf("correctly compared two strings without '?' of a medium length!\n");
};

/**
 * @brief test 'sig_compare' with an example cs_insn with a medium mnemonic and medium op_str.
 */
void
test_strcmp_med_with_op(csh handle) {
    /* arrange. */
    char* a = "abcdefghijklmnop qrstuvwxyz1234567890";
    cs_insn* b = cs_malloc(handle);
    strcpy(b->mnemonic, "abcdefghijklmnop");
    strcpy(b->op_str, "qrstuvwxyz1234567890");

    /* act & assert. */
    assert(sig_compare(a, b));
    cs_free(b, 1);
    printf("correctly compared two strings without '?' of a medium mnemonic and op_str length!\n");
};

/**
 * @brief test 'sig_compare' with an example cs_insn with a long mnemonic.
 */
void
test_strcmp_long(csh handle) {
    /* arrange. */
    char* a = "thisisanotherexamplestringabcde";
    cs_insn* b = cs_malloc(handle);
    strcpy(b->mnemonic, "thisisanotherexamplestringabcde");
    strcpy(b->op_str, "");

    /* act & assert. */
    assert(sig_compare(a, b));
    cs_free(b, 1);
    printf("correctly compared two strings without '?' of a long length!\n");
};

/**
 * @brief test 'sig_compare' with an example cs_insn with a long mnemonic and long op_str.
 */
void
test_strcmp_long_with_op(csh handle) {
    /* arrange. */
    char* a = "thisisanotherexamplestringabcde qrstuvwxyz1234567890aaaaaabbbbbbccccccdddddd";
    cs_insn* b = cs_malloc(handle);
    strcpy(b->mnemonic, "thisisanotherexamplestringabcde");
    strcpy(b->op_str, "qrstuvwxyz1234567890aaaaaabbbbbbccccccdddddd");

    /* act & assert. */
    assert(sig_compare(a, b));
    cs_free(b, 1);
    printf("correctly compared two strings without '?' of a long mnemonic and op_str length!\n");
};

/**
 * @brief test 'sig_compare' with an example cs_insn with a '?' in the mnemonic.
 */
void
test_strcmp_mnemonic(csh handle) {
    /* arrange. */
    char* a = "abc?? def";
    cs_insn* b = cs_malloc(handle);
    strcpy(b->mnemonic, "abc11");
    strcpy(b->op_str, "def");

    /* act & assert. */
    assert(sig_compare(a, b));
    cs_free(b, 1);
    printf("correctly compared two strings with a '?' in the mnemonic!\n");
};

/**
 * @brief test 'sig_compare' with an example cs_insn with a '?' in the mnemonic.
 */
void
test_strcmp_mnemonic_min(csh handle) {
    /* arrange. */
    char* a = "abc???????? def";
    cs_insn* b = cs_malloc(handle);
    strcpy(b->mnemonic, "abc1");
    strcpy(b->op_str, "def");

    /* act & assert. */
    assert(sig_compare(a, b));
    cs_free(b, 1);
    printf("correctly compared two strings with a min of 1 '?' in the mnemonic!\n");
};

/**
 * @brief test 'sig_compare' with an example cs_insn with a '?' in the mnemonic.
 */
void
test_strcmp_mnemonic_max(csh handle) {
    /* arrange. */
    char* a = "abc???????? def";
    cs_insn* b = cs_malloc(handle);
    strcpy(b->mnemonic, "abc11111111");
    strcpy(b->op_str, "def");

    /* act & assert. */
    assert(sig_compare(a, b));
    cs_free(b, 1);
    printf("correctly compared two strings with a max of 8 '?' in the mnemonic!\n");
};

/**
 * @brief test 'sig_compare' with an example cs_insn with a '?' in the op_str.
 */
void
test_strcmp_op_str(csh handle) {
    /* arrange. */
    char* a = "abc def???";
    cs_insn* b = cs_malloc(handle);
    strcpy(b->mnemonic, "abc");
    strcpy(b->op_str, "defghi");

    /* act & assert. */
    assert(sig_compare(a, b));
    cs_free(b, 1);
    printf("correctly compared two strings with a '?' in the op_str!\n");
};

/**
 * @brief test 'sig_compare' with an example cs_insn with a '?' in the op_str.
 */
void
test_strcmp_op_str_min(csh handle) {
    /* arrange. */
    char* a = "abc def????????";
    cs_insn* b = cs_malloc(handle);
    strcpy(b->mnemonic, "abc");
    strcpy(b->op_str, "defg");

    /* act & assert. */
    assert(sig_compare(a, b));
    cs_free(b, 1);
    printf("correctly compared two strings with a min of 1 '?' in the op_str!\n");
};

/**
 * @brief test 'sig_compare' with an example cs_insn with a '?' in the op_str.
 */
void
test_strcmp_op_str_max(csh handle) {
    /* arrange. */
    char* a = "abc def????????";
    cs_insn* b = cs_malloc(handle);
    strcpy(b->mnemonic, "abc");
    strcpy(b->op_str, "defghijklmn");

    /* act & assert. */
    assert(sig_compare(a, b));
    cs_free(b, 1);
    printf("correctly compared two strings with a min of 8 '?' in the op_str!\n");
};

/*! @note below are some tests for the specific common sequences for armhf or aarch64. */

/* ... */
void
test_armhf_chk_1(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "push");
    strcpy(insn->op_str, "{r7, r11, lr}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (push: 1) instruction!\n");
};

/* ... */
void
test_armhf_chk_2(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "push");
    strcpy(insn->op_str, "{r7, r11, r4, r8}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (push: 2) instruction!\n");
};

/* ... */
void
test_armhf_chk_3(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "push");
    strcpy(insn->op_str, "{r7, r11, r4, r8, r10}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (push: 3) instruction!\n");
};

/* ... */
void
test_armhf_chk_4(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "push");
    strcpy(insn->op_str, "{r7}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (push: 4) instruction!\n");
};

/* ... */
void
test_armhf_chk_5(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "sub");
    strcpy(insn->op_str, "sp, sp, #0x12");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (sub: 1) instruction!\n");
};

/* ... */
void
test_armhf_chk_6(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "sub");
    strcpy(insn->op_str, "sp, sp, #0x1234");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (sub: 2) instruction!\n");
};

/* ... */
void
test_armhf_chk_7(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "sub");
    strcpy(insn->op_str, "sp, sp, $0x10");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_UNRECOGNIZED);
    cs_free(insn, 1);
    printf("correctly checked fake armhf (sub: 3?) instruction!\n");
};

/* ... */
void
test_armhf_chk_8(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "stmdb");
    strcpy(insn->op_str, "sp!, {r11}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (stmdb: 1) instruction!\n");
};

/* ... */
void
test_armhf_chk_9(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "stmdb");
    strcpy(insn->op_str, "sp!, {r6, lr}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (stmdb: 2) instruction!\n");
};

/* ... */
void
test_armhf_chk_10(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "stmdb");
    strcpy(insn->op_str, "sp!, {r11, r9, r4, lr}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (stmdb: 3) instruction!\n");
};

/* ... */
void
test_armhf_chk_11(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "stmdb");
    strcpy(insn->op_str, "sp!, {r11, r7, r8}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (stmdb: 4) instruction!\n");
};

/* ... */
void
test_armhf_chk_12(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "pop");
    strcpy(insn->op_str, "{r11, r7, r8}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (pop: 1) instruction!\n");
};

/* ... */
void
test_armhf_chk_13(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "pop");
    strcpy(insn->op_str, "{r11, r7, pc}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (pop: 2) instruction!\n");
};

/* ... */
void
test_armhf_chk_14(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "pop");
    strcpy(insn->op_str, "{r9, pc}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (pop: 3) instruction!\n");
};

/* ... */
void
test_armhf_chk_15(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "pop");
    strcpy(insn->op_str, "{r8, r7, r9, pc}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (pop: 4) instruction!\n");
};

/* ... */
void
test_armhf_chk_16(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "add");
    strcpy(insn->op_str, "sp, sp, #0x10");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (add: 1) instruction!\n");
};

/* ... */
void
test_armhf_chk_17(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "add");
    strcpy(insn->op_str, "sp, sp, $3");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_UNRECOGNIZED);
    cs_free(insn, 1);
    printf("correctly checked fake armhf (add: 2?) instruction!\n");
};

/* ... */
void
test_armhf_chk_18(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "ldmia");
    strcpy(insn->op_str, "sp!, {r11, r5}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (ldmia: 1) instruction!\n");
};

/* ... */
void
test_armhf_chk_19(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "ldmia");
    strcpy(insn->op_str, "sp!, {r11, r5}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (ldmia: 2) instruction!\n");
};

/* ... */
void
test_armhf_chk_20(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "ldmia");
    strcpy(insn->op_str, "sp!, {r11, pc}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (ldmia: 3) instruction!\n");
};

/* ... */
void
test_armhf_chk_21(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "ldmia");
    strcpy(insn->op_str, "sp!, {r11, r5, pc}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (ldmia: 4) instruction!\n");
};

/* ... */
void
test_armhf_chk_22(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "ldmia");
    strcpy(insn->op_str, "sp!, {r11, r5, r7, r4}");

    /* act & assert. */
    assert(sig_armhf_chk(insn) == SIG_ARMHF_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked armhf (ldmia: 5) instruction!\n");
};

/* ... */
void
test_aarch64_chk_1(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "sub");
    strcpy(insn->op_str, "sp, sp, #0x123");

    /* act & assert. */
    assert(sig_aarch64_chk(insn) == SIG_AARCH64_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked aarch64 (sub: 1) instruction!\n");
};

/* ... */
void
test_aarch64_chk_2(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "sub");
    strcpy(insn->op_str, "sp, sp, #0x1434");

    /* act & assert. */
    assert(sig_aarch64_chk(insn) == SIG_AARCH64_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked aarch64 (sub: 2) instruction!\n");
};

/* ... */
void
test_aarch64_chk_3(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "stp");
    strcpy(insn->op_str, "r11, r6, [sp, #0x1234]");

    /* act & assert. */
    assert(sig_aarch64_chk(insn) == SIG_AARCH64_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked aarch64 (stp: 1) instruction!\n");
};

/* ... */
void
test_aarch64_chk_4(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "stp");
    strcpy(insn->op_str, "r13, r5, [sp, #-0x1337]!");

    /* act & assert. */
    assert(sig_aarch64_chk(insn) == SIG_AARCH64_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked aarch64 (stp: 2) instruction!\n");
};

/* ... */
void
test_aarch64_chk_5(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "paciasp");
    strcpy(insn->op_str, "");

    /* act & assert. */
    assert(sig_aarch64_chk(insn) == SIG_AARCH64_PROLOGUE);
    cs_free(insn, 1);
    printf("correctly checked aarch64 (paciasp) instruction!\n");
};

/* ... */
void
test_aarch64_chk_6(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "add");
    strcpy(insn->op_str, "sp, sp, #0x1434");

    /* act & assert. */
    assert(sig_aarch64_chk(insn) == SIG_AARCH64_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked aarch64 (add: 1) instruction!\n");
};

/* ... */
void
test_aarch64_chk_7(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "add");
    strcpy(insn->op_str, "sp, sp, #0x3291");

    /* act & assert. */
    assert(sig_aarch64_chk(insn) == SIG_AARCH64_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked aarch64 (add: 2) instruction!\n");
};

/* ... */
void
test_aarch64_chk_8(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "ldp");
    strcpy(insn->op_str, "r9, r3, [sp, #0x6537]");

    /* act & assert. */
    assert(sig_aarch64_chk(insn) == SIG_AARCH64_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked aarch64 (ldp: 1) instruction!\n");
};

/* ... */
void
test_aarch64_chk_9(csh handle) {
    /* arrange. */
    cs_insn* insn = cs_malloc(handle);
    strcpy(insn->mnemonic, "autiasp");
    strcpy(insn->op_str, "");

    /* act & assert. */
    assert(sig_aarch64_chk(insn) == SIG_AARCH64_EPILOGUE);
    cs_free(insn, 1);
    printf("correctly checked aarch64 (autiasp) instruction!\n");
};

int main() {
    /* open with architecture etc... */
    cs_arch arch;
    cs_mode mode;
#ifdef __amd64__
    arch = CS_ARCH_X86; mode = CS_MODE_64;
#endif
#ifdef __i386__
    arch = CS_ARCH_X86; mode = CS_MODE_32;
#endif
#ifdef __aarch64__
    arch = CS_ARCH_AARCH64; mode = CS_MODE_ARM;
#endif
#ifdef __arm__
    arch = CS_ARCH_ARM; mode = CS_MODE_ARM;
#endif
    csh handle;

    /* detect if we need to use thumb based on the thumb bit. */
    void* address = (void*) &main;
    bool is_thumb = mode == CS_MODE_ARM && (uintptr_t)address & 1u;
    if (is_thumb) {
        address = (void*)((uintptr_t)address & ~1u);
        mode = CS_MODE_THUMB;
    }
    cs_open(arch, mode, &handle);
    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

    /* run tests. */
    printf("----src/int/sig.c: 'sig_compare' unit tests----\n");
    test_strcmp_short(handle);
    test_strcmp_med(handle);
    test_strcmp_med_with_op(handle);
    test_strcmp_long(handle);
    test_strcmp_long_with_op(handle);
    test_strcmp_mnemonic(handle);
    test_strcmp_mnemonic_min(handle);
    test_strcmp_mnemonic_max(handle);
    test_strcmp_op_str(handle);
    test_strcmp_op_str_max(handle);
    test_strcmp_op_str_min(handle);

    printf("\n----src/int/sig.c: 'sig_armhf_chk' unit tests----\n");
    test_armhf_chk_1(handle);
    test_armhf_chk_2(handle);
    test_armhf_chk_3(handle);
    test_armhf_chk_4(handle);
    test_armhf_chk_5(handle);
    test_armhf_chk_6(handle);
    test_armhf_chk_7(handle);
    test_armhf_chk_8(handle);
    test_armhf_chk_9(handle);
    test_armhf_chk_10(handle);
    test_armhf_chk_11(handle);
    test_armhf_chk_12(handle);
    test_armhf_chk_13(handle);
    test_armhf_chk_14(handle);
    test_armhf_chk_15(handle);
    test_armhf_chk_16(handle);
    test_armhf_chk_17(handle);
    test_armhf_chk_18(handle);
    test_armhf_chk_19(handle);
    test_armhf_chk_20(handle);
    test_armhf_chk_21(handle);
    test_armhf_chk_22(handle);

    printf("\n----src/int/sig.c: 'sig_aarch64_chk' unit tests----\n");
    test_aarch64_chk_1(handle);
    test_aarch64_chk_2(handle);
    test_aarch64_chk_3(handle);
    test_aarch64_chk_4(handle);
    test_aarch64_chk_5(handle);
    test_aarch64_chk_6(handle);
    test_aarch64_chk_7(handle);
    test_aarch64_chk_8(handle);
    test_aarch64_chk_9(handle);

    /* close the capstone engine. */
    cs_close(&handle);
    return 0;
};