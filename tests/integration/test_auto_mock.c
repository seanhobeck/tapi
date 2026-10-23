/**
 * @author Sean Hobeck
 * @date 2026-10-23
 */
#include "def.h"

test_noinline int
_target_function(int x) {
    void* a = calloc(1u, x);
    if (a == 0x0) return -1;
    free(a);
    return 0;
}

test_noinline void*
_target_function_2(int x) {
    void* mem = malloc(x);
    if (mem == 0x0) return 0x0;
    free(mem);
    return mem;
}

tapi_condition(calloc_action_fail, ...) {
    return E_TAPI_CONDITION_FAIL;
}

tapi_test(test_target_function_calloc_fail) {
    /* arrange & act. */
    int retval = _target_function(10);

    /* assert. */
    tapi_assert(retval == -1);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_target_function_calloc_fail_errno) {
    /* arrange & act. */
    int retval = _target_function(10);

    /* assert. */
    tapi_assert(retval == -1);
    tapi_assert(errno == ENOMEM);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_condition(calloc_action_conditional, ...) {
    static int count = 0;
    count++;
    if (count == 2) return E_TAPI_CONDITION_FAIL;
    return E_TAPI_CONDITION_ALLOW;
}

tapi_test(test_target_function_calloc_fail_on_second) {
    /* arrange & act. */
    int retval = _target_function(12);
    int retval2 = _target_function(34);

    /* assert. */
    tapi_assert(retval == 0);
    tapi_assert(retval2 == -1);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_condition(malloc_action_fail, ...) {
    return E_TAPI_CONDITION_FAIL;
}

tapi_test(test_target_function_malloc_fail) {
    /* arrange & act. */
    void* retval = _target_function_2(10);

    /* assert. */
    tapi_assert(retval == 0x0);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_test(test_target_function_malloc_fail_errno) {
    /* arrange & act. */
    void* retval = _target_function_2(10);

    /* assert. */
    tapi_assert(retval == 0x0);
    tapi_assert(errno == ENOMEM);
    return E_TAPI_TEST_RESULT_PASSED;
}

tapi_condition(free_action_fail, ...) {
    return E_TAPI_CONDITION_FAIL;
}

tapi_test(test_target_function_free_fail) {
    /* arrange & act. */
    void* retval = _target_function_2(10);

    /* assert. */
    tapi_assert(retval != 0x0);
    free(retval);
    return E_TAPI_TEST_RESULT_PASSED;
}

int main() {
    tapi_context_t* context = tapi_init();
    tapi_add_test_and_auto_mock(context, "test_target_function_calloc_fail", \
        test_target_function_calloc_fail, _target_function, "calloc", 0x0, calloc_action_fail, false);
    tapi_add_test_and_auto_mock(context, "test_target_function_calloc_fail_errno", \
        test_target_function_calloc_fail_errno, _target_function, "calloc", 0x0, calloc_action_fail, true);
    tapi_add_test_and_auto_mock(context, "test_target_function_calloc_fail_on_second", \
        test_target_function_calloc_fail_on_second, _target_function, "calloc", 0x0, calloc_action_conditional, false);
    tapi_add_test_and_auto_mock(context, "test_target_function_malloc_fail", \
        test_target_function_malloc_fail, _target_function_2, "malloc", 0x0, malloc_action_fail, false);
    tapi_add_test_and_auto_mock(context, "test_target_function_malloc_fail_errno", \
        test_target_function_malloc_fail_errno, _target_function_2, "malloc", 0x0, malloc_action_fail, true);
    tapi_add_test_and_auto_mock(context, "test_target_function_free_fail", \
        test_target_function_free_fail, _target_function_2, "free", 0x0, free_action_fail, false);
    tapi_run_context(context);
    return 0;
}