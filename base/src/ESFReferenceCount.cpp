/** @file ESFReferenceCount.cpp
 *  @brief Any class that extends ESFReferenceCount can be used with
 *      ESFSmartPointer.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:08 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_REFERENCE_COUNT_H
#include <ESFReferenceCount.h>
#endif

ESFReferenceCount::ESFReferenceCount() : _refCount() {
  //
  // Important: we are purposely not initializing the _allocator attribute
  // because that would overwrite the pointer assigned by operator new.
  //
}

ESFReferenceCount::~ESFReferenceCount() { ESF_ASSERT(0 == _refCount.get()); }
