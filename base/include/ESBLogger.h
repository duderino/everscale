#ifndef ESB_LOGGER_H
#define ESB_LOGGER_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

#ifndef ESB_THREAD_H
#include <ESBThread.h>
#endif

#ifndef ESB_TIME_H
#include <ESBTime.h>
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

  Logger();
  virtual ~Logger();

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
   *  @param format A printf-style format string.
   *  @return ESB_SUCCESS if successful, another value otherwise.
   */
  virtual Error log(Severity severity, const char *format, ...) __attribute__((format(printf, 3, 4))) = 0;

  /** Write any buffered log messages to the underlying medium.
   */
  virtual void flush() = 0;

  /**
   * Get the current time in seconds since the epoch.  This may be a lower precision implementation, use Time for
   * non-logging time needs.
   *
   * @return seconds since the epoch
   */
  virtual UInt32 now() = 0;

  static void SetInstance(Logger *logger);

  static inline Logger &Instance() { return *_Instance; };

  static const char *SeverityToString(Severity severity);

 private:
  static Logger *_Instance;

  ESB_DISABLE_AUTO_COPY(Logger);
};

}  // namespace ESB

#define ESB_STRING_LITERAL2(x) #x
#define ESB_STRING_LITERAL(x) ESB_STRING_LITERAL2(x)
#define ESB_LOG_PREFIX "] "
#define ESB_LOG_SUFFIX " [" __FILE__ ":" ESB_STRING_LITERAL(__LINE__) "]\n"
//#define ESB_LOG_SUFFIX "\n"
#define ESB_EMERGENCY_LOG_PREFIX "[%u:%lu:EMR" ESB_LOG_PREFIX
#define ESB_ALERT_LOG_PREFIX "[%u:%lu:ALT" ESB_LOG_PREFIX
#define ESB_CRITICAL_LOG_PREFIX "[%u:%lu:CRT" ESB_LOG_PREFIX
#define ESB_ERROR_LOG_PREFIX "[%u:%lu:ERR" ESB_LOG_PREFIX
#define ESB_WARNING_LOG_PREFIX "[%u:%lu:WRN" ESB_LOG_PREFIX
#define ESB_NOTICE_LOG_PREFIX "[%u:%lu:NOT" ESB_LOG_PREFIX
#define ESB_INFO_LOG_PREFIX "[%u:%lu:INF" ESB_LOG_PREFIX
#define ESB_DEBUG_LOG_PREFIX "[%u:%lu:DBG" ESB_LOG_PREFIX
#define ESB_ERRNO_BUFFER_SIZE 50
#define ESB_EMERGENCY_LOGGABLE (ESB::Logger::Instance().isLoggable(ESB::Logger::Emergency))
#define ESB_ALERT_LOGGABLE (ESB::Logger::Instance().isLoggable(ESB::Logger::Alert))
#define ESB_CRITICAL_LOGGABLE (ESB::Logger::Instance().isLoggable(ESB::Logger::Critical))
#define ESB_ERROR_LOGGABLE (ESB::Logger::Instance().isLoggable(ESB::Logger::Err))
#define ESB_WARNING_LOGGABLE (ESB::Logger::Instance().isLoggable(ESB::Logger::Warning))
#define ESB_NOTICE_LOGGABLE (ESB::Logger::Instance().isLoggable(ESB::Logger::Notice))
#define ESB_INFO_LOGGABLE (ESB::Logger::Instance().isLoggable(ESB::Logger::Info))
#define ESB_DEBUG_LOGGABLE (ESB::Logger::Instance().isLoggable(ESB::Logger::Debug))

#define ESB_LOG(LOGGER, SEVERITY, FORMAT, ...)                                                   \
  do {                                                                                           \
    if (LOGGER.isLoggable(SEVERITY)) {                                                           \
      ESB::Logger::Instance().log(SEVERITY, "[%u:%lu:%s" ESB_LOG_PREFIX FORMAT ESB_LOG_SUFFIX,   \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), \
                                  ESB::Logger::SeverityToString(SEVERITY), ##__VA_ARGS__);       \
    }                                                                                            \
  } while (0)

#define ESB_LOG_EMERGENCY(FORMAT, ...)                                                                           \
  do {                                                                                                           \
    if (ESB_EMERGENCY_LOGGABLE) {                                                                                \
      ESB::Logger::Instance().log(ESB::Logger::Emergency, ESB_EMERGENCY_LOG_PREFIX FORMAT ESB_LOG_SUFFIX,        \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__); \
    }                                                                                                            \
  } while (0)

#define ESB_LOG_EMERGENCY_ERRNO(ERRNO, FORMAT, ...)                                                              \
  do {                                                                                                           \
    if (ESB_EMERGENCY_LOGGABLE) {                                                                                \
      char esb_log_buffer[ESB_ERRNO_BUFFER_SIZE];                                                                \
      ESB::DescribeError(ERRNO, esb_log_buffer, sizeof(esb_log_buffer));                                         \
      ESB::Logger::Instance().log(ESB::Logger::Emergency, ESB_EMERGENCY_LOG_PREFIX FORMAT ": %s" ESB_LOG_SUFFIX, \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__,  \
                                  esb_log_buffer);                                                               \
    }                                                                                                            \
  } while (0)

#define ESB_LOG_ALERT(FORMAT, ...)                                                                               \
  do {                                                                                                           \
    if (ESB_ALERT_LOGGABLE) {                                                                                    \
      ESB::Logger::Instance().log(ESB::Logger::Alert, ESB_ALERT_LOG_PREFIX FORMAT ESB_LOG_SUFFIX,                \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__); \
    }                                                                                                            \
  } while (0)

#define ESB_LOG_ALERT_ERRNO(ERRNO, FORMAT, ...)                                                                 \
  do {                                                                                                          \
    if (ESB_ALERT_LOGGABLE) {                                                                                   \
      char esb_log_buffer[ESB_ERRNO_BUFFER_SIZE];                                                               \
      ESB::esb_log_buffer(ERRNO, buffer, sizeof(esb_log_buffer));                                               \
      ESB::Logger::Instance().log(ESB::Logger::Alert, ESB_ALERT_LOG_PREFIX FORMAT ": %s" ESB_LOG_SUFFIX,        \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__, \
                                  esb_log_buffer);                                                              \
    }                                                                                                           \
  } while (0)

#define ESB_LOG_CRITICAL(FORMAT, ...)                                                                            \
  do {                                                                                                           \
    if (ESB_CRITICAL_LOGGABLE) {                                                                                 \
      ESB::Logger::Instance().log(ESB::Logger::Critical, ESB_CRITICAL_LOG_PREFIX FORMAT ESB_LOG_SUFFIX,          \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__); \
    }                                                                                                            \
  } while (0)

#define ESB_LOG_CRITICAL_ERRNO(ERRNO, FORMAT, ...)                                                              \
  do {                                                                                                          \
    if (ESB_CRITICAL_LOGGABLE) {                                                                                \
      char esb_log_buffer[ESB_ERRNO_BUFFER_SIZE];                                                               \
      ESB::DescribeError(ERRNO, esb_log_buffer, sizeof(esb_log_buffer));                                        \
      ESB::Logger::Instance().log(ESB::Logger::Critical, ESB_CRITICAL_LOG_PREFIX FORMAT ": %s" ESB_LOG_SUFFIX,  \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__, \
                                  esb_log_buffer);                                                              \
    }                                                                                                           \
  } while (0)

#define ESB_LOG_ERROR(FORMAT, ...)                                                                               \
  do {                                                                                                           \
    if (ESB_ERROR_LOGGABLE) {                                                                                    \
      ESB::Logger::Instance().log(ESB::Logger::Err, ESB_ERROR_LOG_PREFIX FORMAT ESB_LOG_SUFFIX,                  \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__); \
    }                                                                                                            \
  } while (0)

#define ESB_LOG_ERROR_ERRNO(ERRNO, FORMAT, ...)                                                                 \
  do {                                                                                                          \
    if (ESB_ERROR_LOGGABLE) {                                                                                   \
      char esb_log_buffer[ESB_ERRNO_BUFFER_SIZE];                                                               \
      ESB::DescribeError(ERRNO, esb_log_buffer, sizeof(esb_log_buffer));                                        \
      ESB::Logger::Instance().log(ESB::Logger::Err, ESB_ERROR_LOG_PREFIX FORMAT ": %s" ESB_LOG_SUFFIX,          \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__, \
                                  esb_log_buffer);                                                              \
    }                                                                                                           \
  } while (0)

#define ESB_LOG_WARNING(FORMAT, ...)                                                                             \
  do {                                                                                                           \
    if (ESB_WARNING_LOGGABLE) {                                                                                  \
      ESB::Logger::Instance().log(ESB::Logger::Warning, ESB_WARNING_LOG_PREFIX FORMAT ESB_LOG_SUFFIX,            \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__); \
    }                                                                                                            \
  } while (0)

#define ESB_LOG_WARNING_ERRNO(ERRNO, FORMAT, ...)                                                               \
  do {                                                                                                          \
    if (ESB_WARNING_LOGGABLE) {                                                                                 \
      char esb_log_buffer[ESB_ERRNO_BUFFER_SIZE];                                                               \
      ESB::DescribeError(ERRNO, esb_log_buffer, sizeof(esb_log_buffer));                                        \
      ESB::Logger::Instance().log(ESB::Logger::Warning, ESB_WARNING_LOG_PREFIX FORMAT ": %s" ESB_LOG_SUFFIX,    \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__, \
                                  esb_log_buffer);                                                              \
    }                                                                                                           \
  } while (0)

#define ESB_LOG_NOTICE(FORMAT, ...)                                                                              \
  do {                                                                                                           \
    if (ESB_NOTICE_LOGGABLE) {                                                                                   \
      ESB::Logger::Instance().log(ESB::Logger::Notice, ESB_NOTICE_LOG_PREFIX FORMAT ESB_LOG_SUFFIX,              \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__); \
    }                                                                                                            \
  } while (0)

#define ESB_LOG_NOTICE_ERRNO(ERRNO, FORMAT, ...)                                                           \
  do {                                                                                                     \
    if (ESB_NOTICE_LOGGABLE) {                                                                             \
      char esb_log_buffer[ESB_ERRNO_BUFFER_SIZE];                                                          \
      ESB::DescribeError(ERRNO, esb_log_buffer, sizeof(esb_log_buffer));                                   \
      ESB::Logger::Instance().log(ESB::Logger::Notice, ESB_NOTICE_LOG_PREFIX FORMAT ": %s" ESB_LOG_SUFFIX, \
                                  ESB::Thread::CurrentThreadId(), ##__VA_ARGS__, esb_log_buffer);          \
    }                                                                                                      \
  } while (0)

#define ESB_LOG_INFO(FORMAT, ...)                                                                                \
  do {                                                                                                           \
    if (ESB_INFO_LOGGABLE) {                                                                                     \
      ESB::Logger::Instance().log(ESB::Logger::Info, ESB_INFO_LOG_PREFIX FORMAT ESB_LOG_SUFFIX,                  \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__); \
    }                                                                                                            \
  } while (0)

#define ESB_LOG_INFO_ERRNO(ERRNO, FORMAT, ...)                                                                  \
  do {                                                                                                          \
    if (ESB_INFO_LOGGABLE) {                                                                                    \
      char esb_log_buffer[ESB_ERRNO_BUFFER_SIZE];                                                               \
      ESB::DescribeError(ERRNO, esb_log_buffer, sizeof(esb_log_buffer));                                        \
      ESB::Logger::Instance().log(ESB::Logger::Info, ESB_INFO_LOG_PREFIX FORMAT ": %s" ESB_LOG_SUFFIX,          \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__, \
                                  esb_log_buffer);                                                              \
    }                                                                                                           \
  } while (0)

#define ESB_LOG_DEBUG(FORMAT, ...)                                                                               \
  do {                                                                                                           \
    if (ESB_DEBUG_LOGGABLE) {                                                                                    \
      ESB::Logger::Instance().log(ESB::Logger::Debug, ESB_DEBUG_LOG_PREFIX FORMAT ESB_LOG_SUFFIX,                \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__); \
    }                                                                                                            \
  } while (0)

#define ESB_LOG_DEBUG_ERRNO(ERRNO, FORMAT, ...)                                                                 \
  do {                                                                                                          \
    if (ESB_DEBUG_LOGGABLE) {                                                                                   \
      char esb_log_buffer[ESB_ERRNO_BUFFER_SIZE];                                                               \
      ESB::DescribeError(ERRNO, esb_log_buffer, sizeof(esb_log_buffer));                                        \
      ESB::Logger::Instance().log(ESB::Logger::Debug, ESB_DEBUG_LOG_PREFIX FORMAT ": %s" ESB_LOG_SUFFIX,        \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__, \
                                  esb_log_buffer);                                                              \
    }                                                                                                           \
  } while (0)

#define ESB_LOG_BACKTRACE(FORMAT, ...)                                                                           \
  do {                                                                                                           \
    if (ESB_CRITICAL_LOGGABLE) {                                                                                 \
      ESB::Logger::Instance().log(ESB::Logger::Critical, ESB_CRITICAL_LOG_PREFIX FORMAT "\n",                    \
                                  ESB::Logger::Instance().now(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__); \
    }                                                                                                            \
  } while (0)

#endif
