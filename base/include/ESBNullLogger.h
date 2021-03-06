#ifndef ESB_NULL_LOGGER_H
#define ESB_NULL_LOGGER_H

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ESB {

/** A no-op implementation of the Logger interface.
 *
 *  @ingroup log
 */
class NullLogger : public Logger {
 public:
  /** Constructor
   */
  NullLogger();

  /** Destructor
   */
  virtual ~NullLogger();

  virtual bool isLoggable(Severity severity);

  virtual void setSeverity(Severity severity);

  virtual Error log(Severity severity, const char *format, ...) __attribute__((format(printf, 3, 4)));

  virtual void flush();

  virtual UInt32 now();

  ESB_DEFAULT_FUNCS(NullLogger);
};

}  // namespace ESB

#endif
