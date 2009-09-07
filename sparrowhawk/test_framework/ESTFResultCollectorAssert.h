/** @file ESTFResultCollectorAssert.h
 *  @brief Macros that record test case results in a ESTFResultCollector object.
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

#ifndef ESTF_RESULT_COLLECTOR_ASSERT_H
#define ESTF_RESULT_COLLECTOR_ASSERT_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

#ifndef ESTF_RESULT_COLECTOR_H
#include <ESTFResultCollector.h>
#endif

/** Evaluate an expression and record the result in a result collector.  The
 *  expression itself will be used as the decription of the test record.
 *
 *  @param collector The result collector to modify
 *  @param expr The expression to test
 *  @ingroup test
 */
#define ESTF_ASSERT( collector, expr ) \
    if ( ( expr ) ) \
        collector->addSuccess( #expr, __FILE__, __LINE__ ); \
    else \
        collector->addFailure( #expr, __FILE__, __LINE__ );

/** Add a success result to a result collector.
 *
 *  @param collector The result collector to modify
 *  @param description The description of the success result.
 *  @ingroup test
 */
#define ESTF_SUCCESS( collector, description ) \
    collector->addSuccess( description, __FILE__, __LINE__ );

/** Add a failure result to a result collector.
 *
 *  @param collector The result collector to modify
 *  @param description The description of the failure result.
 *  @ingroup test
 */
#define ESTF_FAILURE( collector, description ) \
    collector->addFailure( description, __FILE__, __LINE__ );

/** Add an error result to a result collector.
 *
 *  @param collector The result collector to modify
 *  @param description The description of the error condition.
 *  @ingroup test
 */
#define ESTF_ERROR( collector, description ) \
    collector->addError( description, __FILE__, __LINE__ );

#endif
