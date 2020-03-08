#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

namespace ESB {

Logger &Logger::_Instance = NullLogger::Instance();

Logger::Logger() {
}

Logger::~Logger() {
}

void Logger::SetInstance(Logger &logger) {
  _Instance = logger;
}

}