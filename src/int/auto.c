/**
 * @author Sean Hobeck
 * @date 2026-09-07
 */
#include "auto.h"

/*! uses FILE*. */
#include <stdio.h>

/*! uses size_t. */
#include <stdlib.h>

/*! uses strcmp. */
#include <string.h>

/*! uses true, false, bool. */
#include <stdbool.h>

/*! uses ENOMEM, other errnos... */
#include <errno.h>

/*! uses va_list. */
#include <stdarg.h>

/*! uses internal. */
#include "intt.h"

/*! -----------------autostubs------------------ !*/
#ifndef TAPI_MINIMAL

/** @brief malloc autostub used by tapi. */
void*
stub_malloc(size_t size);

/** @brief calloc autostub used by tapi. */
void*
stub_calloc(size_t nmemb, size_t size);

/** @brief free autostub used by tapi. */
void
stub_free(void* ptr);

/** @brief realloc autostub used by tapi. */
void*
stub_realloc(void* ptr, size_t size);

/** @brief strlen autostub used by tapi. */
size_t
stub_strlen(const char* s);

/** @brief strcmp autostub used by tapi. */
int
stub_strcmp(const char* s1, const char* s2);

/** @brief strncmp autostub used by tapi. */
int
stub_strncmp(const char* s1, const char* s2, size_t n);

/** @brief strcpy autostub used by tapi. */
char*
stub_strcpy(const char* s1, const char* s2);

/** @brief strncpy autostub used by tapi. */
char*
stub_strncpy(const char* s1, const char* s2, size_t n);

/** @brief memcpy autostub used by tapi. */
void*
stub_memcpy(void* dest, const void* src, size_t n);

/** @brief memmove autostub used by tapi. */
void*
stub_memmove(void* dest, const void* src, size_t n);

/** @brief memset autostub used by tapi. */
void*
stub_memset(void* s, int c, size_t n);

/** @brief printf autostub used by tapi. */
int
stub_printf(const char* format, ...);

/** @brief fprintf autostub used by tapi. */
int
stub_fprintf(FILE* stream, const char* format, ...);

/** @brief sprintf autostub used by tapi. */
int
stub_sprintf(char* str, const char* format, ...);

/** @brief snprintf autostub used by tapi. */
int
stub_snprintf(char* str, size_t size, const char* format, ...);

/** @brief vsprintf autostub used by tapi. */
int
stub_vsprintf(char* str, const char* format, va_list ap);

/** @brief open autostub used by tapi. */
int
stub_open(const char* pathname, int flags, ...);

/** @brief fopen autostub used by tapi. */
FILE*
stub_fopen(const char* filename, const char* mode);

/** @brief freopen autostub used by tapi. */
FILE*
stub_freopen(const char* filename, const char* mode, FILE* stream);

/** @brief read autostub used by tapi. */
ssize_t
stub_read(int fd, const void* buf, size_t count);

/** @brief fread autostub used by tapi. */
size_t
stub_fread(void* ptr, size_t size, size_t nmemb, FILE* stream);

/** @brief write autostub used by tapi. */
ssize_t
stub_write(int fd, const void* buf, size_t count);

/** @brief fwrite autostub used by tapi. */
size_t
stub_fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);

/** @brief close autostub used by tapi. */
int
stub_close(int fd);

/** @brief close autostub used by tapi. */
int
stub_fclose(FILE* stream);

/** @brief getenv autostub used by tapi. */
char*
stub_getenv(const char* name);

/** @brief getpid autostub used by tapi. */
pid_t
stub_getpid(void);

/** @brief time autostub used by tapi. */
time_t
stub_time(time_t* tloc);

/** @brief rand autostub used by tapi. */
int
stub_rand(void);

#ifdef TAPI_WINDOWS
/** @brief strcpy_s autostub used by tapi. */
errno_t
stub_strcpy_s(char* dest, rsize_t dest_size, const char* src);

/** @brief strncpy_s autostub used by tapi. */
errno_t
stub_strncpy_s(char* dest, size_t num_elems, \
    const char* src, size_t count);

/** @brief memcpy_s autostub used by tapi. */
errno_t
stub_memcpy_s(void* dest, size_t dest_size, \
    const void* src, size_t count);

/** @brief memmove_s autostub used by tapi. */
errno_t
stub_memmove_s(void* dest, size_t num_elems, \
    const void* src, size_t count);

/** @brief printf_s autostub used by tapi. */
int
stub_printf_s(const char* format, ...);

/** @brief fprintf_s autostub used by tapi. */
int
stub_fprintf_s(FILE* stream, const char* format, ...);

/** @brief sprintf_s autostub used by tapi. */
int
stub_sprintf_s(char* dest, size_t dest_size, \
    const char* format, ...);

/** @brief _snprintf_s autostub used by tapi. */
int
stub_snprintf_s(char* dest, size_t dest_size, \
    size_t count, const char* format, ...);

/** @brief vsprintf_s autostub used by tapi. */
int
stub_vsprintf_s(char* dest, size_t num_elems, \
    const char* format, va_list ap);

/** @brief _sopen_s autostub used by tapi. */
errno_t
stub_sopen_s(int* pfh, const char* filename, \
    int oflag, int shflag, int pmode);

/** @brief fopen_s autostub used by tapi. */
errno_t
stub_fopen_s(FILE** stream, const char* filename, \
    const char* mode);

/** @brief freopen_s autostub used by tapi. */
errno_t
stub_freopen_s(FILE** stream, const char* filename, \
    const char* mode, FILE* old_stream);

/** @brief _read autostub used by tapi. */
int
stub__read(const int fd, void* buffer, const unsigned int buffer_size);

/** @brief fread_s autostub used by tapi. */
size_t
stub_fread_s(void* buffer, size_t buffer_size, \
    size_t element_size, size_t count, FILE* stream);

/** @brief _write autostub used by tapi. */
int
stub__write(const int fd, void* buffer, const unsigned int count);

/** @brief _close autostub used by tapi. */
int
stub__close(const int fd);

/** @brief rand_s autostub used by tapi. */
errno_t
stub_rand_s(unsigned int* random_value);
#endif

/** the internal autostub table used for automocks. */
internal tapi_autostub_t autostub_table[43u] = {
    { /* malloc. */
        .action = 0x0,
        .stub = stub_malloc,
        .name = "malloc",
        .set_errno = false,
    },
    { /* calloc. */
        .action = 0x0,
        .stub = stub_calloc,
        .name = "calloc",
        .set_errno = false,
    },
    { /* free. */
        .action = 0x0,
        .stub = stub_free,
        .name = "free",
        .set_errno = false,
    },
    { /* realloc. */
        .action = 0x0,
        .stub = stub_realloc,
        .name = "realloc",
        .set_errno = false,
    },
    { /* strlen. */
        .action = 0x0,
        .stub = stub_strlen,
        .name = "strlen",
        .set_errno = false,
    },
    { /* strcmp. */
        .action = 0x0,
        .stub = stub_strcmp,
        .name = "strcmp",
        .set_errno = false,
    },
    { /* strncmp. */
        .action = 0x0,
        .stub = stub_strncmp,
        .name = "strncmp",
        .set_errno = false,
    },
    { /* strcpy. */
        .action = 0x0,
        .stub = stub_strcpy,
        .name = "strcpy",
        .set_errno = false,
    },
    { /* memcpy. */
        .action = 0x0,
        .stub = stub_memcpy,
        .name = "memcpy",
        .set_errno = false,
    },
    { /* memmove. */
        .action = 0x0,
        .stub = stub_memmove,
        .name = "memmove",
        .set_errno = false,
    },
    { /* printf. */
        .action = 0x0,
        .stub = stub_printf,
        .name = "printf",
        .set_errno = false,
    },
    { /* fprintf. */
        .action = 0x0,
        .stub = stub_fprintf,
        .name = "fprintf",
        .set_errno = false,
    },
    { /* sprintf. */
        .action = 0x0,
        .stub = stub_sprintf,
        .name = "sprintf",
        .set_errno = false,
    },
    { /* vsprintf. */
        .action = 0x0,
        .stub = stub_vsprintf,
        .name = "vsprintf",
        .set_errno = false,
    },
    { /* open. */
        .action = 0x0,
        .stub = stub_open,
        .name = "open",
        .set_errno = false,
    },
    { /* fopen. */
        .action = 0x0,
        .stub = stub_fopen,
        .name = "fopen",
        .set_errno = false,
    },
    { /* freopen. */
        .action = 0x0,
        .stub = stub_freopen,
        .name = "freopen",
        .set_errno = false,
    },
    { /* read. */
        .action = 0x0,
        .stub = stub_read,
        .name = "read",
        .set_errno = false,
    },
    { /* fread. */
        .action = 0x0,
        .stub = stub_fread,
        .name = "fread",
        .set_errno = false,
    },
    { /* write. */
        .action = 0x0,
        .stub = stub_write,
        .name = "write",
        .set_errno = false,
    },
    { /* fwrite. */
        .action = 0x0,
        .stub = stub_fwrite,
        .name = "fwrite",
        .set_errno = false,
    },
    { /* close. */
        .action = 0x0,
        .stub = stub_close,
        .name = "close",
        .set_errno = false,
    },
    { /* fclose. */
        .action = 0x0,
        .stub = stub_fclose,
        .name = "fclose",
        .set_errno = false,
    },
    { /* getenv. */
        .action = 0x0,
        .stub = stub_getenv,
        .name = "getenv",
        .set_errno = false,
    },
    { /* getpid. */
        .action = 0x0,
        .stub = stub_getpid,
        .name = "getpid",
        .set_errno = false,
    },
    { /* time. */
        .action = 0x0,
        .stub = stub_time,
        .name = "time",
        .set_errno = false,
    },
    { /* rand. */
        .action = 0x0,
        .stub = stub_rand,
        .name = "rand",
        .set_errno = false,
    },
#ifdef TAPI_WINDOWS
    { /* strcpy_s. */
        .action = 0x0,
        .stub = stub_strcpy_s,
        .name = "strcpy_s",
        .set_errno = false,
    },
    { /* strncpy_s. */
        .action = 0x0,
        .stub = stub_strncpy_s,
        .name = "strncpy_s",
        .set_errno = false,
    },
    { /* memcpy_s. */
        .action = 0x0,
        .stub = stub_memcpy_s,
        .name = "memcpy_s",
        .set_errno = false,
    },
    { /* memmove_s. */
        .action = 0x0,
        .stub = stub_memmove_s,
        .name = "memmove_s",
        .set_errno = false,
    },
    { /* printf_s. */
        .action = 0x0,
        .stub = stub_printf_s,
        .name = "printf_s",
        .set_errno = false,
    },
    { /* sprintf_s. */
        .action = 0x0,
        .stub = stub_sprintf_s,
        .name = "sprintf_s",
        .set_errno = false,
    },
    { /* _snprintf_s. */
        .action = 0x0,
        .stub = stub_snprintf_s,
        .name = "_snprintf_s",
        .set_errno = false,
    },
    { /* vsprintf_s. */
        .action = 0x0,
        .stub = stub_vsprintf_s,
        .name = "vsprintf_s",
        .set_errno = false,
    },
    { /* _sopen_s. */
        .action = 0x0,
        .stub = stub_sopen_s,
        .name = "_sopen_s",
        .set_errno = false,
    },
    { /* fopen_s. */
        .action = 0x0,
        .stub = stub_fopen_s,
        .name = "fopen_s",
        .set_errno = false,
    },
    { /* freopen_s. */
        .action = 0x0,
        .stub = stub_freopen_s,
        .name = "freopen_s",
        .set_errno = false,
    },
    { /* _read. */
        .action = 0x0,
        .stub = stub__read,
        .name = "_read",
        .set_errno = false,
    },
    { /* fread_s. */
        .action = 0x0,
        .stub = stub_fread_s,
        .name = "fread_s",
        .set_errno = false,
    },
    { /* _write. */
        .action = 0x0,
        .stub = stub__write,
        .name = "_write",
        .set_errno = false,
    },
    { /* _close. */
        .action = 0x0,
        .stub = stub__close,
        .name = "_close",
        .set_errno = false,
    },
    { /* rand_s. */
        .action = 0x0,
        .stub = stub_rand_s,
        .name = "rand_s",
        .set_errno = false,
    },
#endif
};

/** @brief malloc autostub used by tapi. */
void*
stub_malloc(size_t size) {
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
stub_calloc(size_t nmemb, size_t size) {
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
stub_free(void* ptr) {
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
stub_realloc(void* ptr, size_t size) {
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

/** @brief strlen autostub used by tapi. */
size_t
stub_strlen(const char* s) {
    tapi_autostub_t autostub = autostub_table[4u]; /* get the strlen autostub. */
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, s);
        if (result == E_TAPI_ACTION_RESULT_FAIL) return 0x0;
    }
    /* proceed as per usual. */
    return strlen(s);
};

/** @brief strcmp autostub used by tapi. */
int
stub_strcmp(const char* s1, const char* s2) {
    tapi_autostub_t autostub = autostub_table[5u]; /* get the strcmp autostub. */
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, s1, s2);
        if (result == E_TAPI_ACTION_RESULT_FAIL) return 0x0;
    }
    /* proceed as per usual. */
    return strcmp(s1, s2);
};

/** @brief strncmp autostub used by tapi. */
int
stub_strncmp(const char* s1, const char* s2, size_t n) {
    tapi_autostub_t autostub = autostub_table[6u]; /* get the strncmp autostub. */
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, s1, s2, n);
        if (result == E_TAPI_ACTION_RESULT_FAIL) return 0x0;
    }
    /* proceed as per usual. */
    return strncmp(s1, s2, n);
};

/** @brief strcpy autostub used by tapi. */
char*
stub_strcpy(const char* s1, const char* s2) {
    tapi_autostub_t autostub = autostub_table[7u]; /* get the strcpy autostub. */
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, s1, s2);
        if (result == E_TAPI_ACTION_RESULT_FAIL) return 0x0;
    }
    /* proceed as per usual. */
    return strcpy(s1, s2);
};

/** @brief strncpy autostub used by tapi. */
char*
stub_strncpy(const char* s1, const char* s2, size_t n) {
    tapi_autostub_t autostub = autostub_table[8u]; /* get the strncpy autostub. */
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, s1, s2, n);
        if (result == E_TAPI_ACTION_RESULT_FAIL) return 0x0;
    }
    /* proceed as per usual. */
    return strncpy(s1, s2, n);
};

/** @brief memcpy autostub used by tapi. */
void*
stub_memcpy(void* dest, const void* src, size_t n)  {
    tapi_autostub_t autostub = autostub_table[9u]; /* get the memcpy autostub. */
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, dest, src, n);
        if (result == E_TAPI_ACTION_RESULT_FAIL) return 0x0;
    }
    /* proceed as per usual. */
    return memcpy(dest, src, n);
};

/** @brief memmove autostub used by tapi. */
void*
stub_memmove(void* dest, const void* src, size_t n) {
    tapi_autostub_t autostub = autostub_table[10u]; /* get the memmove autostub. */
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, dest, src, n);
        if (result == E_TAPI_ACTION_RESULT_FAIL) return 0x0;
    }
    /* proceed as per usual. */
    return memmove(dest, src, n);
};

/** @brief memset autostub used by tapi. */
void*
stub_memset(void* s, int c, size_t n) {
    tapi_autostub_t autostub = autostub_table[11u]; /* get the memset autostub. */
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, s, c, n);
        if (result == E_TAPI_ACTION_RESULT_FAIL) return 0x0;
    }
    /* proceed as per usual. */
    return memset(s, c, n);
};

/** @brief printf autostub used by tapi. */
int
stub_printf(const char* format, ...) {
    tapi_autostub_t autostub = autostub_table[11u]; /* get the printf autostub. */
    va_list list;
    va_start(list, format);
    if (autostub.action != 0x0) {
        /* if there exists an action, we call it and from there check the result. */
        e_tapi_action_result_t result = autostub.action(0x0, );
        if (result == E_TAPI_ACTION_RESULT_FAIL) return 0x0;
    }
    /* proceed as per usual. */
    int retval = vprintf(format, list);
    va_end(list);
    return retval;
};

/** @brief fprintf autostub used by tapi. */
int
stub_fprintf(FILE* stream, const char* format, ...) {

};

/** @brief sprintf autostub used by tapi. */
int
stub_sprintf(char* str, const char* format, ...) {

};

/** @brief snprintf autostub used by tapi. */
int
stub_snprintf(char* str, size_t size, const char* format, ...) {

};

/** @brief vsprintf autostub used by tapi. */
int
stub_vsprintf(char* str, const char* format, va_list ap) {

};

/** @brief open autostub used by tapi. */
int
stub_open(const char* pathname, int flags, ...) {

};

/** @brief fopen autostub used by tapi. */
FILE*
stub_fopen(const char* filename, const char* mode) {

};

/** @brief freopen autostub used by tapi. */
FILE*
stub_freopen(const char* filename, const char* mode, FILE* stream) {

};

/** @brief read autostub used by tapi. */
ssize_t
stub_read(int fd, const void* buf, size_t count) {

};

/** @brief fread autostub used by tapi. */
size_t
stub_fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {

};

/** @brief write autostub used by tapi. */
ssize_t
stub_write(int fd, const void* buf, size_t count) {

};

/** @brief fwrite autostub used by tapi. */
size_t
stub_fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream)  {

};

/** @brief close autostub used by tapi. */
int
stub_close(int fd) {

};

/** @brief close autostub used by tapi. */
int
stub_fclose(FILE* stream) {

};

/** @brief getenv autostub used by tapi. */
char*
stub_getenv(const char* name) {

};

/** @brief getpid autostub used by tapi. */
pid_t
stub_getpid(void) {

};

/** @brief time autostub used by tapi. */
time_t
stub_time(time_t* tloc) {

};

/** @brief rand autostub used by tapi. */
int
stub_rand(void) {

};

#endif
/*! ----------------------==---------------------- !*/

/**
 * @brief find an autostub within the internal autostub table.
 *
 * @param name the name of the possible autostub method.
 * @return a pointer to an autostub structure, or 0x0 o.w.
 */
tapi_autostub_t*
find_auto(const char* name) {
    for (size_t i = 0u; i < sizeof(autostub_table) / sizeof(tapi_autostub_t); i++) {
        if (!strcmp(name, autostub_table[i].name))
            return &autostub_table[i];
    }
    return 0x0;
}