/**
 * @author Sean Hobeck
 * @date 2026-08-03
 */
#include <tapi/tapi.h>

/*! uses TAPI_ADD_TEST_AND_MOCK, etc... */
#include <tapi/mock.h>

/*! uses TAPI_CAPTURE. */
#include <tapi/capture.h>

/*! uses free, calloc, malloc. */
#include <stdlib.h>

/*! uses strcmp. */
#include <string.h>

/*! uses snprintf. */
#include <stdio.h>

/*! noinline macro. */
#ifdef _WIN32
#define TEST_NOINLINE __declspec(noinline)
#else
#define TEST_NOINLINE __attribute__((noinline))
#endif

/*! -----------------tested functions------------------ !*/

TEST_NOINLINE int 
add(int a, int b) {
    return a + b;
}

TEST_NOINLINE int 
sub(int a, int b) {
    return a - b;
}

int
targ_function() {
    int c = add(12, 13); /* first call */
    int d = add(c, 14); /* second call */
    c = sub(c, 4);
    d = sub(d, 11);
    return d + c;
}

/*! -----------------------tests--------------------- !*/

tapi_stub_return_int(stub_add, 2);

tapi_test(test_target_add_1) {
    /* arrange & act. */
    int retval = targ_function();

    /* assert. */
    tapi_assert(retval == 3);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_target_add_2) {
    /* arrange & act. */
    int retval = targ_function();

    /* assert. */
    tapi_assert(retval == 12);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_stub_return_int(stub_sub, -8);

tapi_test(test_target_sub_1) {
    /* arrange & act. */
    int retval = targ_function();

    /* assert. */
    tapi_assert(retval == 20);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_target_sub_2) {
    /* arrange & act. */
    int retval = targ_function();

    /* assert. */
    tapi_assert(retval == 13);
    return E_TAPI_TEST_RESULT_PASSED;
}

/*! ----------------------==---------------------- !*/

#ifndef _WIN32
int main() {
#else
int test_special_mock() {
#endif
    tapi_context_t* context = tapi_init();
    tapi_add_test_and_special_mock(context, "test_target_add_call_1", \
        test_target_add_1, targ_function, add, stub_add, 1);
    tapi_add_test_and_special_mock(context, "test_target_add_call_2", \
        test_target_add_2, targ_function, add, stub_add, 2);
    tapi_add_test_and_special_mock(context, "test_target_sub_call_1", \
        test_target_sub_1, targ_function, sub, stub_sub, 1);
    tapi_add_test_and_special_mock(context, "test_target_sub_call_2", \
        test_target_sub_2, targ_function, sub, stub_sub, 2);
    tapi_run_context(context);
    return 0;
}