/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-07-25
 */
/*! uses lnk_init. */
#include "lnk.h"

/** @brief library entry point. */
#ifdef __gnu_linux__
__attribute__((constructor))
void lt_entry() {
#else
/*! uses BOOL, WINAPI(__stdcall), HINSTANCE, etc... */
#include <windows.h>
BOOL WINAPI DllMain(HINSTANCE hInstanceDll, DWORD dwReason, LPVOID lpvReserved) {
#endif
    lnk_init();
#ifdef _WIN32
    return TRUE;
#endif
}

/** @brief library exit point. */
#ifdef __gnu_linux__
__attribute__((destructor))
#endif
void lt_exit() {
    lnk_cleanup();
}
/** \endcond */