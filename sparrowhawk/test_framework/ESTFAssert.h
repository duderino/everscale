/**    @file ESTFAssert.h
 *    @brief Utility macros for recording test results
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:19 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESTF_ASSERT_H
#define ESTF_ASSERT_H

#ifdef ESTF_USE_RESULT_COLLECTOR

#ifndef ESTF_RESULT_COLLECTOR_ASSERT_H
#include <ESTFResultCollectorAssert.h>
#endif

#else

#ifndef ESTF_SYSTEM_ASSERT_H
#include <ESTFSystemAssert.h>
#endif

#endif

#endif
