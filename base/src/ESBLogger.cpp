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

}  // namespace ESB