/** @file ESFLogger.h
 *  @brief A generic logging interface with support for severity levels
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

#ifndef ESF_LOGGER_H
#define ESF_LOGGER_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ERRNO_H
#include <ESFError.h>
#endif

/** @defgroup log Logging
 */

/** ESFLoggers define the interface that any concrete logger must realize.
 *  ESFLoggers support selective logging by severity level.  The
 *  implementation of the log (i.e., flat file, console, syslog, shared memory)
 *  is left to the implementing subclasses.
 *  <p>
 *  Severity Levels:
 *  <ul>
 *      <li> Emergency.  System cannot function.
 *      <li> Alert.  Immediate intervention is required or else the system
 *           may stop functioning.
 *      <li> Critical.  Severe conditions exist, but the system can continue
 *           to function, perhaps at a reduced level.  Example:  Memory
 *           resources are low causing the system to slow the rate its willing
 *           to accept new tasks.  Critical conditions can affect many
 *           transactions.
 *      <li> Error.  Error conditions exist, but the system can continue to
 *           function.  Example:  An error processing a transaction occurred
 *           causing the loss of that transaction, but other transactions are
 *           unaffected.
 *      <li> Warning.  Possible error conditions exist, but the system can
 *           continue to function.  Example: An error processing a transaction
 *           occurred, but the transaction may still be completed.  Also, other
 *           transactions are unaffected.
 *      <li> Notice.  An important event has occurred that is part of the
 *           system's normal operation.  Example:  A module has started or
 *           shutdown.
 *      <li> Info.  An event has occurred that is part of the system's normal
 *           operation.  Example:  A transaction has been completed.
 *      <li> Debug.  Verbose output suitable for debugging.  Example:  A new
 *           request is received and complete information on its properties
 *           is logged.
 *  </ul>
 *  </p>
 *
 *  @ingroup log
 */
class ESFLogger {
 public:
  /** Severity-level.
   */
  typedef enum {
    None = 0,
    Emergency = 1, /**< System-wide non-recoverable error. */
    Alert = 2,     /**< System-wide non-recoverable error imminent. */
    Critical = 3,  /**< System-wide potentially recoverable error. */
    Error = 4,     /**< Localized non-recoverable error. */
    Warning = 5,   /**< Localized potentially recoverable error. */
    Notice = 6,    /**< Important non-error event. */
    Info = 7,      /**< Non-error event. */
    Debug = 8
    /**< Debugging event. */
  } Severity;

  /** Destructor
   */
  virtual ~ESFLogger(){};

  /** Determine whether a log message will really be logged.
   *
   * @param severity The severity of the message to be logged
   * @return true if the messages will really be logged, false otherwise.
   */
  virtual bool isLoggable(Severity severity) = 0;

  /** Set the severity level at which messages will be logged.
   *
   *  @param severity Messages with a severity greater than or equal to
   *      this severity level will be logged
   */
  virtual void setSeverity(Severity severity) = 0;

  /** Log a message.
   *
   *  @param severity The severity of the event.
   *  @param file The name of the file logging the message.
   *  @param line The line of the file that the message was logged.
   *  @param format A printf-style format string.
   *  @return ESF_SUCCESS if successful, another value otherwise.
   */
  virtual ESFError log(Severity severity, const char *file, int line,
                       const char *format, ...) = 0;
};

#endif /* ! ESF_LOGGER_H */
