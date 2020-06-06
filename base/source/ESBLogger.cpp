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

void Logger::SetInstance(Logger *logger) { _Instance = logger ? logger : &NullInstance; }

const char *Logger::SeverityToString(Logger::Severity severity) {
  switch (severity) {
    case None:
      return "NON";
    case Emergency:
      return "EMR";
    case Alert:
      return "ALR";
    case Critical:
      return "CRT";
    case Err:
      return "ERR";
    case Warning:
      return "WRN";
    case Notice:
      return "NOT";
    case Info:
      return "INF";
    case Debug:
      return "DBG";
    default:
      return "OTH";
  }
}

}  // namespace ESB
