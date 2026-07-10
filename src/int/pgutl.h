/**
 * @author Sean Hobeck
 * @date 2026-07-09
 */
#ifndef PGUTL_H
#define PGUTL_H

/*! uses uintptr_t. */
#include <stdint.h>

/*! uses sysconf, _SC_PAGE_SIZE. */
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/** @return page size on the given architecture, winapi and posix. */
size_t
get_page_size() {
    static size_t page_size = 0u;
#ifndef _WIN32
    long value = sysconf(_SC_PAGESIZE);
    if (page_size == 0u) {
        if (value > 0) page_size = (size_t)value;
        else page_size = 4096u;
    }
#else
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    if (page_size == 0u) page_size = (size_t)si.dwPageSize;
#endif
    return page_size;
}

/**
 * @brief align a page down and cast to an address for usage.
 *
 * @param page the page to be aligned down.
 * @return an address aligned down to the nearest page.
 */
inline void*
page_align_down(void* page, size_t size) {
    return (void*)((uintptr_t)page & ~(size - 1));
}
#endif /* PGUTL_H */
