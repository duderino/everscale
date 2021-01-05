#ifndef ESB_SIMPLE_FILE_LOGGER_H
#define ESB_SIMPLE_FILE_LOGGER_H

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

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

namespace ESB {

/** SimpleFileLogger writes log messages to a file with no file rotation.
 *
 *  @ingroup log
 */
class SimpleFileLogger : public Logger {
 public:
  /** Constructor
   *
   * @param file All log messages will be written to this file handle.
   */
  SimpleFileLogger(FILE *file = stdout, Severity severity = Logger::Warning);

  /** Destructor
   */
  virtual ~SimpleFileLogger();

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
  virtual void setSeverity(Severity severity);

  /** Log a message.
   *
   *  @param severity The severity of the event.
   *  @param format A printf-style format string.
   *  @return ESB_SUCCESS if successful, ESB_NULL_POINTER if any mandatory
   *      arguments are NULL, ESB_OPERATION_NOT_SUPPORTED if this platform
   *      does not suppoort console logging.
   */
  virtual Error log(Severity severity, const char *format, ...) __attribute__((format(printf, 3, 4)));

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  // Disabled
  SimpleFileLogger(const SimpleFileLogger &);
  SimpleFileLogger &operator=(const SimpleFileLogger &);

  Severity _severity;
#ifdef HAVE_FILE_T
  FILE *_file;
#else
#error "FILE * or equivalent is required"
#endif
};

}  // namespace ESB

#endif
