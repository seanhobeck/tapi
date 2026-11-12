/**
 * @author Sean Hobeck
 * @date 2026-11-12
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

#ifdef TAPI_LINUX
/*! uses O_CREAT, open, close, etc... */
#include <fcntl.h>
#endif

/*! uses internal. */
#include "def.h"

/*! -----------------autostubs------------------ !
 *
 * list of currently supported autostubs for POSIX (marked with 'p'), MSVC (marked with 'ms'),
 *  and linux/unix (marked with 'l/u') system/library calls:
 *  _____________________________________
 *  |system/libc    |msvc crt-equiv. api|
 *  =====================================
 *  |malloc(p)      |.                  |
 *  |calloc(p)      |.                  |
 *  |free(p)        |.                  |
 *  |realloc(p)     |.                  |
 *  |strlen(p)      |.                  |
 *  |strcmp(p)      |.                  |
 *  |strncmp(p)     |.                  |
 *  |strcpy(p)      |strcpy_s(ms)       |
 *  |strncpy(p)     |strncpy_s(ms)      |
 *  |memcpy(p)      |memcpy_s(ms)       |
 *  |memmove(p)     |memmove_s(ms)      |
 *  |memset(p)      |.                  |
 *  |printf(p)      |printf_s(ms)       |
 *  |fprintf(p)     |fprintf_s(ms)      |
 *  |sprintf(p)     |sprintf_s(ms)      |
 *  |snprintf(p)    |_snprintf_s(ms)    |
 *  |vsprintf(p)    |vsprintf_s(ms)     |
 *  |open(l/u)      |_sopen_s(ms)       |
 *  |fopen(p)       |fopen_s(ms)        |
 *  |freopen(p)     |freopen_s(ms)      |
 *  |read(l/u)      |_read(ms)          |
 *  |fread(p)       |fread_s(ms)        |
 *  |write(l/u)     |_write(ms)         |
 *  |fwrite(p)      |.                  |
 *  |close(l/u)     |_close(ms)         |
 *  |fclose(p)      |.                  |
 *  |getenv(l/u)    |n/a                |
 *  |getpid(l/u)    |n/a                |
 *  |time(p)        |.                  |
 *  |rand(p)        |rand_s(ms)         |
 *  =====================================
 */
#ifndef TAPI_MINIMAL

/** used next to parameters to be expanded within an argument. */
#define EXPAND(...) __VA_ARGS__
#define EXPAND_WITH_BLANK(...) 0x0, __VA_ARGS__

/** macro to automatically declare an autostub. */
#define AUTOSTUB_DECL(name, retval, args, params, fail, error) \
    retval stub_##name args;

/** macro to automatically define a regular autostub. */
#define AUTOSTUB_DEFN(name, retval, args, params, fail, error) \
    retval stub_##name args { \
        tapi_autostub_t* stub = &autostub_table[AUTO_##name]; \
        if (stub->condition && stub->condition(EXPAND_WITH_BLANK params) == E_TAPI_CONDITION_FAIL) { \
            if ((error) != 0 && stub->set_errno) errno = (error); \
            return (fail); \
        } \
        return name params; \
    };

/** macro to automatically define a enum for an autostub with a given name. */
#define AUTOSTUB_ENUM(name, retval, args, params, fail, error) AUTO_##name,

/** macro to automatically define an entry within our autostub table. */
#define AUTOSTUB_ENTRY(nm, retval, args, params, fail, error) \
    [AUTO_##nm] = { .condition = 0x0, .stub = stub_##nm, .name = #nm, .set_errno = false },

/** macro for each of the posix-compliant autostubs. */
#define POSIX_AUTOSTUBS(M) \
    M(malloc, void*, (size_t size), (size), 0x0, ENOMEM) \
    M(calloc, void*, (size_t nmemb, size_t size), (nmemb, size), 0x0, ENOMEM) \
    M(realloc, void*, (void* ptr, size_t size), (ptr, size), 0x0, ENOMEM) \
    M(strlen, size_t, (char* s), (s), 0x0, 0) \
    M(strcmp, int, (char* s1, char* s2), (s1, s2), 0x0, 0) \
    M(strncmp, int, (char* s1, char* s2, size_t n), (s1, s2, n), 0x0, 0) \
    M(strcpy, char*, (char* dest, char* src), (dest, src), 0x0, 0) \
    M(strncpy, char*, (char* dest, char* src, size_t n), (dest, src, n), 0x0, 0) \
    M(memcpy, void*, (void* dest, void* src, size_t n), (dest, src, n), 0x0, 0) \
    M(memmove, void*, (void* dest, void* src, size_t n), (dest, src, n), 0x0, 0) \
    M(memset, void*, (void* s, int c, size_t n), (s, c, n), 0x0, 0) \
    M(fopen, FILE*, (char* filename, char* mode), (filename, mode), 0x0, 0) \
    M(freopen, FILE*, ( char* filename, char* mode, FILE* st), (filename, mode, st), 0x0, 0) \
    M(fread, size_t, (void* ptr, size_t size, size_t nmemb, FILE* st), (ptr, size, nmemb, st), 0x0, 0) \
    M(fwrite, size_t, (void* ptr, size_t size, size_t nmemb, FILE* st), (ptr, size, nmemb, st), 0x0, 0) \
    M(fclose, int, (FILE* stream), (stream), EOF, 0) \
    M(time, time_t, (time_t* tloc), (tloc), 0x0, 0)

/** macro for each of the extra posix-compliant autostubs (ones that need custom defn). */
#define EXTRA_POSIX_AUTOSTUBS(M) \
    M(rand, 0, 0, 0, 0, 0) \
    M(free, 0, 0, 0, 0, 0) \
    M(printf, 0, 0, 0, 0, 0) \
    M(fprintf, 0, 0, 0, 0, 0) \
    M(sprintf, 0, 0, 0, 0, 0) \
    M(snprintf, 0, 0, 0, 0, 0) \
    M(vsprintf, 0, 0, 0, 0, 0)

/** macro for each of the unix/linux autostubs. */
#define NIX_AUTOSTUBS(M) \
    M(read, ssize_t, (int fd, void* buf, size_t count), (fd, buf, count), 0x0, 0) \
    M(write, ssize_t, (int fd, void* buf, size_t count), (fd, buf, count), 0x0, 0) \
    M(close, int, (int fd), (fd), 0x0, 0) \
    M(getenv, char*, ( char* name), (name), 0x0, 0)

/** macro for each of the extra nix-compliant autostubs (ones that need custom defn). */
#define EXTRA_NIX_AUTOSTUBS(M) \
    M(open, 0, 0, 0, 0, 0) \

/** macro for each of the windows autostubs. */
#define WIN_AUTOSTUBS(M) \
    M(strcpy_s, errno_t, (char* dst, rsize_t size, char* src), (dst, size, src), EINVAL, 0) \
    M(strncpy_s, errno_t, (char* dst, rsize_t size, char* src, size_t n), (dst, size, src, n), EINVAL, 0) \
    M(memcpy_s, errno_t, (void* dst, size_t size, void* src, size_t n), (dst, size, src, n), EINVAL, 0) \
    M(memmove_s, errno_t, (void* dst, size_t size, void* src, size_t n), (dst, size, src, n), EINVAL, 0) \
    M(_sopen_s, errno_t, (int* fd, char* file, int flags, int share, int mode), (fd, file, flags, share, mode), EINVAL, 0) \
    M(fopen_s, errno_t, (FILE** stream, char* file, char* mode), (stream, file, mode), EINVAL, 0) \
    M(freopen_s, errno_t, (FILE** stream, char* file, char* mode, FILE* old), (stream, file, mode, old), EINVAL, 0) \
    M(_read, int, (int fd, void* buf, unsigned int n), (fd, buf, n), EINVAL, 0) \
    M(fread_s, size_t, (void* buf, size_t buf_size, size_t size, size_t n, FILE* stream), (buf, buf_size, size, n, stream), (size_t)EINVAL, 0) \
    M(_write, int, (int fd, void* buf, unsigned int n), (fd, buf, n), EBADF, 0) \
    M(_close, int, (int fd), (fd), -1, 0) \
    M(rand_s, errno_t, (unsigned int* out), (out), EINVAL, 0)

/** macro for each of the extra windows autostubs (ones that need custom defn). */
#define EXTRA_WIN_AUTOSTUBS(M) \
    M(printf_s, 0, 0, 0, 0, 0) \
    M(fprintf_s, 0, 0, 0, 0, 0) \
    M(sprintf_s, 0, 0, 0, 0, 0) \
    M(snprintf_s, 0, 0, 0, 0, 0) \
    M(vsprintf_s, 0, 0, 0, 0, 0) \

/** autostub anon. enum definition. */
typedef enum {
    POSIX_AUTOSTUBS(AUTOSTUB_ENUM)
    EXTRA_POSIX_AUTOSTUBS(AUTOSTUB_ENUM)
#if defined(TAPI_LINUX) || defined(TAPI_UNIX)
    NIX_AUTOSTUBS(AUTOSTUB_ENUM)
    EXTRA_NIX_AUTOSTUBS(AUTOSTUB_ENUM)
#elif defined(TAPI_WINDOWS)
    WIN_AUTOSTUBS(AUTOSTUB_ENUM)
#endif
} e_autostub_id_t;

/** define each of the declarations for each of the autostubs. */
POSIX_AUTOSTUBS(AUTOSTUB_DECL)
#if defined(TAPI_LINUX) || defined(TAPI_UNIX)
NIX_AUTOSTUBS(AUTOSTUB_DECL)
#elif defined(TAPI_WINDOWS)
WIN_AUTOSTUBS(AUTOSTUB_DECL)
#endif

/** declare each of the methods that were not covered in the macros above. */
int stub_rand();
void stub_free(void* ptr);
int stub_printf(const char* format, ...);
int stub_fprintf(FILE* stream, const char* format, ...);
int stub_sprintf(char* dst, const char* format, ...);
int stub_snprintf(char* dst, size_t size, const char* format, ...);
int stub_vsprintf(char* dst, const char* format, va_list ap);
#if defined(TAPI_LINUX) || defined(TAPI_UNIX)
int stub_open(const char* path, int flags, ...);
#else
int stub_printf_s(const char* format, ...);
int stub_fprintf_s(FILE* stream, const char* format, ...);
int stub_sprintf_s(char* dst, size_t size, const char* format, ...);
int stub_snprintf_s(char* dst, size_t size, size_t count, const char* format, ...);
int stub_vsprintf_s(char8 dst, size_t size, const char* format, va_list ap);
#endif

/** the internal autostub table used for automocks. */
internal tapi_autostub_t autostub_table[43u] = {
    POSIX_AUTOSTUBS(AUTOSTUB_ENTRY)
    EXTRA_POSIX_AUTOSTUBS(AUTOSTUB_ENTRY)
    #if defined(TAPI_LINUX) || defined(TAPI_UNIX)
    NIX_AUTOSTUBS(AUTOSTUB_ENTRY)
    EXTRA_NIX_AUTOSTUBS(AUTOSTUB_ENTRY)
    #elif defined(TAPI_WINDOWS)
    WIN_AUTOSTUBS(AUTOSTUB_ENTRY)
    EXTRA_WIN_AUTOSTUBS(AUTOSTUB_ENTRY)
    #endif
};

/** define the methods for the non-special autostubs. */
POSIX_AUTOSTUBS(AUTOSTUB_DEFN)
int
stub_rand(void) {
    tapi_autostub_t autostub = autostub_table[AUTO_rand]; /* get the rand autostub. */
    if (autostub.condition != 0x0) {
        e_tapi_condition_result_t result = autostub.condition(0x0);
        if (result == E_TAPI_CONDITION_FAIL) return 0x0;
    }
    return rand();
};
void
stub_free(void* ptr) {
    tapi_autostub_t autostub = autostub_table[AUTO_free]; /* get the free autostub. */
    if (autostub.condition != 0x0) {
        e_tapi_condition_result_t result = autostub.condition(0x0, ptr);
        if (result == E_TAPI_CONDITION_FAIL) {
            return;
        }
    }
    free(ptr);
};
int
stub_printf(const char* format, ...) {
    tapi_autostub_t autostub = autostub_table[AUTO_printf]; /* get the printf autostub. */
    va_list list;
    va_start(list, format);
    if (autostub.condition != 0x0) {
        e_tapi_condition_result_t result = autostub.condition(0x0, format, list);
        if (result == E_TAPI_CONDITION_FAIL) {
            va_end(list);
            return 0x0;
        }
    }
    int retval = vprintf(format, list);
    va_end(list);
    return retval;
};
int
stub_fprintf(FILE* stream, const char* format, ...) {
    tapi_autostub_t autostub = autostub_table[AUTO_fprintf]; /* get the fprintf autostub. */
    va_list list;
    va_start(list, format);
    if (autostub.condition != 0x0) {
        e_tapi_condition_result_t result = autostub.condition(0x0, stream, format, list);
        if (result == E_TAPI_CONDITION_FAIL) {
            va_end(list);
            return 0x0;
        }
    }
    int retval = vfprintf(stream, format, list);
    va_end(list);
    return retval;
};
int
stub_sprintf(char* str, const char* format, ...) {
    tapi_autostub_t autostub = autostub_table[AUTO_sprintf]; /* get the sprintf autostub. */
    va_list list;
    va_start(list, format);
    if (autostub.condition != 0x0) {
        e_tapi_condition_result_t result = autostub.condition(0x0, str, format, list);
        if (result == E_TAPI_CONDITION_FAIL) {
            va_end(list);
            return 0x0;
        }
    }
    int retval = vsprintf(str, format, list);
    va_end(list);
    return retval;
};
int
stub_snprintf(char* str, size_t size, const char* format, ...) {
    tapi_autostub_t autostub = autostub_table[AUTO_snprintf]; /* get the snprintf autostub. */
    va_list list;
    va_start(list, format);
    if (autostub.condition != 0x0) {
        e_tapi_condition_result_t result = autostub.condition(0x0, str, size, format, list);
        if (result == E_TAPI_CONDITION_FAIL) {
            va_end(list);
            return 0x0;
        }
    }
    int retval = vsnprintf(str, size, format, list);
    va_end(list);
    return retval;
};
int
stub_vsprintf(char* str, const char* format, va_list ap)  {
    tapi_autostub_t autostub = autostub_table[AUTO_vsprintf]; /* get the vsprintf autostub. */
    if (autostub.condition != 0x0) {
        e_tapi_condition_result_t result = autostub.condition(0x0, str, format, ap);
        if (result == E_TAPI_CONDITION_FAIL) return 0x0;
    }
    return vsprintf(str, format, ap);
};
#if defined(TAPI_LINUX) || defined(TAPI_UNIX)
NIX_AUTOSTUBS(AUTOSTUB_DEFN)
int
stub_open(const char* pathname, int flags, ...) {
    tapi_autostub_t autostub = autostub_table[AUTO_open]; /* get the open autostub. */

    /* get the mode (optional variadic param). */
    mode_t mode = 0x0;
    if (flags & O_CREAT) {
        va_list list;
        va_start(list, flags);
        mode = va_arg(list, mode_t);
        va_end(list);
    }
    /* if there exists a condition, we call it and from there check the result. */
    if (autostub.condition != 0x0) {
        e_tapi_condition_result_t result;
        if (mode != 0x0) result = autostub.condition(0x0, pathname, flags, mode);
        else result = autostub.condition(0x0, pathname, flags);
        if (result == E_TAPI_CONDITION_FAIL) return 0x0;
    }
    if (mode != 0x0) return open(pathname, flags, mode);
    return open(pathname, flags);
};
#else
WIN_AUTOSTUBS(AUTOSTUB_DEFN)
int
stub_printf_s(const char* format, ...) {
    tapi_autostub_t autostub = autostub_table[28u]; /* get the printf_s autostub. */
    va_list list;
    va_start(list, format);
    if (autostub.condition != 0x0) {
        e_tapi_condition_result_t result = autostub.condition(0x0, format, list);
        if (result == E_TAPI_CONDITION_FAIL) {
            va_end(list);
            return 0x0;
        }
    }
    int retval = vprintf_s(format, list);
    va_end(list);
    return retval;
};
int
stub_fprintf_s(FILE* stream, const char* format, ...)  {
    tapi_autostub_t autostub = autostub_table[29u]; /* get the fprintf_s autostub. */
    va_list list;
    va_start(list, format);
    if (autostub.condition != 0x0) {
        e_tapi_condition_result_t result = autostub.condition(0x0, stream, format, list);
        if (result == E_TAPI_CONDITION_FAIL) {
            va_end(list);
            return 0x0;
        }
    }
    int retval = vfprintf_s(stream, format, list);
    va_end(list);
    return retval;
};
int
stub_sprintf_s(char* dest, size_t dest_size, \
    const char* format, ...)  {
    tapi_autostub_t autostub = autostub_table[30u]; /* get the sprintf_s autostub. */
    va_list list;
    va_start(list, format);
    if (autostub.condition != 0x0) {
        e_tapi_condition_result_t result = autostub.condition(0x0, dest, dest_size, format, list);
        if (result == E_TAPI_CONDITION_FAIL) {
            va_end(list);
            return 0x0;
        }
    }
    int retval = vsprintf_s(dest, dest_size, format, list);
    va_end(list);
    return retval;
};
int
stub_snprintf_s(char* dest, size_t dest_size, \
    size_t count, const char* format, ...)  {
    tapi_autostub_t autostub = autostub_table[31u]; /* get the snprintf_s autostub. */
    va_list list;
    va_start(list, format);
    if (autostub.condition != 0x0) {
        e_tapi_condition_result_t result = autostub.condition(0x0, dest, dest_size, \
            count, format, list);
        if (result == E_TAPI_CONDITION_FAIL) {
            va_end(list);
            return 0x0;
        }
    }
    int retval = vsprintf_s(dest, dest_size, count, format, list);
    va_end(list);
    return retval;
};
int
stub_vsprintf_s(char* dest, size_t num_elems, \
    const char* format, va_list ap) {
    tapi_autostub_t autostub = autostub_table[32u]; /* get the vsprintf_s autostub. */
    if (autostub.condition != 0x0) {
        e_tapi_condition_result_t result = autostub.condition(0x0, dest, num_elems, \
            format, ap);
        if (result == E_TAPI_CONDITION_FAIL) {
            va_end(list);
            return 0x0;
        }
    }
    return vsprintf_s(dest, num_elems, format, ap);
};
#endif
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