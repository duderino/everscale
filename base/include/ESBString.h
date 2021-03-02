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
 * @param patternLength The length of the pattern
 * @param str The string to match against the pattern.
 * @param strLength The length of the pattern
 * @return -1 if no match, 0 if exact match, > 0 if wildcard match (the less specific the match, the higher the return
 * value)
 */
extern int StringWildcardMatch(const char *pattern, UInt32 patternLength, const char *str, UInt32 strLength);

/**
 * Determine whether a string matches a wildcard ('*') pattern - at most one wildcard is supported.
 *
 * @param pattern The pattern to match, at most 1 wildcard is supported.  Must be NULL terminated.
 * @param str The string to match against the pattern.  Must be NULL terminated.
 * @return -1 if no match, 0 if exact match, > 0 if wildcard match (the less specific the match, the higher the return
 * value)
 */
inline int StringWildcardMatch(const char *pattern, const char *str) {
  if (!pattern || !str) {
    return -1;
  }
  return StringWildcardMatch(pattern, strlen(pattern), str, strlen(str));
}

/**
 * Read a pointer from non-aligned memory
 *
 * @param buffer The non-aligned buffer to read from
 * @return The pointer
 */
extern void *ReadPointer(const unsigned char *buffer);

/**
 * Write a pointer to non-aligned memory
 *
 * @param buffer The non-aligned buffer to write to.  Must have room for pointer size bytes.
 * @param pointer The pointer to write
 */
extern void WritePointer(unsigned char *buffer, void *pointer);

}  // namespace ESB

#endif
