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

#ifndef ESB_SYSTEM_TIME_SOURCE_H
#include <ESBSystemTimeSource.h>
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
  SimpleFileLogger(FILE *file = stdout, TimeSource &source = SystemTimeSource::Instance());

  /** Destructor
   */
  virtual ~SimpleFileLogger();

  virtual bool isLoggable(Severity severity);

  virtual void setSeverity(Severity severity);

  virtual Error log(Severity severity, const char *format, ...) __attribute__((format(printf, 3, 4)));

  virtual UInt32 now();

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

  TimeSource &_timeSource;
  Severity _severity;
#ifdef HAVE_FILE_T
  FILE *_file;
#else
#error "FILE * or equivalent is required"
#endif
};

}  // namespace ESB

#endif
