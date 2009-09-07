/** @file ESFAssert.h
 *  @brief A collection of assertion macros
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_ASSERT_H
#define ESF_ASSERT_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#ifdef HAVE_DECL_ASSERT
#define ESF_ASSERT assert
#else
#error "Platform requires an assert macro"
#endif

#endif /* ! ESF_ASSERT_H */
