/**
 * @author Sean Hobeck
 * @date 2026-07-07
 */
#ifndef PLT_H
#define PLT_H

/** @brief ... */
typedef struct {
    char* name; /* name of the plt stub. */
    void* address; /* address of the stub. */
} pltr_asc_t;

/** @brief fill the hashmap corresponding a name of a procedure to its stub within the plt. */
void
plt_init(void);

/**
 * @brief resolve a library/system call's plt stub address. this is done via searching through
 *  the processes elf/peb/mach-o headers to read through a generated plt table.
 *
 * @param name the name of the given library/system call.
 * @return an address corresponding to the plt stub function, or 0x0 o.w.
 */
void*
plt_resolve(const char* name);
#endif /* PLT_H */