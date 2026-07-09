/**
 * @author Sean Hobeck
 * @date 2026-07-07
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

/* region for all tested functions. */
#pragma region
int add(int a, int b) {
    return a + b;
}

int sub(int a, int b) {
    return a - b;
}

int
target_function() {
    int c = add(12, 13); /* first call */
    int d = add(c, 14); /* second call */
    c = sub(c, 4);
    d = sub(d, 11);
    return d + c;
}
#pragma endregion

tapi_stub_return_int(stub_add, 2);

tapi_test(test_target_add_1) {
    /* arrange & act. */
    int retval = target_function();

    /* assert. */
    tapi_assert(retval == 3);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_target_add_2) {
    /* arrange & act. */
    int retval = target_function();

    /* assert. */
    tapi_assert(retval == 12);
    return E_TAPI_TEST_RESULT_PASSED;
}

int main() {
    tapi_context_t* context = tapi_init();
    tapi_add_test_and_special_mock(context, "test_target_add_call_1", \
        test_target_add_1, target_function, add, stub_add, 1);
    tapi_add_test_and_special_mock(context, "test_target_add_call_2", \
        test_target_add_2, target_function, add, stub_add, 2);
    tapi_run_context(context);
    return 0;
}