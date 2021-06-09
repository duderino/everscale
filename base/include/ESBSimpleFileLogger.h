#ifndef ESB_SIMPLE_FILE_LOGGER_H
#define ESB_SIMPLE_FILE_LOGGER_H

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
  SimpleFileLogger(FILE *file = stdout, Severity severity = Logger::Warning,
                   TimeSource &source = SystemTimeSource::Instance(), bool flushable = true);

  /** Destructor
   */
  virtual ~SimpleFileLogger();

  virtual bool isLoggable(Severity severity);

  virtual void setSeverity(Severity severity);

  virtual Error log(Severity severity, const char *format, ...) __attribute__((format(printf, 3, 4)));

  virtual void flush();

  virtual UInt32 now();

 private:
  bool _flushable;
  TimeSource &_timeSource;
  Severity _severity;
#ifdef HAVE_FILE_T
  FILE *_file;
#else
#error "FILE * or equivalent is required"
#endif

  ESB_DEFAULT_FUNCS(SimpleFileLogger);
};

}  // namespace ESB

#endif
