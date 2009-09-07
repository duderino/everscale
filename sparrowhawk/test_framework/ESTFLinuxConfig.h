/** @file ESTFLinuxConfig.h
 *  @brief A description of the Linux platform.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:19 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESTF_LINUX_CONFIG_H
#define ESTF_LINUX_CONFIG_H

#include <assert.h>

#define ESTF_NATIVE_ASSERT assert

#define HAVE_SYS_TIME_H
#define HAVE_STRUCT_TIMEVAL
#define HAVE_GETTIMEOFDAY

#define HAVE_STDLIB_H
#define HAVE_RAND_R
#define HAVE_DECL_RAND_MAX

#define HAVE_PTHREAD_H
#define HAVE_PTHREAD_T
#define HAVE_PTHREAD_CREATE
#define HAVE_PTHREAD_JOIN
#define HAVE_PTHREAD_SELF
#define HAVE_PTHREAD_YIELD

#define HAVE_TIME_H
#define HAVE_TIMESPEC_T
#define HAVE_NANOSLEEP

#endif /* ! HAF_DARWIN_CONFIG_H */
