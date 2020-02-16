/** @file ESFComparator.h
 *  @brief ESFComparators are used by collections and sorting routines to
 *      order sets of keys.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_COMPARATOR_H
#define ESF_COMPARATOR_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

/** @defgroup collection Collections
 */

/** ESFComparators are used by collections and sorting routines to order sets
 *  of keys.
 *
 *  @ingroup collection
 */
class ESFComparator {
 public:
  /** Default constructor.
   */
  ESFComparator();

  /** Default destructor.
   */
  virtual ~ESFComparator();

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

#endif /* ! ESF_COMPARATOR_H */
