#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

namespace ESB {

NullLogger NullLogger::_Instance;

NullLogger::NullLogger() {}

NullLogger::~NullLogger() {}

bool NullLogger::isLoggable(Severity severity) { return false; }

void NullLogger::setSeverity(Severity severity) {}

Error NullLogger::log(Severity severity, const char *file, int line,
                      const char *format, ...) {
  return ESB_SUCCESS;
}

}  // namespace ESB
