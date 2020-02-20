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
 *  @ingroup unit-test
 */
#define ESTF_ASSERT(collector, expr) \
  if (collector) assert(expr);

/** Add a success result to a result collector.
 *
 *  @param collector The result collector to modify
 *  @param description The description of the success result.
 *  @ingroup unit-test
 */
#define ESTF_SUCCESS(collector, description) \
  if (collector)                             \
    ;

/** Add a failure result to a result collector.
 *
 *  @param collector The result collector to modify
 *  @param description The description of the failure result.
 *  @ingroup unit-test
 */
#define ESTF_FAILURE(collector, description) \
  if (collector) assert((void *)"FAILURE: " == (void *)description)

/** Add an error result to a result collector.
 *
 *  @param collector The result collector to modify
 *  @param description The description of the error condition.
 *  @ingroup unit-test
 */
#define ESTF_ERROR(collector, description) \
  if (collector) assert((void *)"ERROR: " == (void *)description)

#endif
