/**
 * @author Sean Hobeck
 * @date 2026-09-07
 */
#ifndef AUTO_H
#define AUTO_H

/*! uses tapi_autostub_t. */
#include <tapi/mock.h>

/**
 * @brief find an autostub within the internal autostub table.
 *
 * @param name the name of the possible autostub method.
 * @return a pointer to an autostub structure, or 0x0 o.w.
 */
tapi_autostub_t*
find_auto(const char* name);
#endif /* AUTO_H */
