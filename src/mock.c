/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-08-21
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

/*! uses lnk_resolve, lnk_qr_thunk. */
#include "lnk.h"

/*! uses reloc_lookup. */
#include "reloc.h"

/*! uses internal. */
#include "int/intt.h"
/** \endcond */


/*! -----------------autostubs------------------ !*/
#ifndef TAPI_MINIMAL

/** @brief malloc autostub used by tapi. */
void*
tapi_stub_malloc(size_t size);

/** @brief calloc autostub used by tapi. */
void*
tapi_stub_calloc(size_t nmemb, size_t size);

/** @brief free autostub used by tapi. */
void
tapi_stub_free(void* ptr);

/** @brief realloc autostub used by tapi. */
void*
tapi_stub_realloc(void* ptr, size_t size);

/** @brief strlen autostub used by tapi. */
size_t
tapi_stub_strlen(const char* s);

/** @brief strcmp autostub used by tapi. */
int
tapi_stub_strcmp(const char* s1, const char* s2);

/** @brief strncmp autostub used by tapi. */
int
tapi_stub_strncmp(const char* s1, const char* s2, size_t n);

/** @brief strcpy autostub used by tapi. */
char*
tapi_stub_strcpy(const char* s1, const char* s2);

/** @brief strncpy autostub used by tapi. */
char*
tapi_stub_strncpy(const char* s1, const char* s2);

/** @brief memcpy autostub used by tapi. */
void*
tapi_stub_memcpy(void* dest, const void* src, size_t n);

/** @brief memmove autostub used by tapi. */
void*
tapi_stub_memmove(void* dest, const void* src, size_t n);

/** @brief memset autostub used by tapi. */
void*
tapi_stub_memset(void* s, int c, size_t n);

/** @brief strcat autostub used by tapi. */
char*
tapi_stub_strcat(char* dest, const char* src);

/** @brief strncat autostub used by tapi. */
char*
tapi_stub_strncat(char* dest, const char* src, size_t ssize);

/** @brief printf autostub used by tapi. */
int
tapi_stub_printf(const char* format, ...);

/** @brief fprintf autostub used by tapi. */
int
tapi_stub_fprintf(FILE* stream, const char* format, ...);

/** @brief sprintf autostub used by tapi. */
int
tapi_stub_sprintf(char* str, const char* format, ...);

/** @brief snprintf autostub used by tapi. */
int
tapi_stub_snprintf(char* str, size_t size, const char* format, ...);

/** @brief vsnprintf autostub used by tapi. */
int
tapi_stub_vsnprintf(char* str, size_t size, const char* format, va_list ap);

/** @brief open autostub used by tapi. */
int
tapi_stub_open(const char* pathname, int flags, ...);

/** @brief fopen autostub used by tapi. */
FILE*
tapi_stub_fopen(const char* filename, const char* mode);

/** @brief freopen autostub used by tapi. */
FILE*
tapi_stub_freopen(const char* filename, const char* mode, FILE* stream);

/** @brief read autostub used by tapi. */
ssize_t
tapi_stub_read(int fd, const void* buf, size_t count);

/** @brief fread autostub used by tapi. */
size_t
tapi_stub_fread(void* ptr, size_t size, size_t nmemb, FILE* stream);

/** @brief write autostub used by tapi. */
ssize_t
tapi_stub_write(int fd, const void* buf, size_t count);

/** @brief fwrite autostub used by tapi. */
size_t
tapi_stub_fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);

/** @brief close autostub used by tapi. */
int
tapi_stub_close(int fd);

/** @brief close autostub used by tapi. */
int
tapi_stub_fclose(FILE* stream);

/** @brief getenv autostub used by tapi. */
char*
tapi_stub_getenv(const char* name);

/** @brief getpid autostub used by tapi. */
pid_t
tapi_stub_getpid(void);

/** @brief time autostub used by tapi. */
time_t
tapi_stub_time(time_t* tloc);

/** @brief rand autostub used by tapi. */
int
tapi_stub_rand(void);

/** the internal autostub table used for automocks. */
internal tapi_autostub_t autostub_table[53u] = {
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
    },
    { /* realloc. */
        .action = 0x0,
        .stub = tapi_stub_realloc,
        .name = "realloc",
        .set_errno = false,
    },
    { /* strlen. */
        .action = 0x0,
        .stub = tapi_stub_strlen,
        .name = "strlen",
        .set_errno = false,
    },
    { /* strcmp. */
        .action = 0x0,
        .stub = tapi_stub_strcmp,
        .name = "strcmp",
        .set_errno = false,
    },
    { /* strncmp. */
        .action = 0x0,
        .stub = tapi_stub_strncmp,
        .name = "strncmp",
        .set_errno = false,
    },
    { /* strcpy. */
        .action = 0x0,
        .stub = tapi_stub_strcpy,
        .name = "strcpy",
        .set_errno = false,
    },
    { /* memcpy. */
        .action = 0x0,
        .stub = tapi_stub_memcpy,
        .name = "memcpy",
        .set_errno = false,
    },
    { /* memmove. */
        .action = 0x0,
        .stub = tapi_stub_memmove,
        .name = "memmove",
        .set_errno = false,
    },
    { /* strcat. */
        .action = 0x0,
        .stub = tapi_stub_strcat,
        .name = "strcat",
        .set_errno = false,
    },
    { /* strncat. */
        .action = 0x0,
        .stub = tapi_stub_strncat,
        .name = "strncat",
        .set_errno = false,
    },
    { /* printf. */
        .action = 0x0,
        .stub = tapi_stub_printf,
        .name = "printf",
        .set_errno = false,
    },
    { /* fprintf. */
        .action = 0x0,
        .stub = tapi_stub_fprintf,
        .name = "fprintf",
        .set_errno = false,
    },
    { /* sprintf. */
        .action = 0x0,
        .stub = tapi_stub_sprintf,
        .name = "sprintf",
        .set_errno = false,
    },
    { /* vsprintf. */
        .action = 0x0,
        .stub = tapi_stub_vsprintf,
        .name = "vsprintf",
        .set_errno = false,
    },
    { /* open. */
        .action = 0x0,
        .stub = tapi_stub_open,
        .name = "open",
        .set_errno = false,
    },
    { /* fopen. */
        .action = 0x0,
        .stub = tapi_stub_fopen,
        .name = "fopen",
        .set_errno = false,
    },
    { /* freopen. */
        .action = 0x0,
        .stub = tapi_stub_freopen,
        .name = "freopen",
        .set_errno = false,
    },
    { /* read. */
        .action = 0x0,
        .stub = tapi_stub_read,
        .name = "read",
        .set_errno = false,
    },
    { /* fread. */
        .action = 0x0,
        .stub = tapi_stub_fread,
        .name = "fread",
        .set_errno = false,
    },
    { /* write. */
        .action = 0x0,
        .stub = tapi_stub_write,
        .name = "write",
        .set_errno = false,
    },
    { /* fwrite. */
        .action = 0x0,
        .stub = tapi_stub_fwrite,
        .name = "fwrite",
        .set_errno = false,
    },
    { /* close. */
        .action = 0x0,
        .stub = tapi_stub_close,
        .name = "close",
        .set_errno = false,
    },
    { /* fclose. */
        .action = 0x0,
        .stub = tapi_stub_fclose,
        .name = "fclose",
        .set_errno = false,
    },
    { /* getenv. */
        .action = 0x0,
        .stub = tapi_stub_getenv,
        .name = "getenv",
        .set_errno = false,
    },
    { /* getpid. */
        .action = 0x0,
        .stub = tapi_stub_getpid,
        .name = "getpid",
        .set_errno = false,
    },
    { /* time. */
        .action = 0x0,
        .stub = tapi_stub_time,
        .name = "time",
        .set_errno = false,
    },
    { /* rand. */
        .action = 0x0,
        .stub = tapi_stub_rand,
        .name = "rand",
        .set_errno = false,
    },
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

/** @brief realloc autostub used by tapi. */
void*
tapi_stub_realloc(void* ptr, size_t size) {
    tapi_autostub_t autostub = autostub_table[3u]; /* get the realloc autostub. */
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, ptr, size);
        if (result == E_TAPI_ACTION_RESULT_FAIL) {
            /* regular fail on realloc, if set_errno then ENOMEM... */
            if (autostub.set_errno) errno = ENOMEM;
            return 0x0;
        }
    }
    /* proceed as per usual. */
    return realloc(ptr, size);
};

#endif
/*! ----------------------==---------------------- !*/

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
#ifndef TAPI_WINDOWS
    mock->orig = orig;
#else
    mock->orig = lnk_qr_thunk(orig);  /* on windows, most calls are to the iat thunk and not to the function itself. */
#endif
    mock->target = target;
    mock->mocked = mocked;
    mock->call_index = call_index;
    mock->calls = tapi_dyna_create();
    mock->fun_size = det_function_size(orig, TAPI_MAX_DET_DEPTH);
    /* we are using a max of 4096 bytes (by default). */
    mock->type = call_index != 0u ? E_TAPI_MOCK_SPECIAL : E_TAPI_MOCK_REGULAR;
    return mock;
};

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
tapi_mock_t*
tapi_make_auto_mock(void* orig, const char* target_name, void* mocked, \
    tapi_action_t action, bool set_errno) {
    /* allocate the structure. */
    tapi_mock_t* mock = calloc(1u, sizeof *mock);
#ifndef TAPI_WINDOWS
    mock->orig = orig;
#else
    mock->orig = lnk_qr_thunk(orig); /* again most calls are to the thunk, no reloc is needed. */
#endif
    mock->target = lnk_resolve(target_name);
    if (mock->target == 0x0) {
        fprintf(stderr, "tapi_make_auto_mock; failed to resolve " \
                        "plt/got/iat address for %s!\n", target_name);
        free(mock);
        return 0x0;
    }
    mock->mocked = mocked;
    mock->call_index = 0u;
    mock->calls = tapi_dyna_create();
    mock->fun_size = det_function_size(orig, TAPI_MAX_DET_DEPTH);
    /* we are using a max of 4096 bytes (by default). */
    mock->type = E_TAPI_MOCK_AUTO;
    /* simply iterate through the table of given autostubs, find it if possible. */
    mock->data.info.autostub = 0x0;
    mock->data.info.action = action;
    mock->data.info.set_errno = set_errno;
    for (size_t i = 0u; i < sizeof(autostub_table) / sizeof(tapi_autostub_t); i++) {
        if (!strcmp(target_name, autostub_table[i].name))
            mock->data.info.autostub = &autostub_table[i];
    }
    return mock;
};
#endif

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
            fprintf(stderr, "tapi_apply_mock; cannot find index %zu for mock!\n", mock->call_index);
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
        int32_t result = patch_call_target(context, call, mock->mocked);

        /* we read the new bytes and store. */
        /* NOLINTNEXTLINE */
        memcpy(mock->mocked_bytes, mock->call, mock->size);
        tapi_dyna_push(mock->calls, call);
        tapi_dyna_pop(list, mock->call_index - 1u);
        dyna_foreach(list, det_call_t*, other)
            free(other);
        dyna_endforeach(list)
        tapi_dyna_free(list);
        return;
    }

#ifndef TAPI_MINIMAL
    /* for every 'auto mock' we change the details of the internal table before the target is called. */
    if (mock->type == E_TAPI_MOCK_AUTO && mock->data.info.autostub != 0x0) {
        mock->data.info.autostub->action = mock->data.info.action;
        mock->data.info.autostub->set_errno = mock->data.info.set_errno;
        mock->mocked = mock->data.info.autostub->stub;
    }
#endif

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
        int32_t result = patch_call_target(context, call, mock->mocked);

        /* we read the new bytes and store. */
        /* NOLINTNEXTLINE */
        memcpy(mock->mocked_bytes, mock->call, mock->size);
        tapi_dyna_push(mock->calls, call);
        if (result == 0u) break;
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
    if (mock->type != E_TAPI_MOCK_AUTO && mock->call == 0x0) {
        /* NOLINTNEXTLINE */
        fprintf(stderr, "tapi_cleanup_mock; cannot restore unapplied mock.\n");
        return;
    }

    /* is this a 'regular/auto mock', if so we have to replace every occurrence. */
    if (mock->type != E_TAPI_MOCK_SPECIAL) {
        det_call_t* first = tapi_dyna_get(mock->calls, 0u);
        dyna_foreach(mock->calls, det_call_t*, call)
            /* we replace the call target with the original bytes, iff they are the
             * same type of call (always except for x86 rip-based ind. calls). */
            if (first->spec == call->spec) {
                memcpy(call->call, mock->orig_bytes, call->size);
                continue;
            }

            /* otherwise we re-patch it with the old call address and type we had. */
            int32_t result = patch_call_target(context, call, mock->target);
            if (result == 0u) {
                /* NOLINTNEXTLINE */
                fprintf(stderr, "tapi_cleanup_mock; cannot patch original target!\n");
                break;
            }
        dyna_endforeach(mock->calls)

#ifndef TAPI_MINIMAL
        /* reset the autostub. */
        if (mock->data.info.autostub != 0x0) {
            mock->data.info.autostub->action = 0x0;
            mock->data.info.autostub->set_errno = false;
        }

        /* free each call captured. */
        dyna_foreach(mock->calls, det_call_t*, iter_call)
            free(iter_call);
        dyna_endforeach(mock->calls)
#endif
    }
    else {
        /* we then have to restore the call target for future tests. */
        det_call_t* call = tapi_dyna_pop(mock->calls, 0u);
        memcpy(call->call, mock->orig_bytes, call->size);
        free(call);
    }
    tapi_dyna_free(mock->calls);
    free(mock);
};