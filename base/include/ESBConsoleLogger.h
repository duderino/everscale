#ifndef ESB_CONSOLE_LOGGER_H
#define ESB_CONSOLE_LOGGER_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ESB {

/** ConsoleLogger is a singleton that implements the Logger interface by
 *  writing log messages to stderr.
 *
 *  @ingroup log
 */
class ConsoleLogger : public Logger {
 public:
  /** Initialize the ConsoleLogger
   *
   *  @param severity Messages with a severity greater than or equal to
   *      this severity level will be logged.
   *  @return ESB_SUCCESS if successful, ESB_OPERATION_NOT_SUPPORTED if this
   *      platform does support console logging.
   */
  static Error Initialize(Severity severity);

  /** Destroy the ConsoleLogger
   *
   *  @return ESB_SUCCESS if successful, ESB_OPERATION_NOT_SUPPORTED if this
   *      platform does not support console logging.
   */
  static Error Destroy();

  /** Get a ConsoleLogger instance.  This method should only be called
   *  when the ConsoleLogger is initialized.
   *
   *  @return the instance
   */
  static ConsoleLogger *Instance();

  /** Destructor
   */
  virtual ~ConsoleLogger();

  /** Determine whether a log message will really be logged.
   *
   * @param severity The severity of the message to be logged
   * @return true if the messages will really be logged, false otherwise.
   */
  virtual bool isLoggable(Severity severity);

  /** Set the severity level at which messages will be logged.
   *
   *  @param severity Messages with a severity greater than or equal to
   *      this severity level will be logged
   */
  void setSeverity(Severity severity);

  /** Log a message.
   *
   *  @param severity The severity of the event.
   *  @param file The name of the file logging the message.
   *  @param line The line of the file that the message was logged.
   *  @param format A printf-style format string.
   *  @return ESB_SUCCESS if successful, ESB_NULL_POINTER if any mandatory
   *      arguments are NULL, ESB_OPERATION_NOT_SUPPORTED if this platform
   *      does not suppoort console logging.
   */
  Error log(Severity severity, const char *file, int line, const char *format,
            ...);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  // Singleton idiom
  ConsoleLogger();

  // Disabled
  ConsoleLogger(const ConsoleLogger &);
  ConsoleLogger &operator=(const ConsoleLogger &);

  static ConsoleLogger _Instance;

  Severity _severity;
};

}  // namespace ESB

#endif
