/*
 * Copyright (C) 2026 Sean C. Hobeck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-05-29
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
 * @brief set up many tests to be run in concession.
 *
 * @param tests the array of tests to be set up for a test file.
 * @param count the number of tests to be set up.
 */
TAPI_EXPORT void
tapi_test_setup(tapi_test_t** tests, size_t count);

/**
 * @brief add a test to your testing suite.
 *
 * @param test the test to be added.
 */
TAPI_EXPORT void
tapi_test_add(tapi_test_t* test);

/** @brief run all the tests set up in concession. */
TAPI_EXPORT void
tapi_test_run(void);

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
 * @brief free and destroy a list of tests after they have been ran.
 *
 * @param tests the tests to be freed (this also frees all of its elements, including mocks).
 * @param length the number of tests to be freed.
 */
TAPI_EXPORT void
tapi_test_destroy(tapi_test_t** tests, size_t length);

/** concatenation implementation. */
#define TAPI_CONCAT_IMPL(a, b) a##b
#define TAPI_CONCAT(a, b) TAPI_CONCAT_IMPL(a, b)

/** assert on a condition and fail a given test if not met. */
#define TAPI_ASSERT(cond) if (!(cond)) return E_TAPI_TEST_RESULT_FAILED;

/** quickly make a test. */
#define TAPI_MAKE_TEST(name) \
    e_tapi_test_result_t name(void)

/** quickly add a test to the test suite. */
#define TAPI_ADD_TEST(name, function) \
    tapi_test_add(tapi_test_make(name, function));

/** quickly add a test with a mock value to the test suite. */
#define TAPI_ADD_TEST_AND_MOCK(name, test_function, tested_function, target_function, \
    stub_function) \
    do { \
        tapi_test_t* TAPI_CONCAT(_gentest_, __LINE__) = tapi_test_make(name, test_function); \
        tapi_test_add_mock(TAPI_CONCAT(_gentest_, __LINE__), tested_function, target_function, \
        stub_function); \
        tapi_test_add(TAPI_CONCAT(_gentest_, __LINE__)); \
    } while(0);

/**
 * quickly create a test suite; variable arguments should be used with tapi_quick_test and
 *    tapi_quick_test_and_mock.
 */
#define TAPI_RUN_TESTS(...) \
    __VA_ARGS__; \
    tapi_test_run();
#endif /* TAPI_H */