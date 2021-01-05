#ifndef ESB_STRING_H
#define ESB_STRING_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ESB {

/**
 * Calculate a one way hash code for a NULL=terminated string.
 *
 * @param str The string to hash
 * @return a hash code for the string
 * @ingroup util
 */
extern ESB::UInt32 StringHash(const char *str);

}  // namespace ESB

#endif
