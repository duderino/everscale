/** @file ESFError.h
 *  @brief An abstraction over system error codes and error reporting
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

#ifndef ESF_ERROR_H
#define ESF_ERROR_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

/** @defgroup error Error */

/** An error code used to describe the result of operations.
 *
 *  @ingroup error
 */
typedef int ESFError;

/** Get an ESFError describing the last system error that occurred for the
 *  calling thread.
 *
 *  @return The ESFError code.  If no error has occured, this will return
 *      ESF_SUCCESS.
 *  @ingroup error
 */
extern ESFError ESFGetLastError();

/** Convert a system integer error code to an ESFError code.
 *
 *  @param error The system error code to convert
 *  @return The corresponding ESFError code.
 *  @ingroup error
 */
extern ESFError ESFConvertError(int error);

/** Describe a ESFError error code.
 *
 *  @param error The error code to describe
 *  @param buffer The buffer where the textual description will be stored
 *  @param size The size of the buffer
 *  @ingroup error
 */
extern void ESFDescribeError(ESFError error, char *buffer, int size);

/** Operation was successful.
 *
 *  @ingroup error
 */
#define ESF_SUCCESS 0

/** An uncategorized error occurred
 *
 *  @ingroup error
 */
#define ESF_OTHER_ERROR (-1)

/** Function does not implement this operation
 *
 *  @ingroup error
 */
#define ESF_OPERATION_NOT_SUPPORTED (-2)

/** Operation encountered a NULL pointer where it shouldn't have
 *
 *  @ingroup error
 */
#define ESF_NULL_POINTER (-3)

/** Operation would have produced multiple keys with the same value
 *
 *  @ingroup error
 */
#define ESF_UNIQUENESS_VIOLATION (-4)

/** Operation passed an argument with an invalid value
 *
 *  @ingroup error
 */
#define ESF_INVALID_ARGUMENT (-5)

/** Operation failed to allocate necessary memory
 *
 *  @ingroup error
 */
#define ESF_OUT_OF_MEMORY (-6)

/** Operation performed on an object that has not been initialized or whose
 *  intialization failed.
 *
 *  @ingroup error
 */
#define ESF_NOT_INITIALIZED (-7)

/** Operation could not be completed now, but should be attempted again later.
 *
 *  @ingroup error
 */
#define ESF_AGAIN (-8)

/** Operation was interrupted before it could complete
 *
 *  @ingroup error
 */
#define ESF_INTR (-9)

/** Operation is still in progress
 *
 *  @ingroup error
 */
#define ESF_INPROGRESS (-10)

/** Operation timed out before it could complete
 *
 *  @ingroup error
 */
#define ESF_TIMEOUT (-11)

/** Operation was passed an argument that was not long enough
 *
 *  @ingroup error
 */
#define ESF_ARGUMENT_TOO_SHORT (-12)

/** Operation was successful, but result was partially or fully truncated
 *
 *  @ingroup error
 */
#define ESF_RESULT_TRUNCATED (-13)

/** Object is in a state where it cannot perform the operation
 *
 *  @ingroup error
 *  @see ESF_NOT_INITIALIZED
 */
#define ESF_INVALID_STATE (-14)

/** Object does not own the requested resource.
 *
 *  @ingroup error
 */
#define ESF_NOT_OWNER (-15)

/** Object has resources currently in use.
 *
 *  @ingroup error
 */
#define ESF_IN_USE (-16)

/** Operation cannot find the specified element
 *
 *  @ingroup error
 */
#define ESF_CANNOT_FIND (-17)

/** Operation passed an invalid iterator (an iterator with a isNull method
 *  that returns true).
 *
 *  @ingroup error
 */
#define ESF_INVALID_ITERATOR (-18)

/** Operation cannot be completed without exceeding a hard limit.
 *
 *  @ingroup error
 */
#define ESF_OVERFLOW (-19)

/** Operation cannot perform requested conversion.
 *
 *  @ingroup error
 */
#define ESF_CANNOT_CONVERT (-20)

/** Operation was passed an argument with an illegal encoding.
 *
 *  @ingroup error
 */
#define ESF_ILLEGAL_ENCODING (-21)

/** Operation was passed an unsupported character set or encoding.
 *
 *  @ingroup error
 */
#define ESF_UNSUPPORTED_CHARSET (-22)

/** Operation was passed an index outside of the bounds of an array.
 *
 *  @ingroup error
 */
#define ESF_OUT_OF_BOUNDS (-23)

/** Operation cannot be completed because shutdown has already been
 *  called.
 *
 * @ingroup error
 */
#define ESF_SHUTDOWN (-24)

#endif
