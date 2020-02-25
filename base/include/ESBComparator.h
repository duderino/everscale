#ifndef ESB_COMPARATOR_H
#define ESB_COMPARATOR_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

namespace ESB {

/** @defgroup collection Collections
 */

/** Comparators are used by collections and sorting routines to order sets
 *  of keys.
 *
 *  @ingroup collection
 */
class Comparator {
 public:
  /** Default constructor.
   */
  Comparator();

  /** Default destructor.
   */
  virtual ~Comparator();

  /** Compare two locations.
   *
   *  @param first The first location to compare.
   *  @param second The second location to compare.
   *  @return 0 if both locations are equal, a negative number if the first
   *      location is less than the second, or a positive number if the first
   *      location is greater than the second.
   */
  virtual int compare(const void *first, const void *second) const = 0;
};

}  // namespace ESB

#endif
