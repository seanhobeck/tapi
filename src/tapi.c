/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-06-18
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
 * @brief initialize a new context instance for tapi. this is a thread-safe context holding all
 *  data required to set up tests, mocks, captures, and sinks.
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
 * @brief run all the tests setup in the context in order. this will run all the tests in the
 *  dynamic array held by 'context' and will apply associated special and regular mocks before
 *  each test.
 *
 * @param context the tapi context to be used.
 */
void
tapi_run_tests(tapi_context_t* context) {
    /* iterate through each test, */
    size_t passed = 0u;
    dyna_foreach_it(context->tests, tapi_test_t*, test, i)
        /* call setup, apply the mocks, */
        if (test->setup != 0x0) test->setup();
        dyna_foreach_it(test->mocks, tapi_mock_t*, mock, j)
            tapi_apply_mock(context, mock);
        dyna_endforeach(test->mocks);

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
        dyna_foreach_it(test->mocks, tapi_mock_t*, mock, j)
            tapi_cleanup_mock(context, mock);
        dyna_endforeach(test->mocks);
        if (test->teardown != 0x0) test->teardown();
    dyna_endforeach(context->tests);

    /* on exit, we clean up the internal guard list. */
    guard_cleanup(context);
    printf("tapi; total tests passed: [%zu/%zu].\n", passed, context->tests->length);
};

/**
 * @brief make a new test given minimal information. this function will automatically allocate
 *  the pointer, as well as the memory required by the name and the rest of the fields in the
 *  struct.
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
 * @brief free and clean up a context after the tests have been executed. this will clean up and
 *  free all data held by as well as the pointer to 'context'; 'context' should not be used
 *  after this is called, either use it before this function or make a new context.
 *
 * @param context the tapi context containing all the data to be freed (this will be freed).
 */
void
tapi_cleanup(tapi_context_t* context) {
    /* free each test but not the list itself, that isn't ours. */
    for (size_t i = 0; i < context->tests->length; i++) {
        tapi_test_t* test = dyna_get(context->tests, tapi_test_t*, i);
        tapi_dyna_free(test->mocks);
        free(test->name);
        free(test);
    }
    free(context);
};