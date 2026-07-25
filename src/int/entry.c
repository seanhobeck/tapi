/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-07-09
 */
/*! uses plt_init. */
#include "int/lnk.h"

/** @brief library entry point. */
#ifdef __gnu_linux__
__attribute__((constructor))
#endif
void lt_entry() {
    lnk_init();
}

/** @brief library exit point. */
#ifdef __gnu_linux__
__attribute__((destructor))
#endif
void lt_exit() {
    lnk_cleanup();
}
/** \endcond */