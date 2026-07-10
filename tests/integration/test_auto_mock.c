/**
 * @author Sean Hobeck
 * @date 2026-07-09
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
int
target_function(int x) {
    void* a = calloc(1u, x);
    if (a == 0x0) return -1;
    free(a);
    return 0;
}
#pragma endregion

tapi_action(calloc_action, ...) {
    return 0x0;
}

tapi_test(test_target_function_calloc_fail) {
    /* arrange & act. */
    int retval = target_function(10);

    /* assert. */
    tapi_assert(retval == -1);
    return E_TAPI_TEST_RESULT_PASSED;
}

int main() {
    tapi_context_t* context = tapi_init();
    tapi_add_test_and_auto_mock(context, "test_target_function_calloc_fail", \
        test_target_function_calloc_fail, target_function, "calloc", 0x0, calloc_action);
    tapi_run_context(context);
    return 0;
}