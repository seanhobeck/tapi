/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-06-28
 */
/*! uses plt_init. */
#include "int/plt.h"

/** @brief library entry point. */
#ifdef __gnu_linux__
__attribute__((constructor))
#endif
void lt_entry() {
    plt_init();
}

/** @brief library exit point. */
#ifdef __gnu_linux__
__attribute__((destructor))
#endif
void lt_exit() {

}
/** \endcond */