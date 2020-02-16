/** @file ESTFSystemAssert.h
 *  @brief Macros that pass test case results to the system assert facility.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 */

#ifndef ESTF_SYSTEM_ASSERT_H
#define ESTF_SYSTEM_ASSERT_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

/** Evaluate an expression and record the result in a result collector.  The
 *  expression itself will be used as the decription of the test record.
 *
 *  @param collector The result collector to modify
 *  @param expr The expression to test
 *  @ingroup test
 */
#define ESTF_ASSERT( collector, expr ) \
    if ( collector ) ESTF_NATIVE_ASSERT( expr );

/** Add a success result to a result collector.
 *
 *  @param collector The result collector to modify
 *  @param description The description of the success result.
 *  @ingroup test
 */
#define ESTF_SUCCESS( collector, description )  \
    if ( collector );

/** Add a failure result to a result collector.
 *
 *  @param collector The result collector to modify
 *  @param description The description of the failure result.
 *  @ingroup test
 */
#define ESTF_FAILURE( collector, description ) \
    if ( collector ) ESTF_NATIVE_ASSERT( (void *) "FAILURE: " == (void *) description )

/** Add an error result to a result collector.
 *
 *  @param collector The result collector to modify
 *  @param description The description of the error condition.
 *  @ingroup test
 */
#define ESTF_ERROR( collector, description ) \
    if ( collector ) ESTF_NATIVE_ASSERT( (void *) "ERROR: " == (void *) description )

#endif
