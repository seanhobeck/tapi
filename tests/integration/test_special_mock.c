/**
 * @author Sean Hobeck
 * @date 2026-06-18
 */
#include <tapi/tapi.h>

/*! @uses TAPI_ADD_TEST_AND_MOCK, etc... */
#include <tapi/mock.h>

/*! @uses TAPI_CAPTURE. */
#include <tapi/capture.h>

/*! @uses free, calloc, malloc. */
#include <stdlib.h>

/*! @uses strcmp. */
#include <string.h>

/*! @uses snprintf. */
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
    int c = add(12, 13);
    int d = add(c, 14);
    c = sub(c, 4);
    d = sub(d, 11);
    return d + c;
}
#pragma endregion

tapi_stub_return_int(stub_add, 2);

tapi_test(test_target_add) {
    /* arrange & act. */
    int retval = target_function();

    /* assert. */
    tapi_assert(retval == -11);
    return E_TAPI_TEST_RESULT_PASSED;
}

int main() {
    tapi_context_t* context = tapi_init();
    tapi_add_test_and_special_mock(context, "test_target_add", \
        test_target_add, target_function, add, stub_add, 0x0);
    tapi_run_context(context);
    return 0;
}