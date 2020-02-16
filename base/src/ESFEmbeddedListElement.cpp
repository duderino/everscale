/** @file ESFEmbeddedListElement.cpp
 *  @brief An element that can be stored in a ESFEmbeddedList
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

#ifndef ESF_EMBEDDED_LIST_ELEMENT_H
#include <ESFEmbeddedListElement.h>
#endif

ESFEmbeddedListElement::ESFEmbeddedListElement() : _next(0), _previous(0) {}

ESFEmbeddedListElement::~ESFEmbeddedListElement() {}
