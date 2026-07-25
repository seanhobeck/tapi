/**
 * @author Sean Hobeck
 * @date 2026-07-21
 */
#ifndef LNK_H
#define LNK_H

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

/** @brief clean up the internal plt_table. */
void
lnk_cleanup(void);
#endif /* LNK_H */