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

/**
 * Determine whether a string matches a wildcard ('*') pattern - at most one wildcard is supported.
 *
 * @param pattern The pattern to match, at most 1 wildcard is supported
 * @param str The string to match against the pattern.
 * @return -1 if no match, 0 if exact match, > 0 if wildcard match (the less specific the match, the higher the return
 * value)
 *
 */
extern int StringWildcardMatch(const char *pattern, UInt32 patternLength, const char *str, UInt32 strLength);

inline int StringWildcardMatch(const char *pattern, const char *str) {
  if (!pattern || !str) {
    return -1;
  }
  return StringWildcardMatch(pattern, strlen(pattern), str, strlen(str));
}

}  // namespace ESB

#endif
