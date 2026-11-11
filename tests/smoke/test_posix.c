/**
 * @author Sean Hobeck
 * @date 2026-11-11
 */
#include "def.h"
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

test_noinline int
target_function_1(int x) {
    void* a = malloc(x);
    if (a == 0x0) return -1;
    free(a);
    void* b = calloc(1u, x);
    if (b == 0x0) return -2;
    void* c = realloc(b, 12u);
    if (c == 0x0) return -3;
    free(c);
    return 0;
}

test_noinline int
target_function_2(char* string) {
    size_t strlen_retval = strlen(string);
    if (strlen_retval == 0) return -1;
    int strcmp_retval = strcmp(string, "1234567");
    if (strcmp_retval == 0) return -2;
    int strncmp_retval = strncmp(string, "1234567890123", 10);
    if (strncmp_retval == 0) return -3;
    char* buffer = malloc(10);
    char* strcpy_retval = strcpy(buffer, string);
    char* strncpy_retval = strncpy(buffer, string, 10);
    free(buffer);
    if (strcpy_retval == 0x0) return -4;
    if (strncpy_retval == 0x0) return -5;
    return 0;
}

/* some bytes, used for testing. */
static int* bytes = {0xabcd, 0xcdef, 0xabcd, 0xcdef, 0xffff};

test_noinline int
target_function_3(void* region) {
    void* new_region = malloc(5);
    void* memcpy_retval = memcpy(new_region, region, 5);
    void* memset_retval = memset(new_region, 0, 5);
    void* memmove_retval = memmove(new_region, region, 5);
    free(new_region);
    if (memcpy_retval == 0x0) return -1;
    if (memset_retval == 0x0) return -2;
    if (memmove_retval == 0x0) return -3;
    return 0;
}

test_noinline int
target_function_4(const char* string, ...) {
    int printf_retval = printf(string);
    if (printf_retval == 0) return -1;
    int fprintf_retval = fprintf(stdout, string);
    if (fprintf_retval == 0) return -2;
    size_t length = strlen(string);
    char* buffer = malloc(100);
    int sprintf_retval = sprintf(buffer, "%s", string);
    int snprintf_retval = snprintf(buffer, length, "%s", string);
    va_list args;
    va_start(args, string);
    int vsprintf_retval = vsprintf(buffer, string, args);
    free(buffer);
    if (sprintf_retval == 0x0) return -3;
    if (snprintf_retval == 0x0) return -4;
    if (vsprintf_retval == 0x0) return -5;
    return 0;
}

test_noinline int
target_function_5() {
    FILE* fopen_retval = fopen("log/tmp.log", "w+");
    if (fopen_retval == 0x0) return -1;
    fprintf(fopen_retval, "hellothere!aaaaaaaa");
    fseek(fopen_retval, 0, SEEK_SET);
    char* buffer = malloc(100);
    size_t fread_retval = fread(buffer, 5, 1, fopen_retval);
    size_t fwrite_retval = fwrite(buffer, 1, 1, fopen_retval);
    free(buffer);
    if (fread_retval == 0x0) return -2;
    if (fwrite_retval == 0x0) return -3;
    int fclose_retval = fclose(fopen_retval);
    if (fclose_retval == EOF) return -4;
    return 0;
}

test_noinline int
target_function_6() {
    time_t time_retval = time(NULL);
    if (time_retval == 0) return -1;
    int rand_retval = rand();
    if (rand_retval == 0) return -2;
    return 0;
}

/** a condition to return a failure. */
tapi_condition(posix_action_fail, ...) {
    return E_TAPI_CONDITION_FAIL;
}

/** expand virtual args. */
#define EXPAND_ARGS(...) __VA_ARGS__

/** we create one test for success, and one for failure. */
#define TEST_DEFINITIONS(function, target_function, test_args, \
    failure_retval, set_errno, errno_val) \
    tapi_test(test_##function) { \
        /* arrange & act. */ \
        int retval = target_function(EXPAND_ARGS test_args); \
        /* assert. */ \
        tapi_assert(retval == 0); \
        return E_TAPI_TEST_RESULT_PASSED; \
    } \
    \
    tapi_test(test_##function##_failure) { \
        /* arrange & act. */ \
        int retval = target_function(EXPAND_ARGS test_args); \
        /* assert. */ \
        tapi_assert(retval == failure_retval); \
        if (set_errno == true) \
            tapi_assert(errno == errno_val); \
        return E_TAPI_TEST_RESULT_PASSED; \
    }

/** we declare each test like defined above. */
#define TEST_DECLARATIONS(function, target_function, test_args, \
    failure_retval, set_errno, errno_val) \
    tapi_add_test_and_auto_mock(context, "test_" #function, \
        test_##function, target_function, #function, 0x0, 0x0, set_errno); \
    tapi_add_test_and_auto_mock(context, "test_" #function "_failure", \
        test_##function##_failure, target_function, #function, 0x0, posix_action_fail, set_errno);

/** apply a macro to each of the POSIX methods to be tested here. */
#define POSIX_TESTS(M) \
    M(malloc, target_function_1, (20u), -1, true, ENOMEM) \
    M(calloc, target_function_1, (20u), -2, true, ENOMEM) \
    M(realloc, target_function_1, (20u), -3, true, ENOMEM) \
    M(strlen, target_function_2, ("hello!aaaa"), -1, false, 0x0) \
    M(strcmp, target_function_2, ("hello!bbbb"), -2, false, 0x0) \
    M(strncmp, target_function_2, ("hello!cccc"), -3, false, 0x0) \
    M(strcpy, target_function_2, ("hello!dddd"), -4, false, 0x0) \
    M(strncpy, target_function_2, ("hello!eeee"), -5, false, 0x0) \
    M(memcpy, target_function_3, ((void*)&bytes), -1, false, 0x0) \
    M(memset, target_function_3, ((void*)&bytes), -2, false, 0x0) \
    M(memmove, target_function_3, ((void*)&bytes), -3, false, 0x0) \
    M(printf, target_function_4, ("."), -1, false, 0x0) \
    M(fprintf, target_function_4, ("."), -2, false, 0x0) \
    M(sprintf, target_function_4, ("."), -3, false, 0x0) \
    M(snprintf, target_function_4, ("."), -4, false, 0x0) \
    M(vsprintf, target_function_4, ("."), -5, false, 0x0) \
    M(fopen, target_function_5, (), -1, false, 0x0) \
    M(fread, target_function_5, (), -2, false, 0x0) \
    M(fwrite, target_function_5, (), -3, false, 0x0) \
    M(fclose, target_function_5, (), -4, false, 0x0) \
    M(time, target_function_6, (), -1, false, 0x0) \
    M(rand, target_function_6, (), -2, false, 0x0) \

/** define all tests. */
POSIX_TESTS(TEST_DEFINITIONS)

int main(void) {
    tapi_context_t* context = tapi_init();
    POSIX_TESTS(TEST_DECLARATIONS)
    tapi_run_context(context);
    return 0;
}