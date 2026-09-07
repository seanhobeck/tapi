/*
 * Copyright (C) 2026 Sean C. Hobeck
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-09-07
 */
#ifndef TAPI_MOCK_H
#define TAPI_MOCK_H

/*! uses TAPI_EXPORT, TAPI, tapi_context_t. */
#include <tapi/tapi.h>

/*! uses size_t. */
#include <stddef.h>

/*! uses bool, true, false. */
#include <stdbool.h>
/** \endcond */

/**
 * to make this library more minimal and controlled, you can define 'TAPI_MINIMAL' during compilation
 *  by adding the minimal=1 flag (for make) or -DMINIMIAL (for msvc) to your compilation process for tapi.
 */
#ifndef TAPI_MINIMAL
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
 * @brief a structure to keep track of special mocks that can be automatically stubbed with
 *  pre-built stubs for ease of use.
 *
 * `tapi_autostub_t` is a data structure for representing the data required internally by tapi to
 *  automatically stub commonly used special mocks, ie. malloc, free, calloc, fopen, etc... these
 *  functions can then be conditioned to fail under certain conditions, allowing testers to test
 *  for failures.
 *
 * list of currently supported autostubs for POSIX & MSVC system/library calls:
 *  _____________________________________
 *  |system/libc    |msvc crt-equiv. api|
 *  =====================================
 *  |malloc         |.                  |
 *  |calloc         |.                  |
 *  |free           |.                  |
 *  |realloc        |.                  |
 *  |strlen         |.                  |
 *  |strcmp         |.                  |
 *  |strncmp        |.                  |
 *  |strcpy         |strcpy_s           |
 *  |strncpy        |strncpy_s          |
 *  |memcpy         |memcpy_s           |
 *  |memmove        |memmove_s          |
 *  |memset         |.                  |
 *  |strcat         |strcat_s           |
 *  |strncat        |strncat_s          |
 *  |printf         |printf_s           |
 *  |fprintf        |fprintf_s          |
 *  |sprintf        |sprintf_s          |
 *  |snprintf       |_snprintf_s        |
 *  |vsprintf       |vsprintf_s         |
 *  |open           |_open              |
 *  |fopen          |fopen_s            |
 *  |freopen        |freopen_s          |
 *  |read           |_read              |
 *  |fread          |fread_s            |
 *  |write          |_write             |
 *  |fwrite         |.                  |
 *  |close          |_close             |
 *  |fclose         |.                  |
 *  |getenv         |getenv_s           |
 *  |getpid         |_getpid            |
 *  |time           |.                  |
 *  |rand           |rand_s             |
 *  =====================================
 */
typedef struct {
    /** pointer to the stub itself. */
    void* stub;
    /** pointer to an action */
    tapi_action_t action;
    /** the name of the special function (library/system call). */
    char* name;
    /** should we be setting errno on failure for this action? */
    bool set_errno;
} tapi_autostub_t;
#endif

/**
 * @brief enum for differentiating different types of mocks.
 *
 *  'regular mock': replace all calls to <target> within the original function.
 *  'specific mock': replace the very first found call to <target> within the original function.
 *  'auto mock': this is a library/system call where the address needs to be resolved, treated like
 *      a 'special mock' where stubs can be provided automatically if the function is commonly used.
 */
typedef enum {
    E_TAPI_MOCK_REGULAR = 0x1, /* all occurrences of the call target are replaced. */
    E_TAPI_MOCK_SPECIAL, /* the n-th occurrence of the call target is replaced. */
    E_TAPI_MOCK_AUTO, /* possibly autostub the mock, this is reserved for library/system calls. */
} e_tapi_mock_type_t;


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
    /** 15 bytes of the original call target. */
    unsigned char orig_bytes[15u];
    /** 15 bytes of the mocked call target. */
    unsigned char mocked_bytes[15u];
    /** is this a special kind of mock, ie. does it need to all possible call targets? */
    e_tapi_mock_type_t type;
    /** the n-th call target occurrence within the original function (for special mocks only). */
    size_t call_index;
    /** the dynamic list of det_call_t structures. */
    tapi_dyna_t* calls;

#ifndef TAPI_MINIMAL
    /*! anon struct to encompass all 'auto mock' data. */
    union {
        struct {
            /** a pointer to an autostub structure if found in tapi's internal table (see above). */
            tapi_autostub_t* autostub;
            /** an action associated with an autostub structure. */
            tapi_action_t action;
            /** should errno be set by the autostub? */
            bool set_errno;
        } info;
    } data;
#endif
} tapi_mock_t;

/**
 * @brief mock all/specified call occurrence(s) to a target with a call to a mocked function
 *  instead. this will automatically allocate the mock structure ready to be applied whenever and
 *  wherever required.
 *
 * @param orig the original function to search for target in.
 * @param target the target address to be replaced.
 * @param mocked the function to replace the target call with.
 * @param call_index the n-th call occurrence that should be replaced, if zero is given, the mock is
 *  assumed to be regular not special and will replace every call occurrence found.
 * @return an allocated mock structure ready to be applied.
 */
TAPI_EXPORT tapi_mock_t* TAPI
tapi_make_mock(void* orig, void* target, void* mocked, size_t call_index);

#ifndef TAPI_MINIMAL
/**
 * @brief mock all call occurrences to a target with a call to an autostub or a mocked function
 *  instead. this should only be used on system/library calls with addresses that need to be resolved,
 *  ie. calloc, free, fopen, fclose.
 *
 * @param orig the original function to search for target in.
 * @param target_name the target system/library call name.
 * @param mocked the function to replace the target call with. this should only be
 *  given if an autostub cannot be used on the specified system/library call (see more above).
 * @param action the action associated with the autostub used in this mock.
 * @param set_errno should the autostub associated with this mock set errno?
 * @return an allocated mock structure ready to be applied.
 */
TAPI_EXPORT tapi_mock_t* TAPI
tapi_make_auto_mock(void* orig, const char* target_name, void* mocked, \
    tapi_action_t action, bool set_errno);
#endif

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
TAPI_EXPORT void TAPI
tapi_apply_mock(tapi_context_t* context, tapi_mock_t* mock);

/**
 * @brief restore the contents of a function and free the mock. this frees all the data held by
 *  the mock, the pointer itself and the guard held within the context. 'mock' should not be used
 *  after this is called, either copy the data required before this call or create a new mock.
 *
 * @param context the context of tapi to be used.
 * @param mock the mock structure to be freed and restored.
 */
TAPI_EXPORT void TAPI
tapi_cleanup_mock(tapi_context_t* context, tapi_mock_t* mock);

/** quickly create an action function to be used in an autostub. */
#define tapi_action(action_name, ...) \
    e_tapi_action_result_t action_name(void* blank, __VA_ARGS__)

/** quickly create a test with an automock to the test suite. */
#define tapi_add_test_and_auto_mock(context, name, test_function, tested_function, target_name, \
    stub_function, action, set_errno) \
    tapi_test_t* TAPI_CONCAT(_gentest_, __LINE__) = tapi_make_test(name, test_function); \
    tapi_mock_t* TAPI_CONCAT(_genmock_, __LINE__) = tapi_make_auto_mock(tested_function, \
        target_name, stub_function, action, set_errno); \
    tapi_dyna_push(TAPI_CONCAT(_gentest_, __LINE__)->mocks, TAPI_CONCAT(_genmock_, __LINE__)); \
    tapi_dyna_push(context->tests, TAPI_CONCAT(_gentest_, __LINE__));

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

/** quickly add a test with a mock to the test suite. */
#define tapi_add_test_and_mock(context, name, test_function, tested_function, target_function, \
    stub_function) \
    tapi_test_t* TAPI_CONCAT(_gentest_, __LINE__) = tapi_make_test(name, test_function); \
    tapi_mock_t* TAPI_CONCAT(_genmock_, __LINE__) = tapi_make_mock(tested_function, \
        target_function, stub_function, 0u); \
    tapi_dyna_push(TAPI_CONCAT(_gentest_, __LINE__)->mocks, TAPI_CONCAT(_genmock_, __LINE__)); \
    tapi_dyna_push(context->tests, TAPI_CONCAT(_gentest_, __LINE__));

/** quickly add a test with a mock on the n-th call to the test suite. */
#define tapi_add_test_and_special_mock(context, name, test_function, tested_function, \
    target_function, stub_function, n) \
    tapi_test_t* TAPI_CONCAT(_gentest_, __LINE__) = tapi_make_test(name, test_function); \
    tapi_mock_t* TAPI_CONCAT(_genmock_, __LINE__) = tapi_make_mock(tested_function, \
        target_function, stub_function, n); \
    tapi_dyna_push(TAPI_CONCAT(_gentest_, __LINE__)->mocks, TAPI_CONCAT(_genmock_, __LINE__)); \
    tapi_dyna_push(context->tests, TAPI_CONCAT(_gentest_, __LINE__));
#endif /* TAPI_MOCK_H */