/**
 * @author Sean Hobeck
 * @date 2026-09-05
 */
#ifndef INTT_H
#define INTT_H
/*! uses platform-specific macros. */
#include <tapi/platform.h>

/* internal specifier. */
#define internal static

/** enum for different internal tools results, quick eval. */
typedef enum {
    E_INTT_RESULT_FAILURE = 0x0,
    E_INTT_RESULT_SUCCESS = 0x1,
} e_intt_result_t;

/* quick macro. */
#define e_intt_passed(result) (result == E_INTT_RESULT_SUCCESS)

/*! uses uintptr_t. */
#include <stdint.h>

/*! uses sysconf, _SC_PAGE_SIZE. */
#ifdef TAPI_WINDOWS
#include <windows.h>

/*! uses size_t. */
#include <stddef.h>
#define UNUSED
#else
#include <unistd.h>
#define UNUSED __attribute__((unused))
#endif

/** @return page size on the given architecture, winapi and posix. */
UNUSED static size_t get_page_size() {
#ifndef TAPI_WINDOWS
    static size_t page_size = 0u;
    long value = sysconf(_SC_PAGESIZE);
    if (page_size == 0u) {
        if (value > 0) page_size = (size_t)value;
        else page_size = 4096u;
    }
    return page_size;
#else
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (size_t) si.dwPageSize;
#endif
}

/**
 * @brief align a page down and cast to an address for usage.
 *
 * @param page the page to be aligned down.
 * @return an address aligned down to the nearest page.
 */
UNUSED static void*
page_align_down(void* page, size_t size) {
    return (void*)((uintptr_t)page & ~(size - 1));
}
#endif /* INTT_H */