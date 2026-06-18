/*
 * Copyright (C) 2026 Sean C. Hobeck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-06-18
 */
#ifndef TAPI_MOCK_H
#define TAPI_MOCK_H

/*! @uses TAPI_EXPORT, tapi_context_t. */
#include <tapi/tapi.h>

/*! @uses size_t. */
#include <stddef.h>

/*! @uses bool, true, false. */
#include <stdbool.h>
/** \endcond */

#pragma region unimplemented
/** an enum for different results from an action. */
typedef enum {
    E_TAPI_ACTION_RESULT_ALLOW = 0x0, /* allow the mock to proceed as usual. */
    E_TAPI_ACTION_RESULT_FAIL, /* force the mock to fail. */
} e_tapi_action_result_t __attribute__((deprecated));

/** a function pointer for an action/ condition that is checked
 *  at the start of every autostub implemented by tapi. if an action
 *  returns true, the autostub will trigger a failure, and henceforth
 *  the tested function will receive a failure from this special mock
 *  as well. */
typedef e_tapi_action_result_t (*tapi_action_t)(void* blank, ...) __attribute__((deprecated));

/**
 * @note to use an implementation of tapi that does not contain automatic stubbing
 *  for very common special mocks (ie. malloc, free, fopen, fclose, etc...) you can
 *  #undef TAPI_AUTOSTUB, but please note that you will have to specify a target
 *  stub that you will have to make for some of the functions listed below.
 */
#define TAPI_AUTOSTUB
#ifdef TAPI_AUTOSTUB
/**
 * @brief a structure to keep track of special mocks that can be automatically stubbed with
 *  pre-built stubs for ease of use.
 *
 * `tapi_autostub_t` is a data structure for representing the data required internally by tapi to
 *  automatically stub commonly used special mocks, ie. malloc, free, calloc, fopen, etc... these
 *  functions can then be conditioned to fail under certain conditions, allowing testers to test
 *  for failures.
 */
typedef struct {
    /** pointer to the stub itself. */
    void* stub;
    /** pointer to an action */
    tapi_action_t action;
    /** the address of the special function (library or syscall). */
    void* address;
    /** should we be setting errno on failure for this action? */
    bool set_errno;
} tapi_autostub_t __attribute__((deprecated));
/* currently not in use, need to @todo; address resolver for plt. */

/** @brief malloc autostub used by tapi. */
TAPI_HIDDEN void*
tapi_stub_malloc(size_t size);

/** @brief calloc autostub used by tapi. */
TAPI_HIDDEN void*
tapi_stub_calloc(size_t nmemb, size_t size);

/** @brief free autostub used by tapi. */
TAPI_HIDDEN void
tapi_stub_free(void* ptr);
#endif
#pragma endregion
/* this entire region is unimplemented and deprecated until an @plt address
 *  resolver module can be made for POSIX-compliant systems. */

/**
 * @brief a patch-based runtime mock for redirecting calls within a tested function.
 *
 * `tapi_mock_t` is a data structure for mocking the result of a call within a tested function. this
 *   is mainly used for isolation from other functions, and to primarily test your functions logic,
 *   arithmetic, and functionality. mocks/ stub functions can be as simple as returning a
 *   value, or as complex as you need them to be; complexity is completely left up to the user.
 *
 * @see tapi_make_mock()
 * @see tapi_make_special_mock()
 * @see tapi_apply_mock()
 * @see tapi_cleanup_mocK()
 */
typedef struct {
    /** original, mocked, and target functions. */
    void* orig, *mocked, *target;
    /** address of the call in the original function. */
    void* call;
    /** size of the patch and function. */
    size_t size, fun_size;
    /** first 32 bytes of the original target function. */
    unsigned char orig_bytes[32u];
    /** first 32 bytes of the mocked function. */
    unsigned char mocked_bytes[32u];
    /** is this a special kind of mock, ie. does it need to all possible call targets? */
    bool is_special;
#ifdef TAPI_AUTOSTUB
    /** a pointer to an autostub structure if found in tapi's internal table (see above). */
    tapi_autostub_t* autostub;
#endif
} tapi_mock_t;

/**
 * @brief mock the first call occurrence to a target with a call to a mocked function instead.
 *  this will automatically allocate the mock structure ready to be applied whenever and wherever
 *  required.
 *
 * @param orig the original function to search for target in.
 * @param target the target address to be replaced.
 * @param mocked the function to replace the target call with.
 * @return an allocated mock structure ready to be applied.
 */
TAPI_EXPORT tapi_mock_t*
tapi_make_mock(void* orig, void* target, void* mocked);

/**
 * @brief mock all call occurrences to a target with a call to a mocked function instead, if
 *  specified. tapi considers a special mock a type of mock that requires every call to be mocked
 *  within a specified function, special mocks that have 'autostubs' are usually very common POSIX
 *  libc or system calls.
 *
 * @param orig the original function to search for target in.
 * @param target the target address to be replaced.
 * @param mocked the function to replace the target call with,
 *  please note that since re-creating a mock for every POSIX
 *  compliant function would take a ridiculous amount of space,
 *  you occasionally would have to treat this as a regular mock
 *  and still provide a mocked stub address, otherwise if it is
 *  in the table specified above, then no address is required.
 * @param action the action/ condition function that allows the
 *  mock to either pass or fail based on certain conditions.
 *
 * @return an allocated mock structure ready to be applied.
 */
TAPI_EXPORT tapi_mock_t*
tapi_make_special_mock(void* orig, void* target, \
    void* mocked, tapi_action_t action);

/** @note this means that we are using a maximum search len of 4096 bytes for determining a
 * functions size; we only iterate through 4096 bytes worth of possible opcodes (every
 * architecture varies in opcode size). this can be overwritten like so,
 *
 * ...
 * #undef TAPI_MAX_DET_DEPTH
 * #define TAPI_MAX_DET_DEPTH (your_value)
 * ...
 */
#define TAPI_MAX_DET_DEPTH 0x1000

/**
 * @brief apply a call target (or multiple) patch in memory to route to the given stub by the
 * mock. this is usually performed by `tapi_run_tests`, but can be used under special
 * circumstances if required.
 *
 * @param context the context of tapi to be used.
 * @param mock the mock to be applied.
 */
TAPI_EXPORT void
tapi_apply_mock(tapi_context_t* context, tapi_mock_t* mock);

/**
 * @brief restore the contents of a function and free the mock. this frees all the data held by
 *  the mock, the pointer itself and the guard held within the context. 'mock' should not be used
 *  after this is called, either copy the data required before this call or create a new mock.
 *
 * @param context the context of tapi to be used.
 * @param mock the mock structure to be freed and restored.
 */
TAPI_EXPORT void
tapi_cleanup_mock(tapi_context_t* context, tapi_mock_t* mock);

/** quickly create an action function to be used in an autostub. */
#define tapi_action(action_name, ...) \
    e_tapi_action_result_t action_name(void* blank, __VA_ARGS__)

/** quickly create a simple stub to return a given value. */
#define tapi_stub_return(func_name, return_type, return_value) \
    return_type func_name() { return return_value; }

/** quickly create a stub to return a given integer. */
#define tapi_stub_return_int(func_name, return_value) \
    tapi_stub_return(func_name, int, return_value)

/** quickly create a stub to return a given pointer. */
#define tapi_stub_return_ptr(func_name, return_value) \
    tapi_stub_return(func_name, void*, return_value)

/** quickly create a stub to return null. */
#define tapi_stub_return_null(func_name) \
    tapi_stub_return(func_name, void*, null)

/** quickly create a stub to return a string/ string literal. */
#define tapi_stub_return_strl(func_name, return_value) \
    tapi_stub_return(func_name, char*, return_value)

/** quickly add a test with a mock value to the test suite. */
#define tapi_add_test_and_mock(context, name, test_function, tested_function, target_function, \
    stub_function) \
    tapi_test_t* TAPI_CONCAT(_gentest_, __LINE__) = tapi_make_test(name, test_function); \
    tapi_mock_t* TAPI_CONCAT(_genmock_, __LINE__) = tapi_make_mock(tested_function, \
        target_function, stub_function); \
    tapi_dyna_push(TAPI_CONCAT(_gentest_, __LINE__)->mocks, TAPI_CONCAT(_genmock_, __LINE__)); \
    tapi_dyna_push(context->tests, TAPI_CONCAT(_gentest_, __LINE__));

/** quickly add a test with a mock value to the test suite. */
#define tapi_add_test_and_special_mock(context, name, test_function, tested_function, \
    target_function, stub_function, action) \
    tapi_test_t* TAPI_CONCAT(_gentest_, __LINE__) = tapi_make_test(name, test_function); \
    tapi_mock_t* TAPI_CONCAT(_genmock_, __LINE__) = tapi_make_special_mock(tested_function, \
        target_function, stub_function, action); \
    tapi_dyna_push(TAPI_CONCAT(_gentest_, __LINE__)->mocks, TAPI_CONCAT(_genmock_, __LINE__)); \
    tapi_dyna_push(context->tests, TAPI_CONCAT(_gentest_, __LINE__));
#endif /* TAPI_MOCK_H */