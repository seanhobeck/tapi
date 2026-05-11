/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-03-18
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

/* local testing suite. */
static tapi_dyna_t* l_tests;

/**
 * @brief set up many tests to be run in concession.
 *
 * @param tests the array of tests to be set up for a test file.
 * @param count the number of tests to be set up.
 */
void
tapi_test_setup(tapi_test_t** tests, size_t count) {
    /* if we already have tests. */
    if (l_tests != 0x0) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi, setup_tests; tests != null; refer to tapi_add_test().\n");
        return;
    }

    /* and we are done. */
    l_tests = tapi_dyna_create();
    for (size_t i = 0u; i < count; i++)
        tapi_dyna_push(l_tests, tests[i]);
}

/**
 * @brief add a test to your testing suite.
 *
 * @param test the test to be added.
 */
void
tapi_test_add(tapi_test_t* test) {
    /* if we don't have tests. */
    if (l_tests == 0x0)
        l_tests = tapi_dyna_create();
    tapi_dyna_push(l_tests, test); /* very simple push. */
};

/** @brief run all the tests set up in concession. */
void
tapi_test_run(void) {
    /* iterate through each test, */
    size_t passed = 0u;
    DYNA_FOREACH_IT(l_tests, tapi_test_t*, test, i)
        /* call setup, apply the mocks, */
        if (test->setup != 0x0) test->setup();
        DYNA_FOREACH_IT(test->mocks, tapi_mock_t*, mock, j)
            tapi_mock_apply(mock);
        DYNA_ENDFOREACH;

        /* call the test, */
        test->result = test->function();
        if (test->result == E_TAPI_TEST_RESULT_PASSED) {
            passed++;
            printf("[%zu/%zu] tapi: %s, passed.\n", passed, l_tests->length, test->name);
        }
        else if (test->result == E_TAPI_TEST_RESULT_SKIPPED) {
            printf("[%zu/%zu] tapi: %s, skipped.\n", passed, l_tests->length, test->name);
        }
        else {
            printf("[%zu/%zu] tapi: %s, failed.\n", passed, l_tests->length, test->name);
        }

        /* then call teardown and restore mocks. */
        DYNA_FOREACH_IT(test->mocks, tapi_mock_t*, mock, j)
            tapi_mock_restore(mock);
        DYNA_ENDFOREACH;
        if (test->teardown != 0x0) test->teardown();
    DYNA_ENDFOREACH;

    /* on exit, we clean up the internal guard list. */
    guard_cleanup();
    printf("tapi; total tests passed: [%zu/%zu].\n", passed, l_tests->length);
};

/**
 * @brief make a new test given minimal information.
 *
 * @param name the name of the test.
 * @param function the test function to be used.
 */
tapi_test_t*
tapi_test_make(const char* name, tapi_test_func_t function) {
    /* allocate and make the structure. */
    tapi_test_t* test = calloc(2, sizeof *test);
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
void
tapi_test_add_mock(tapi_test_t* test, void* tested, void* target, void* mocked) {
    /* create a dynamic array if it doesn't already exist. */
    if (test->mocks == 0x0)
        test->mocks = tapi_dyna_create();

    /* create the mock ptr and push it onto the dynamic array. */
    tapi_mock_t* mock = tapi_mock_create(tested, target, mocked);
    tapi_dyna_push(test->mocks, mock);
}

/**
 * @brief free and destroy a list of tests after they have been ran.
 *
 * @param tests the tests to be freed (this also frees all of its elements, including mocks).
 * @param length the number of tests to be freed.
 */
void
tapi_test_destroy(tapi_test_t** tests, size_t length) {
    /* free each test but not the list itself, that isn't ours. */
    for (size_t i = 0; i < length; i++) {
        tapi_dyna_free(tests[i]->mocks);
        free(tests[i]->name);
        free(tests[i]);
    }
};