/*
 * Copyright (C) 2026 Sean C. Hobeck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-06-12
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

/** an enum for different results from an action. */
typedef enum {
    E_TAPI_ACTION_RESULT_ALLOW = 0x0, /* allow the mock to proceed as usual. */
    E_TAPI_ACTION_RESULT_FAIL, /* force the mock to fail. */
} e_tapi_action_result_t;

/** a function pointer for an action/ condition that is checked
 *  at the start of every autostub implemented by tapi. if an action
 *  returns true, the autostub will trigger a failure, and henceforth
 *  the tested function will receive a failure from this special mock
 *  as well. */
typedef e_tapi_action_result_t (*tapi_action_t)(void* blank, ...);

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
} tapi_autostub_t;

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

/**
 * @brief a patch-based runtime mock for redirecting calls within a tested function.
 *
 * `tapi_mock_t` is a data structure for mocking the result of a call within a tested function. this
 *   is mainly used for isolation from other functions, and to primarily test your functions logic,
 *   arithmetic, and functionality. mocks/ stub functions can be as simple as returning a
 *   value, or as complex as you need them to be; complexity is completely left up to the user.
 *
 * @see tapi_mock_create()
 * @see tapi_mock_apply()
 * @see tapi_mock_restore()
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
    /** is this a special kind of mock; library or system \
      * call (from plt/got/iat on windows). */
    bool is_special;
#ifdef TAPI_AUTOSTUB
    /** a pointer to an autostub structure if found in tapi's internal table (see above). */
    tapi_autostub_t* autostub;
#endif
} tapi_mock_t;

/**
 * @brief mock the first call occurrence to a target with a call
 *  to a mocked function instead.
 *
 * @param orig the original function to search for target in.
 * @param target the target address to be replaced.
 * @param mocked the function to replace the target call with.
 * @return an allocated mock structure ready to be applied.
 */
TAPI_EXPORT tapi_mock_t*
tapi_mock_create(void* orig, void* target, void* mocked);

/**
 * @brief mock all call occurrences to a target with a call to
 *  a mocked function instead, if specified.
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
tapi_special_mock_create(void* orig, void* target, \
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
 * @brief apply the mocks patch in memory; write stub to route to
 *  the given mocked function pointer.
 *
 * @param context the context of tapi to be used.
 * @param mock the mock to be applied.
 */
TAPI_EXPORT void
tapi_mock_apply(tapi_context_t* context, tapi_mock_t* mock);

/**
 * @brief restore the contents of a function and free the mock.
 *
 * @param context the context of tapi to be used.
 * @param mock the mock structure to be freed and restored.
 */
TAPI_EXPORT void
tapi_mock_restore(tapi_context_t* context, tapi_mock_t* mock);

/** create a simple mock to return a given value. */
#define TAPI_MOCK_RETURN(func_name, return_type, return_value) \
    return_type func_name() { return return_value; }

/** create a mock to return a given integer. */
#define TAPI_MOCK_RETURN_INT(func_name, return_value) \
    TAPI_MOCK_RETURN(func_name, int, return_value)

/** create a mock to return a given pointer. */
#define TAPI_MOCK_RETURN_PTR(func_name, return_value) \
    TAPI_MOCK_RETURN(func_name, void*, return_value)

/** create a mock to return null. */
#define TAPI_MOCK_RETURN_NULL(func_name) \
    TAPI_MOCK_RETURN(func_name, void*, null)

/** create a mock to return a string/ string literal. */
#define TAPI_MOCK_RETURN_STRL(func_name, return_value) \
    TAPI_MOCK_RETURN(func_name, char*, return_value)
#endif /* TAPI_MOCK_H */