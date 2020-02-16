/** @file ESFObject.h
 *  @brief An object
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

#ifndef ESF_OBJECT_H
#define ESF_OBJECT_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

/** An object
 *
 *  @ingroup collection
 */
class ESFObject {
 public:
  ESFObject(){};

  virtual ~ESFObject(){};
};

#endif /* ! ESF_OBJECT_H */
