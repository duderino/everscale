#ifndef ESB_STRING_H
#include <ESBString.h>
#endif

namespace ESB {

ESB::UInt32 StringHash(const char *str) {
  assert(str);
  if (!str) {
    return 0U;
  }

  // From Sedgewick's Algorithms in C, 4th ed, p.579
  ESB::UInt32 h = 0U, a = 31415, b = 27183, M = 7919;
  for (const char *v = str; *v; v++, a = a * b % (M - 1)) {
    h = (a * h + *v) % M;
  }
  return h;
}

static int ForwardMatch(const char *pattern, UInt32 patternLength, const char *str, UInt32 strLength,
                        bool *hasWildcard) {
  if (!hasWildcard) {
    return -1;
  }

  *hasWildcard = false;

  if (!pattern || !str) {
    return -1;
  }

  if (0 == patternLength || 0 == strLength) {
    return patternLength == strLength ? 0 : -1;
  }

  int patternIdx = 0;
  int strIdx = 0;
  int matchedChars = 0;

  while (true) {
    if ('*' == pattern[patternIdx]) {
      *hasWildcard = true;
      return matchedChars;
    }

    if (pattern[patternIdx] != str[strIdx]) {
      return -1;
    }

    if (strIdx >= strLength) {
      return patternIdx >= patternLength ? matchedChars : -1;
    }

    if (patternIdx >= patternLength) {
      return -1;
    }

    ++matchedChars;
    ++patternIdx;
    ++strIdx;
  }
}

static int ReverseMatch(const char *pattern, UInt32 patternLength, const char *str, UInt32 strLength) {
  if (!pattern || !str) {
    return -1;
  }

  if (0 == patternLength || 0 == strLength) {
    return patternLength == strLength ? 0 : -1;
  }

  int patternIdx = patternLength - 1;
  int strIdx = strLength - 1;
  int matchedChars = 0;

  while (true) {
    if ('*' == pattern[patternIdx]) {
      return matchedChars;
    }

    if (pattern[patternIdx] != str[strIdx]) {
      return -1;
    }

    if (0 == strIdx) {
      if (0 == patternIdx || '*' == pattern[patternIdx - 1]) {
        return matchedChars + 1;
      }
      return -1;
    }

    if (0 == patternIdx) {
      return -1;
    }

    ++matchedChars;
    --patternIdx;
    --strIdx;
  }
}

int StringWildcardMatch(const char *pattern, UInt32 patternLength, const char *str, UInt32 strLength) {
  bool hasWildcard = false;
  int forwardResult = ForwardMatch(pattern, patternLength, str, strLength, &hasWildcard);

  if (0 > forwardResult) {
    return -1;
  }

  if (!hasWildcard) {
    return strLength - forwardResult;
  }

  int reverseResult = ReverseMatch(pattern, patternLength, str, strLength);

#ifndef NDEBUG
  {
    int numWildcards = 0;
    for (int i = 0; i < patternLength; ++i) {
      if ('*' == pattern[patternLength]) {
        ++numWildcards;
      }
    }
    assert(numWildcards <= 1);
  };
#endif

  // add one for the '*' itself so the 'bar' == 'bar' exact match is more specific than 'bar*' ~= 'bar' or '*bar' ~=
  // 'bar'
  return 0 > reverseResult ? -1 : strLength - forwardResult - reverseResult + 1;
}

}  // namespace ESB
