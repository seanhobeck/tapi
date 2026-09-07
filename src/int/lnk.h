/**
 * @author Sean Hobeck
 * @date 2026-09-05
 */
#ifndef LNK_H
#define LNK_H

/*! uses platform-specific macros. */
#include <tapi/platform.h>

/** @brief fill the hashmap corresponding a name of a procedure to its stub within the plt/iat/got. */
void
lnk_init(void);

/**
 * @brief resolve a library/system call's plt stub address. this is done via searching through
 *  the processes elf/peb/mach-o headers to read through a generated plt table.
 *
 * @param name the name of the given library/system call.
 * @return an address corresponding to the plt stub function, or 0x0 o.w.
 */
void*
lnk_resolve(const char* name);

#ifdef TAPI_WINDOWS
/**
 * @brief quickly resolve a windows thunk.
 * 
 * @param address the address of the thunk.
 * @return the actual address of what is being called at the thunk.
 */
void*
lnk_qr_thunk(void* address);
#endif

/** @brief clean up the internal plt_table. */
void
lnk_cleanup(void);
#endif /* LNK_H */