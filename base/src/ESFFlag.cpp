/** @file ESFFlag.cpp
 *  @brief An atomic flag
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:08 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_FLAG_H
#include <ESFFlag.h>
#endif

ESFFlag::ESFFlag(bool value) :
    _value(value) {
}

ESFFlag::~ESFFlag() {
}

