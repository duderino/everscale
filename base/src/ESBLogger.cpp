#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

namespace ESB {

static NullLogger NullInstance;
Logger *Logger::_Instance = &NullInstance;

Logger::Logger() {}

Logger::~Logger() {}

void Logger::SetInstance(Logger *logger) {
  _Instance = logger ? logger : &NullInstance;
}

const char *Logger::SeverityToString(Logger::Severity severity) {
  switch (severity) {
    case None:
      return "NONE";
    case Emergency:
      return "EMERGENCY";
    case Alert:
      return "ALERT";
    case Critical:
      return "CRITICAL";
    case Err:
      return "ERROR";
    case Warning:
      return "WARNING";
    case Notice:
      return "NOTICE";
    case Info:
      return "INFO";
    case Debug:
      return "DEBUG";
    default:
      return "OTHER";
  }
}

}  // namespace ESB