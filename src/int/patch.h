/**
 * @author Sean Hobeck
 * @date 2026-06-09
 */
#ifndef PATCH_H
#define PATCH_H

/*! @uses det_call_t. */
#include "det.h"

/*! @uses tapi_context_t. */
#include <tapi/tapi.h>

/**
 * @brief binary patch a call to a target using
 *
 * @param context the tapi context to be used.
 * @param call the call structure info representing the call to be patched.
 * @param new_target the new target address to set the new call to.
 * @return 1 if successful, and 0 o.w.
 */
int32_t
patch_call_target(tapi_context_t* context, const det_call_t* call, const void* new_target);
#endif /* PATCH_H */