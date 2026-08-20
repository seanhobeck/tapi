/**
 * @author Sean Hobeck
 * @date 2026-08-20
 */
/*! uses int. module to be tested. */
#include "int/guard.h"

/*! uses tapi_context_t. */
#include <tapi/tapi.h>

/*! uses assert. */
#include <assert.h>

/*! uses size_t. */
#include <stddef.h>

/*! uses calloc, free. */
#include <stdlib.h>

/*! uses printf. */
#include <stdio.h>

/*! uses strcpy. */
#include <string.h>

/*! uses lnk_qr_thunk. */
#include "int/lnk.h"

/*! uses det_function_size. */
#include "int/det.h"

/*! noinline macro. */
#ifdef _WIN32
#define TEST_NOINLINE __declspec(noinline)
#define TEST_ADDRESSOF(function) lnk_qr_thunk(&function)
#else
#define TEST_NOINLINE __attribute__((noinline))
#define raddressof(function) &function
#endif

/*! uses VirtualQuery for windows. */
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include <unistd.h>
#endif

/*!
 * this is a function that results in a large amount of space allocated \
 *  due to casts, calculations and calls.
 */
TEST_NOINLINE long
some_function(int a, short b, char** c) {
    long d = a + b * **c;
    printf("this is the result of this function, %ld\n", d);
    short e = d * a;
    long f = (long)e - (long)&(*c);
    return f;
};

#ifndef _WIN32
int main() {
#else
/*! for test_guard. */
#include "test_guard.h"

int test_guard() {
#endif
    /* guard_create unit tests. */
    printf("----src/int/guard.c: 'guard_create' unit tests----\n");
    tapi_context_t* context = calloc(1u, sizeof *context);
    context->guards = tapi_dyna_create();

    /* arrange & act. */
#ifndef _WIN32
    void* address = &some_function;
#else
    void* address = lnk_qr_thunk(&some_function);
#endif
    size_t size = det_function_size(address, 0x100);
    guard_create(context, address, size);

    /* assert. */
#ifdef __linux__
    assert(context->guards->length == 1u);

    /* open this processes maps. */
    FILE* ptr = fopen("/proc/self/maps", "r");
    if (!ptr) {
        fprintf(stderr, "failed to open /proc/self/maps! invalid permissions?\n");
        return 1;
    }

    /* read the lines of the maps. */
    char line[1024];
    while (fgets(line, sizeof(line), ptr)) {
        unsigned long start, end;
        char perms[5];
        if (sscanf(line, "%lx-%lx %4s", &start, &end, perms) == 3) {
            if ((unsigned long)address >= start && (unsigned long)address <= end) {
                /* read, write and execute. */
                if (perms[0] == 'r' && perms[1] == 'w' && perms[2] == 'x') {
                    printf("successful page permission(linux) change!\n");
                }
                else assert("failed, page permissions not correct(linux)!\n");
            }
        }
    }
    fclose(ptr);
#endif
#ifdef _WIN32
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(address, &mbi, sizeof(mbi)) != 0) {
        if (mbi.Protect & PAGE_EXECUTE_READWRITE) {
            printf("successful page permission(win32) change!\n\n");
        }
        else assert("failed, page permissions not correct(win32)!\n");
    }
#endif
    return 0;
};