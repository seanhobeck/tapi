/*
 * Copyright (C) 2026 Sean C. Hobeck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-06-16
 */
#ifndef TAPI_H
#define TAPI_H

/*! @uses tapi_dyna_t. */
#include <tapi/dyna.h>
/** \endcond */

/** enum for different types of results from a test. */
typedef enum {
    E_TAPI_TEST_RESULT_PASSED = 0x1, /** test passed. */
    E_TAPI_TEST_RESULT_FAILED, /** test failed. */
    E_TAPI_TEST_RESULT_SKIPPED, /** test skipped. */
} e_tapi_test_result_t;

/** a function pointer type for test functions. */
typedef e_tapi_test_result_t (*tapi_test_func_t)(void);

/** a function pointer type for setup and teardown functions. */
typedef void (*tapi_gen_func_t)(void);

/**
 * @brief a context for tapi; a thread-safe state of execution for tapi to use.
 *
 * `tapi_context_t` is a data structure for storing data for tapi to use as a saved state of
 *  execution. this contains a list of tests, as well as a list of active memory guards that are
 *  currently in use.
 */
typedef struct {
    /** dynamic array of test pointers. */
    tapi_dyna_t* tests;
    /** dynamic array of guard structs (internal use only). */
    tapi_dyna_t* guards;
} tapi_context_t;

/**
 * @brief a singular test within a test suite.
 *
 * `tapi_test_t` is a data structure for tests within the tapi. this contains a name, description,
 *   setup, and teardown functions, as well as other mocking and output capture information.
 *
 * @see tapi_test_setup()
 * @see tapi_test_add()
 * @see tapi_test_run()
 * @see tapi_test_create()
 * @see tapi_test_add_mock()
 * @see tapi_test_destroy()
 */
typedef struct {
    /** name of the test. */
    char* name;
    /** pointer to the test function. */
    tapi_test_func_t function;
    /** pointer to the setup and teardown functions. */
    tapi_gen_func_t setup, teardown;
    /** dynamic array of mock pointers, to create a tapi_mock_t* please @see {tapi/mock.h} */
    tapi_dyna_t* mocks;
    /** result of calling the test. */
    e_tapi_test_result_t result;
} tapi_test_t;

/**
 * @brief initialize a new context instance for tapi.
 *
 * @return a new tapi_context_t structure.
 */
TAPI_EXPORT tapi_context_t*
tapi_init(void);

/**
 * @brief run all the tests setup in the context in order.
 *
 * @param context the tapi context to be used.
 */
TAPI_EXPORT void
tapi_run_tests(tapi_context_t* context);

/**
 * @brief make a new test given minimal information.
 *
 * @param name the name of the test.
 * @param function the test function to be used.
 */
TAPI_EXPORT tapi_test_t*
tapi_make_test(const char* name, tapi_test_func_t function);

/**
 * @brief free and clean up a context after the tests have been ran.
 *
 * @param context the tapi context containing all the data to be freed (this will be freed).
 */
TAPI_EXPORT void
tapi_cleanup(tapi_context_t* context);

/** concatenation implementation. */
#define TAPI_CONCAT_IMPL(a, b) a##b
#define TAPI_CONCAT(a, b) TAPI_CONCAT_IMPL(a, b)

/** assert on a condition and fail a given test if not met. */
#define TAPI_ASSERT(cond) if (!(cond)) return E_TAPI_TEST_RESULT_FAILED;

/** quickly make a test. */
#define TAPI_TEST(name) \
    e_tapi_test_result_t name(void)

/** quickly add a test to the test suite. */
#define TAPI_ADD_TEST(context, name, function) \
    tapi_dyna_push(context->tests, tapi_test_make(name, function));

/** quickly add a test with a mock value to the test suite. */
#define TAPI_ADD_TEST_AND_MOCK(context, name, test_function, tested_function, target_function, stub_function) \
    tapi_test_t* TAPI_CONCAT(_gentest_, __LINE__) = tapi_make_test(name, test_function); \
    tapi_mock_t* TAPI_CONCAT(_genmock_, __LINE__) = tapi_make_mock(tested, target, mocked); \
    tapi_dyna_push(TAPI_CONCAT(_gentest_, __LINE__)->mocks, TAPI_CONCAT(_genmock_, __LINE__)); \
    tapi_dyna_push(context->tests, TAPI_CONCAT(_gentest_, __LINE__));

/**
 * quickly run the tapi context and cleanup; variable arguments should be used
 *  with TAPI_ADD_TEST and TAPI_ADD_TEST_AND_MOCK.
 */
#define TAPI_RUN_TESTS(context, ...) \
    __VA_ARGS__; \
    tapi_run_tests(context); \
    tapi_cleanup(context);
#endif /* TAPI_H */