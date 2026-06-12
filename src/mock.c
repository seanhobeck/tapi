/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-06-12
 */
#include <tapi/mock.h>

/*! @uses cs_open. */
#include <capstone/capstone.h>

/*! @uses memcpy. */
#include <string.h>

/*! @uses det_function_size. */
#include "det.h"

/*! @uses patch_call_target. */
#include "patch.h"

/*! @uses internal. */
#include "int/intt.h"
/** \endcond */

/* see tapi/mock.h for more info... */
#ifdef TAPI_AUTOSTUB
internal tapi_autostub_t autostub_table[3u] = {
    { /* malloc. */
        .action = 0x0,
        .stub = tapi_stub_malloc,
        .name = "malloc",
    },
    { /* calloc. */
        .action = 0x0,
        .stub = tapi_stub_calloc,
        .name = "calloc",
    },
    { /* free. */
        .action = 0x0,
        .stub = tapi_stub_free,
        .name = "free",
    }
};

/** @brief malloc autostub used by tapi. */
void*
tapi_stub_malloc(size_t size) {
    tapi_autostub_t autostub = autostub_table[0u]; /* get the malloc autostub. */
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, size);
        if (result == E_TAPI_ACTION_RESULT_FAIL) {
            /* regular fail on malloc, does NOT set errno. todo; add support for setting errno. */
            return 0x0;
        }
    }
    /* proceed as per usual. */
    return malloc(size);
};

/** @brief calloc autostub used by tapi. */
void*
tapi_stub_calloc(size_t nmemb, size_t size) {
    tapi_autostub_t autostub = autostub_table[1u]; /* get the calloc autostub. */
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, nmemb, size);
        if (result == E_TAPI_ACTION_RESULT_FAIL) {
            /* regular fail on calloc... */
            return 0x0;
        }
    }
    /* proceed as per usual. */
    return calloc(nmemb, size);
};

/** @brief free autostub used by tapi. */
void
tapi_stub_free(void* ptr) {
    tapi_autostub_t autostub = autostub_table[1u]; /* get the free autostub. */
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, ptr);
        if (result == E_TAPI_ACTION_RESULT_FAIL) {
            /* regular fail on calloc... */
            return;
        }
    }
    /* proceed as per usual. */
    free(ptr);
};
#endif

/**
 * @brief mock the first call occurrence to a target with a call
 *  to a mocked function instead.
 *
 * @param orig the original function to search for target in.
 * @param target the target address to be replaced.
 * @param mocked the function to replace the target call with.
 * @return an allocated mock structure ready to be applied.
 */
tapi_mock_t*
tapi_mock_create(void* orig, void* target, void* mocked) {
    /* allocate the structure. */
    tapi_mock_t* mock = calloc(1u, sizeof *mock);
    mock->orig = orig;
    mock->target = target;
    mock->mocked = mocked;
    mock->fun_size = det_function_size(orig, TAPI_MAX_DET_DEPTH);
    mock->is_special = false;
#ifdef TAPI_AUTOSTUB
    mock->autostub = 0x0;
#endif
    /* we are using a max of 4096 bytes (by default). */
    return mock;
};

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
 *  in the table specified above @see { special_table }, then no
 *  address is required (0x0).
 * @param action the action/ condition function that allows the
 *  mock to either pass or fail based on certain conditions.
 *
 * @return an allocated mock structure ready to be applied.
 */
tapi_mock_t*
tapi_special_mock_create(void* orig, void* target, \
    void* mocked, tapi_action_t action) {
    return 0x0;
};

/**
 * @brief apply the mocks patch in memory; write stub to route to
 *  the given mocked function pointer.
 *
 * @param context the context of tapi to be used.
 * @param mock the mock to be applied.
 */
void
tapi_mock_apply(tapi_context_t* context, tapi_mock_t* mock) {
    /* determine call info. */
    det_call_t* call = det_call_target(mock->orig, mock->target);
    if (call == 0x0) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi, mock_apply; cannot find target call in function.\n");
        return;
    }

    mock->call = call->call;
    mock->size = call->size;
    /* NOLINTNEXTLINE */
    memcpy(mock->orig_bytes, call->bytes, mock->size);

    /* apply the patch to the call, given the context. */
    patch_call_target(context, call, mock->mocked);

    /* we read the new bytes and store. */
    /* NOLINTNEXTLINE */
    memcpy(mock->mocked_bytes, mock->call, mock->size);
    free(call);
};

/**
 * @brief restore the contents of a function and free the mock.
 *
 * @param context the context of tapi to be used.
 * @param mock the mock structure to be freed and restored.
 */
void
tapi_mock_restore(tapi_context_t* context, tapi_mock_t* mock) {
    /* we can't restore a mock that hasn't been applied... */
    if (mock->call == 0x0) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi, mock_restore; cannot restore unapplied mock.\n");
        return;
    }

    /* we then have to restore the bytes for future tests that could call that same function. */
    det_call_t* call = det_call_target(mock->orig, mock->mocked);
    patch_call_target(context, call, mock->target);
    free(call);
    free(mock);
};