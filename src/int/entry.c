/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-08-05
 */
/*! uses lnk_init. */
#include "lnk.h"

/** @brief library entry point. */
#ifdef __gnu_linux__
__attribute__((constructor))
void lt_entry() {
    lnk_init();
    return;
#elif defined(_WIN32)
/*! uses BOOL, WINAPI(__stdcall), HINSTANCE, etc... */
#include <windows.h>
BOOL WINAPI DllMain(HINSTANCE hInstanceDll, DWORD dwReason, LPVOID lpvReserved) {
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        lnk_init();
        break;
    case DLL_PROCESS_DETACH:
        lnk_cleanup();
        break;
    }
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