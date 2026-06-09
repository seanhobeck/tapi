/*
 * Copyright (C) 2026 Sean C. Hobeck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-06-09
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
    /** dynamic array of mock pointers. */
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
 * @brief set up many tests to be run in concession.
 *
 * @param context the tapi context to be used.
 * @param tests the array of tests to be set up for a test file.
 * @param count the number of tests to be set up.
 */
TAPI_EXPORT void
tapi_test_setup(tapi_context_t* context, tapi_test_t** tests, size_t count);

/**
 * @brief add a test to your testing suite.
 *
 * @param context the tapi context to be used.
 * @param test the test to be added.
 */
TAPI_EXPORT void
tapi_test_add(tapi_context_t* context, tapi_test_t* test);

/**
 * @brief run all the tests setup in the context in order.
 *
 * @param context the tapi context to be used.
 */
TAPI_EXPORT void
tapi_test_run(tapi_context_t* context);

/**
 * @brief make a new test given minimal information.
 *
 * @param name the name of the test.
 * @param function the test function to be used.
 */
TAPI_EXPORT tapi_test_t*
tapi_test_make(const char* name, tapi_test_func_t function);

/**
 * @brief add a mock to a certain test.
 *
 * @param test the test to be altered.
 * @param tested the tested function to search through.
 * @param target the target address to redirect to mock.
 * @param mocked the mocked result to be redirected to.
 */
TAPI_EXPORT void
tapi_test_add_mock(tapi_test_t* test, void* tested, void* target, void* mocked);

/**
 * @brief free and clean up a context after the tests have been ran.
 *
 * @param context the tapi context containing all the data to be freed (this will be freed).
 */
TAPI_EXPORT void
tapi_test_cleanup(tapi_context_t* context);

/** concatenation implementation. */
#define TAPI_CONCAT_IMPL(a, b) a##b
#define TAPI_CONCAT(a, b) TAPI_CONCAT_IMPL(a, b)

/** assert on a condition and fail a given test if not met. */
#define TAPI_ASSERT(cond) if (!(cond)) return E_TAPI_TEST_RESULT_FAILED;

/** quickly make a test. */
#define TAPI_MAKE_TEST(name) \
    e_tapi_test_result_t name(void)

/** quickly add a test to the test suite. */
#define TAPI_ADD_TEST(context, name, function) \
    tapi_test_add(context, tapi_test_make(name, function));

/** quickly add a test with a mock value to the test suite. */
#define TAPI_ADD_TEST_AND_MOCK(context, name, test_function, tested_function, target_function, \
    stub_function) \
    do { \
        tapi_test_t* TAPI_CONCAT(_gentest_, __LINE__) = tapi_test_make(name, test_function); \
        tapi_test_add_mock(TAPI_CONCAT(_gentest_, __LINE__), tested_function, \
            target_function, stub_function); \
        tapi_test_add(context, TAPI_CONCAT(_gentest_, __LINE__)); \
    } while(0);

/**
 * quickly run the tapi context and cleanup; variable arguments should be used
 *  with TAPI_ADD_TEST and TAPI_ADD_TEST_AND_MOCK.
 */
#define TAPI_RUN_TESTS(context, ...) \
    __VA_ARGS__; \
    tapi_test_run(context); \
    tapi_test_cleanup(context);
#endif /* TAPI_H */