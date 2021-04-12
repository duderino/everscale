#ifndef ESB_COMPARATOR_H
#define ESB_COMPARATOR_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
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
   *  @param key The key to lookup.
   *  @param value The value to compare to the key.
   *  @return 0 if the key is matches the value, if the key is less than the value, or a positive number if the key is
   * greater than the value.
   */
  virtual int compare(const void *key, const void *value) const = 0;
};

/** HashComparators add hashing to Comparators.
 *
 *  @ingroup collection
 */
class HashComparator : public Comparator {
 public:
  /** Default constructor.
   */
  HashComparator();

  /** Default destructor.
   */
  virtual ~HashComparator();

  /** Generate a hash code from a key.
   *
   *  @param key The key to hash
   *  @return the hash code
   */
  virtual UInt64 hash(const void *key) const = 0;
};

}  // namespace ESB

#endif
