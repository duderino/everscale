#ifndef ESB_LOGGER_H
#define ESB_LOGGER_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ERRNO_H
#include <ESBError.h>
#endif

namespace ESB {

/** @defgroup log Logging
 */

/** Loggers define the interface that any concrete logger must realize.
 *  Loggers support selective logging by severity level.  The
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
class Logger {
 public:
  /** Severity-level.
   */
  typedef enum {
    None = 0,
    Emergency = 1, /**< System-wide non-recoverable error. */
    Alert = 2,     /**< System-wide non-recoverable error imminent. */
    Critical = 3,  /**< System-wide potentially recoverable error. */
    Err = 4,       /**< Localized non-recoverable error. */
    Warning = 5,   /**< Localized potentially recoverable error. */
    Notice = 6,    /**< Important non-error event. */
    Info = 7,      /**< Non-error event. */
    Debug = 8
    /**< Debugging event. */
  } Severity;

  /** Destructor
   */
  virtual ~Logger(){};

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
   *  @return ESB_SUCCESS if successful, another value otherwise.
   */
  virtual Error log(Severity severity, const char *file, int line,
                    const char *format, ...) = 0;
};

}  // namespace ESB

#endif
