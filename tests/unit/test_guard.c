/**
 * @author Sean Hobeck
 * @date 2026-05-23
 */
/*! @uses int. module to be tested. */
#include "int/guard.h"

/*! @uses assert. */
#include <assert.h>

/*! @uses size_t. */
#include <stddef.h>

/*! @uses printf. */
#include <stdio.h>

/*! @uses strcpy. */
#include <string.h>

/*! @uses VirtualQuery for windows. */
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
long
some_function(int a, short b, char** c) {
    long d = a + b * **c;
    printf("this is the result of this function, %d\n", d);
    short e = d * a;
    long f = (long)e - (long)&(*c);
    return f;
};

int main(int argc, char** argv) {
    /* guard_create unit tests. */
    printf("----src/int/guard.c: 'guard_create' unit tests----\n");

    /* arrange & act. */
    void* address = &some_function;
    guard_create(address, 0xf0);

    /* assert. */
#ifdef __linux__
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
#endif
#ifdef _WIN32
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(address, &mbi, sizeof(mbi)) != 0) {
        if (mbi.Protect & PAGE_EXECUTE_READWRITE) {
            printf("successful page permission(win32) change!\n");
        }
        else assert("failed, page permissions not correct(win32)!\n");
    }
#endif
    fclose(ptr);
    return 0;
};