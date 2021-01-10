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

}  // namespace ESB
