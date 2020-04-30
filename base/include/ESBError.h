#ifndef ESB_ERROR_H
#define ESB_ERROR_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

namespace ESB {

/** @defgroup error Error */

/** An error code used to describe the result of operations.
 *
 *  @ingroup error
 */
typedef int Error;

/** Get an Error describing the last system error that occurred for the
 *  calling thread.
 *
 *  @return The Error code.  If no error has occured, this will return
 *      ESB_SUCCESS.
 *  @ingroup error
 */
extern Error LastError();

/** Convert a system integer error code to an Error code.
 *
 *  @param error The system error code to convert
 *  @return The corresponding Error code.
 *  @ingroup error
 */
extern Error ConvertError(int error);

/** Describe a Error error code.
 *
 *  @param error The error code to describe
 *  @param buffer The buffer where the textual description will be stored
 *  @param size The size of the buffer
 *  @ingroup error
 */
extern void DescribeError(Error error, char *buffer, int size);

/** Operation was successful.
 *
 *  @ingroup error
 */
#define ESB_SUCCESS 0

/** An uncategorized error occurred
 *
 *  @ingroup error
 */
#define ESB_OTHER_ERROR (-1)

/** Function does not implement this operation
 *
 *  @ingroup error
 */
#define ESB_OPERATION_NOT_SUPPORTED (-2)

/** Operation encountered a NULL pointer where it shouldn't have
 *
 *  @ingroup error
 */
#define ESB_NULL_POINTER (-3)

/** Operation would have produced multiple keys with the same value
 *
 *  @ingroup error
 */
#define ESB_UNIQUENESS_VIOLATION (-4)

/** Operation passed an argument with an invalid value
 *
 *  @ingroup error
 */
#define ESB_INVALID_ARGUMENT (-5)

/** Operation failed to allocate necessary memory
 *
 *  @ingroup error
 */
#define ESB_OUT_OF_MEMORY (-6)

/** Operation performed on an object that has not been initialized or whose
 *  intialization failed.
 *
 *  @ingroup error
 */
#define ESB_NOT_INITIALIZED (-7)

/** Operation could not be completed now, but should be attempted again later.
 *
 *  @ingroup error
 */
#define ESB_AGAIN (-8)

/** Operation was interrupted before it could complete
 *
 *  @ingroup error
 */
#define ESB_INTR (-9)

/** Operation is still in progress
 *
 *  @ingroup error
 */
#define ESB_INPROGRESS (-10)

/** Operation timed out before it could complete
 *
 *  @ingroup error
 */
#define ESB_TIMEOUT (-11)

/** Operation was passed an argument that was not long enough
 *
 *  @ingroup error
 */
#define ESB_ARGUMENT_TOO_SHORT (-12)

/** Operation was successful, but result was partially or fully truncated
 *
 *  @ingroup error
 */
#define ESB_RESULT_TRUNCATED (-13)

/** Object is in a state where it cannot perform the operation
 *
 *  @ingroup error
 *  @see ESB_NOT_INITIALIZED
 */
#define ESB_INVALID_STATE (-14)

/** Object does not own the requested resource.
 *
 *  @ingroup error
 */
#define ESB_NOT_OWNER (-15)

/** Object has resources currently in use.
 *
 *  @ingroup error
 */
#define ESB_IN_USE (-16)

/** Operation cannot find the specified element
 *
 *  @ingroup error
 */
#define ESB_CANNOT_FIND (-17)

/** Operation passed an invalid iterator (an iterator with a isNull method
 *  that returns true).
 *
 *  @ingroup error
 */
#define ESB_INVALID_ITERATOR (-18)

/** Operation cannot be completed without exceeding a hard limit.
 *
 *  @ingroup error
 */
#define ESB_OVERFLOW (-19)

/** Operation cannot perform requested conversion.
 *
 *  @ingroup error
 */
#define ESB_CANNOT_CONVERT (-20)

/** Operation was passed an argument with an illegal encoding.
 *
 *  @ingroup error
 */
#define ESB_ILLEGAL_ENCODING (-21)

/** Operation was passed an unsupported character set or encoding.
 *
 *  @ingroup error
 */
#define ESB_UNSUPPORTED_CHARSET (-22)

/** Operation was passed an index outside of the bounds of an array.
 *
 *  @ingroup error
 */
#define ESB_OUT_OF_BOUNDS (-23)

/** Operation cannot be completed because shutdown has already been
 *  called.
 *
 * @ingroup error
 */
#define ESB_SHUTDOWN (-24)

/**
 * Operation is finished and should be cleaned up
 *
 * @ingroup error
 */
#define ESB_CLEANUP (-25)

/**
 * Operation has been paused
 *
 * @ingroup error
 */
#define ESB_PAUSE (-26)

/**
 * Send a response immediately
 *
 * @ingroup error
 */
#define ESB_SEND_RESPONSE (-27)

/**
 * Close the socket immediately
 *
 * @ingroup error
 */
#define ESB_CLOSE_SOCKET (-28)

/**
 * Operation has not been implemented
 *
 * @ingroup error
 */
#define ESB_NOT_IMPLEMENTED (-29)

}  // namespace ESB

#endif
