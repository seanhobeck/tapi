/**
 * @author Sean Hobeck
 * @date 2026-02-21
 */
#ifndef PATCH_H
#define PATCH_H

/*! @uses det_call_t. */
#include "det.h"

/**
 * @brief binary patch a call to a target using
 *
 * @param call the call structure info representing the call to be patched.
 * @param new_target the new target address to set the new call to.
 * @return 1 if successful, and 0 o.w.
 */
int32_t
patch_call_target(const det_call_t* call, const void* new_target);
#endif /* PATCH_H */