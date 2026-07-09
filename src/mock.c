/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-07-09
 */
#include <tapi/mock.h>

/*! uses cs_open, csh, etc... */
#include <capstone/capstone.h>

/*! uses memcpy. */
#include <string.h>

/*! uses errno. */
#include <errno.h>

/*! uses det_function_size. */
#include "det.h"

/*! uses patch_call_target. */
#include "patch.h"

/*! uses plt_resolve. */
#include "plt.h"

/*! uses internal. */
#include "int/intt.h"
/** \endcond */

/** @brief malloc autostub used by tapi. */
void*
tapi_stub_malloc(size_t size);

/** @brief calloc autostub used by tapi. */
void*
tapi_stub_calloc(size_t nmemb, size_t size);

/** @brief free autostub used by tapi. */
void
tapi_stub_free(void* ptr);

/** the internal autostub table used for automocks. */
internal tapi_autostub_t autostub_table[3u] = {
    { /* malloc. */
        .action = 0x0,
        .stub = tapi_stub_malloc,
        .name = "malloc",
        .set_errno = false,
    },
    { /* calloc. */
        .action = 0x0,
        .stub = tapi_stub_calloc,
        .name = "calloc",
        .set_errno = false,
    },
    { /* free. */
        .action = 0x0,
        .stub = tapi_stub_free,
        .name = "free",
        .set_errno = false,
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
            /* regular fail on malloc, if set_errno then ENOMEM. */
            if (autostub.set_errno) errno = ENOMEM;
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
            /* regular fail on calloc, if set_errno then ENOMEM... */
            if (autostub.set_errno) errno = ENOMEM;
            return 0x0;
        }
    }
    /* proceed as per usual. */
    return calloc(nmemb, size);
};

/** @brief free autostub used by tapi. */
void
tapi_stub_free(void* ptr) {
    tapi_autostub_t autostub = autostub_table[2u]; /* get the free autostub. */
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, ptr);
        if (result == E_TAPI_ACTION_RESULT_FAIL) {
            /* regular fail on free... */
            return;
        }
    }
    /* proceed as per usual. */
    free(ptr);
};

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
tapi_mock_t*
tapi_make_mock(void* orig, void* target, void* mocked, size_t call_index) {
    /* allocate the structure. */
    tapi_mock_t* mock = calloc(1u, sizeof *mock);
    mock->orig = orig;
    mock->target = target;
    mock->mocked = mocked;
    mock->call_index = call_index;
    mock->fun_size = det_function_size(orig, TAPI_MAX_DET_DEPTH);
    /* we are using a max of 4096 bytes (by default). */
    mock->type = call_index != 0u ? E_TAPI_MOCK_SPECIAL : E_TAPI_MOCK_REGULAR;
    mock->autostub = 0x0;
    return mock;
};

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
 * @return an allocated mock structure ready to be applied.
 */
tapi_mock_t*
tapi_make_auto_mock(void* orig, const char* target_name, void* mocked, tapi_action_t action) {
    /* allocate the structure. */
    tapi_mock_t* mock = calloc(1u, sizeof *mock);
    mock->orig = orig;
    mock->target = plt_resolve(target_name);
    if (mock->target == 0x0) {
        fprintf(stderr, "tapi_make_auto_mock; failed to resolve " \
                        "plt address for %s!\n", target_name);
        free(mock);
        return 0x0;
    }
    mock->mocked = mocked;
    mock->call_index = 0u;
    mock->fun_size = det_function_size(orig, TAPI_MAX_DET_DEPTH);
    /* we are using a max of 4096 bytes (by default). */
    mock->type = E_TAPI_MOCK_AUTO;
    /* simply iterate through the table of given autostubs, find it if possible. */
    mock->autostub = 0x0;
    for (size_t i = 0u; i < sizeof(autostub_table) / sizeof(tapi_autostub_t); i++) {
        if (!strcmp(target_name, autostub_table[i].name)) {
            mock->autostub = &autostub_table[i];
            mock->mocked = autostub_table[i].stub; /* set the stub. */
        }
    }
    return mock;
};

/**
 * @brief apply a call target (or multiple) patch in memory to route to the given stub by the
 * mock. this is usually performed by `tapi_run_tests`, but can be used under special
 * circumstances if required.
 *
 * @param context the context of tapi to be used.
 * @param mock the mock to be applied.
 */
void
tapi_apply_mock(tapi_context_t* context, tapi_mock_t* mock) {
    /* we find all call targets to the target, then from there try to get the n-th index. */
    if (mock->type == E_TAPI_MOCK_SPECIAL) {
        tapi_dyna_t* list = det_call_targets(mock->orig, mock->target);
        if (list == 0x0) return;

        /* iterate through call call targets. */
        if (list->length + 1u < mock->call_index) {
            fprintf(stderr, "tapi_apply_mock; cannot find index %d for mock!\n", mock->call_index);
            return;
        }

        /* get the call and work from there. */
        det_call_t* call = dyna_get(list, det_call_t*, mock->call_index - 1u);
        if (call == 0x0) return;
        mock->call = call->call;
        mock->size = call->size;
        /* NOLINTNEXTLINE */
        memcpy(mock->orig_bytes, call->bytes, mock->size);

        /* apply the patch to the call, given the context. */
        patch_call_target(context, call, mock->mocked);

        /* we read the new bytes and store. */
        /* NOLINTNEXTLINE */
        memcpy(mock->mocked_bytes, mock->call, mock->size);
        dyna_foreach(list, det_call_t*, call)
            free(call);
        dyna_endforeach(list)
        tapi_dyna_free(list);
        return;
    }

    /* if this is a 'regular/auto mock' we need to find ALL occurrences of this call target. */
    for (size_t i = 0u; i < mock->fun_size; i++) {;
        det_call_t* call = det_call_target(mock->orig, mock->target);
        if (call == 0x0) break;

        /* if this is the first occurrence, we set the size and length of the call (they
         * shouldn't change since they are identical for multiple occurrences). */
        if (i == 0u) {
            mock->call = call->call;
            mock->size = call->size;
        }
        /* NOLINTNEXTLINE */
        memcpy(mock->orig_bytes, call->bytes, mock->size);

        /* apply the patch to the call, given the context. */
        patch_call_target(context, call, mock->mocked);

        /* we read the new bytes and store. */
        /* NOLINTNEXTLINE */
        memcpy(mock->mocked_bytes, mock->call, mock->size);
        free(call);
    }
};

/**
 * @brief restore the contents of a function and free the mock. this frees all the data held by
 *  the mock, the pointer itself and the guard held within the context. 'mock' should not be used
 *  after this is called, either copy the data required before this call or create a new mock.
 *
 * @param context the context of tapi to be used.
 * @param mock the mock structure to be freed and restored.
 */
void
tapi_cleanup_mock(tapi_context_t* context, tapi_mock_t* mock) {
    /* we can't restore a mock that hasn't been applied... */
    if (mock->call == 0x0) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi, mock_restore; cannot restore unapplied mock.\n");
        return;
    }

    /* is this a 'special/auto mock', if so we have to replace every occurrence. */
    if (mock->type != E_TAPI_MOCK_SPECIAL) {
        for (size_t i = 0u; i < mock->fun_size; i++) {
            det_call_t* call = det_call_target(mock->orig, mock->target);
            if (call == 0x0) break;

            /* otherwise we re-patch it with the correct call. */
            patch_call_target(context, call, mock->mocked);
            free(call);
        }

        /* reset the autostub. */
        if (mock->autostub != 0x0) {
            mock->autostub->action = 0x0;
            mock->autostub->set_errno = false;
        }
        free(mock);
    }
    else {
        /* we then have to restore the bytes for future tests that could call that same function. */
        det_call_t* call = det_call_target(mock->orig, mock->mocked);
        patch_call_target(context, call, mock->target);
        free(call);
        free(mock);
    }
};