/**
 * \cond
 * @author Sean Hobeck
 * @date 2026-06-07
 */
/** @brief library entry point. */
#ifdef __gnu_linux__
__attribute__((constructor))
#endif
void lt_entry() {

}

/** @brief library exit point. */
#ifdef __gnu_linux__
__attribute__((destructor))
#endif
void lt_exit() {

}
/** \endcond */