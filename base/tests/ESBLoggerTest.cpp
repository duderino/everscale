#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_CONSOLE_LOGGER_H
#include <ESBConsoleLogger.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#include <gtest/gtest.h>

namespace ESB {

class TestLogger : public Logger {
 public:
  TestLogger(Logger &logger)
      : _lastSeverity(Logger::None), _lastFormat(0), _logger(logger){};
  virtual ~TestLogger(){};

  /** Determine whether a log message will really be logged.
   *
   * @param severity The severity of the message to be logged
   * @return true if the messages will really be logged, false otherwise.
   */
  virtual bool isLoggable(Severity severity) {
    _lastSeverity = severity;
    return _logger.isLoggable(severity);
  }

  /** Set the severity level at which messages will be logged.
   *
   *  @param severity Messages with a severity greater than or equal to
   *      this severity level will be logged
   */
  virtual void setSeverity(Severity severity) {
    _lastSeverity = severity;
    return _logger.setSeverity(severity);
  }

  /** Log a message.
   *
   *  @param severity The severity of the event.
   *  @param format A printf-style format string.
   *  @return ESB_SUCCESS if successful, another value otherwise.
   */
  virtual Error log(Severity severity, const char *format, ...)
      __attribute__((format(printf, 3, 4))) {
    _lastFormat = format;
    return ESB_SUCCESS;
  }

  inline Severity getLastSeverity() { return _lastSeverity; }

  inline const char *getLastFormat() { return _lastFormat; }

 private:
  Severity _lastSeverity;
  const char *_lastFormat;
  Logger &_logger;
};

TEST(Logger, NullLogger) {
  Logger::SetInstance(0);

  ESB_LOG_DEBUG("foo = %s", "foo");
  ESB_LOG_WARNING("bar = %s", "bar");
  ESB_LOG_ERROR("baz = %s", "baz");
  ESB_LOG_CRITICAL("qux");
}

TEST(Logger, ConsoleLogger) {
  ConsoleLogger logger;
  logger.setSeverity(Logger::Warning);
  Logger::SetInstance(&logger);

  ESB_LOG_DEBUG("foo = %s", "foo");
  ESB_LOG_WARNING("bar = %s", "bar");
  ESB_LOG_ERROR("baz = %s", "baz");
  ESB_LOG_CRITICAL("qux");
}

TEST(Logger, Severity) {
  ConsoleLogger logger;
  TestLogger decorator(logger);
  decorator.setSeverity(Logger::Warning);
  Logger::SetInstance(&decorator);

  ESB_LOG_DEBUG("foo = %s", "foo");
  EXPECT_EQ(Logger::Debug, decorator.getLastSeverity());
  EXPECT_EQ(0, decorator.getLastFormat());

  ESB_LOG_WARNING("bar = %s", "bar");
  EXPECT_EQ(Logger::Warning, decorator.getLastSeverity());
  EXPECT_TRUE(strstr(ESB_SAFE_STR(decorator.getLastFormat()), "bar"));

  ESB_LOG_ERROR("baz = %s", "baz");
  EXPECT_EQ(Logger::Err, decorator.getLastSeverity());
  EXPECT_TRUE(strstr(ESB_SAFE_STR(decorator.getLastFormat()), "baz"));

  ESB_LOG_CRITICAL("qux");
  EXPECT_EQ(Logger::Critical, decorator.getLastSeverity());
  EXPECT_TRUE(strstr(ESB_SAFE_STR(decorator.getLastFormat()), "qux"));
}

TEST(Logger, Errno) {
  ConsoleLogger logger;
  logger.setSeverity(Logger::Warning);
  Logger::SetInstance(&logger);

  ESB_LOG_WARNING_ERRNO(ConvertError(ENOMEM), "errno test");
  ESB_LOG_WARNING_ERRNO(ConvertError(EAFNOSUPPORT), "errno test");
}

}  // namespace ESB