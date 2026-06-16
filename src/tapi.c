/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-06-16
 */
#include <tapi/tapi.h>

/*! @uses fprintf, stderr. */
#include <stdio.h>

/*! @uses calloc, free. */
#include <stdlib.h>

/*! @uses strlen, memcpy. */
#include <string.h>

/*! @uses tapi_mock_t, tapi_apply_mock. */
#include <tapi/mock.h>

/*! @uses guard_cleanup. */
#include "guard.h"
/** \endcond */

/**
 * @brief initialize a new context instance for tapi.
 *
 * @return a new tapi_context_t structure.
 */
tapi_context_t*
tapi_init(void) {
    /* allocate the tapi context, set a new list for the guards, but none for the tests. */
    tapi_context_t* context = calloc(1u, sizeof *context);
    context->guards = tapi_dyna_create();
    context->tests = tapi_dyna_create();
    return context;
};

/**
 * @brief set up many tests to be run in concession.
 *
 * @param context the tapi context to be used.
 * @param tests the array of tests to be set up for a test file.
 * @param count the number of tests to be set up.
 */
void
tapi_test_setup(tapi_context_t* context, tapi_test_t** tests, size_t count) {
    /* and we are done. */
    for (size_t i = 0u; i < count; i++)
        tapi_dyna_push(context->tests, tests[i]);
}

/**
 * @brief run all the tests setup in the context in order.
 *
 * @param context the tapi context to be used.
 */
void
tapi_run_tests(tapi_context_t* context) {
    /* iterate through each test, */
    size_t passed = 0u;
    DYNA_FOREACH_IT(context->tests, tapi_test_t*, test, i)
        /* call setup, apply the mocks, */
        if (test->setup != 0x0) test->setup();
        DYNA_FOREACH_IT(test->mocks, tapi_mock_t*, mock, j)
            tapi_apply_mock(context, mock);
        DYNA_ENDFOREACH(test->mocks);

        /* call the test, */
        test->result = test->function();
        if (test->result == E_TAPI_TEST_RESULT_PASSED) {
            passed++;
            printf("[%zu/%zu] tapi: %s, passed.\n", passed, context->tests->length, test->name);
        }
        else if (test->result == E_TAPI_TEST_RESULT_SKIPPED) {
            printf("[%zu/%zu] tapi: %s, skipped.\n", passed, context->tests->length, test->name);
        }
        else {
            printf("[%zu/%zu] tapi: %s, failed.\n", passed, context->tests->length, test->name);
        }

        /* then call teardown and restore mocks. */
        DYNA_FOREACH_IT(test->mocks, tapi_mock_t*, mock, j)
            tapi_cleanup_mock(context, mock);
        DYNA_ENDFOREACH(test->mocks);
        if (test->teardown != 0x0) test->teardown();
    DYNA_ENDFOREACH(context->tests);

    /* on exit, we clean up the internal guard list. */
    guard_cleanup(context);
    printf("tapi; total tests passed: [%zu/%zu].\n", passed, context->tests->length);
};

/**
 * @brief make a new test given minimal information.
 *
 * @param name the name of the test.
 * @param function the test function to be used.
 */
tapi_test_t*
tapi_make_test(const char* name, tapi_test_func_t function) {
    /* allocate and make the structure. */
    tapi_test_t* test = calloc(1u, sizeof *test);
    size_t length = strlen(name);
    test->name = calloc(1u, length + 1u);
    /* NOLINTNEXTLINE */
    strcpy(test->name, name);
    test->name[length] = '\0';
    test->mocks = tapi_dyna_create();
    test->function = function;
    return test;
}

/**
 * @brief add a mock to a certain test.
 *
 * @param test the test to be altered.
 * @param tested the tested function to search through.
 * @param target the target address to redirect to mock.
 * @param mocked the mocked result to be redirected to.
 */
TAPI_EXPORT void
tapi_test_add_mock(tapi_test_t* test, void* tested, void* target, void* mocked) {
    /* create the mock ptr and push it onto the dynamic array. */
    tapi_mock_t* mock = tapi_make_mock(tested, target, mocked);
    tapi_dyna_push(test->mocks, mock);
}

/**
 * @brief free and clean up a context after the tests have been ran.
 *
 * @param context the tapi context containing all the data to be freed (this will be freed).
 */
void
tapi_cleanup(tapi_context_t* context) {
    /* free each test but not the list itself, that isn't ours. */
    for (size_t i = 0; i < context->tests->length; i++) {
        tapi_test_t* test = DYNA_GET(context->tests, tapi_test_t*, i);
        tapi_dyna_free(test->mocks);
        free(test->name);
        free(test);
    }
    free(context);
};