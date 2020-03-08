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
  /** Singleton accessor.
   */
  inline static NullLogger *GetInstance() { return &_Instance; }

  /** Destructor
   */
  virtual ~NullLogger();

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
   *  @param file The name of the file logging the message.
   *  @param line The line of the file that the message was logged.
   *  @param format A printf-style format string.
   *  @return ESB_SUCCESS if successful, another value otherwise.
   */
  virtual Error log(Severity severity, const char *format, ...) __attribute__ ((format (printf, 3, 4)));

 private:
  /** Constructor
   */
  NullLogger();

  // Disabled
  NullLogger(const NullLogger &);
  void operator=(const NullLogger &);

  static NullLogger _Instance;
};

}  // namespace ESB

#endif
