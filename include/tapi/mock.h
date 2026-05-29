/*
 * Copyright (C) 2026 Sean C. Hobeck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-05-29
 */
#ifndef TAPI_MOCK_H
#define TAPI_MOCK_H

/*! @uses TAPI_EXPORT. */
#include <tapi/tapi.h>

/*! @uses size_t. */
#include <stddef.h>
/** \endcond */

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
} tapi_mock_t;

/**
 * @brief mock the first call occurrence to a target with a call
 *  to a mocked function instead.
 *
 * @param orig the original function to search for target in.
 * @param target the target address to be replaced.
 * @param mocked the function to replace the target call with.
 * @return an allocated mock structure with all data, ready to be applied.
 */
TAPI_EXPORT tapi_mock_t*
tapi_mock_create(void* orig, void* target, void* mocked);

/**
 * @brief apply the mocks patch in memory; write stub to route to
 *  the given mocked function pointer.
 *
 * @param mock the mock to be applied.
 */
TAPI_EXPORT void
tapi_mock_apply(tapi_mock_t* mock);

/**
 * @brief restore the contents of a function and free the mock.
 *
 * @param mock the mock structure to be freed and restored.
 */
TAPI_EXPORT void
tapi_mock_restore(tapi_mock_t* mock);

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