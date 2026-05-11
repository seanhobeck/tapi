/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-03-09
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
/** \endcond */

/**
 * @brief mock the first call occurrence to a target with a call
 *  to a mocked function instead.
 *
 * @param orig the original function to search for target in.
 * @param target the target address to be replaced.
 * @param mocked the function to replace the target call with.
 * @return an allocated mock structure with all data, ready to be applied.
 */
tapi_mock_t*
tapi_mock_create(void* orig, void* target, void* mocked) {
    /* allocate the structure. */
    tapi_mock_t* mock = calloc(1u, sizeof *mock);
    mock->orig = orig;
    mock->target = target;
    mock->mocked = mocked;
    mock->fun_size = det_function_size(orig, 0x1000); /* we are using a max of 4096 bytes. */
    return mock;
};

/**
 * @brief apply the mocks patch in memory; write stub to route to
 *  the given mocked function pointer.
 *
 * @param mock the mock to be applied.
 */
void
tapi_mock_apply(tapi_mock_t* mock) {
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

    /* apply the patch to the call. */
    patch_call_target(call, mock->mocked);

    /* we read the new bytes and store. */
    /* NOLINTNEXTLINE */
    memcpy(mock->mocked_bytes, mock->call, mock->size);
    free(call);
};

/**
 * @brief restore the contents of a function and free the mock.
 *
 * @param mock the mock structure to be freed and restored.
 */
void
tapi_mock_restore(tapi_mock_t* mock) {
    /* we can't restore a mock that hasn't been applied... */
    if (mock->call == 0x0) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi, mock_restore; cannot restore unapplied mock.\n");
        return;
    }

    /* we then have to restore the bytes for future tests that could call that same function. */
    det_call_t* call = det_call_target(mock->orig, mock->mocked);
    patch_call_target(call, mock->target);
    free(call);
    free(mock);
};