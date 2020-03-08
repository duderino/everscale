#ifndef ESB_CONSOLE_LOGGER_H
#include <ESBConsoleLogger.h>
#endif
#ifndef ESB_THREAD_H
#include <ESBThread.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#else
#error "Need stdio.h or equivalent"
#endif

#if defined HAVE_STDARG_H
#include <stdarg.h>
#else
#error "Need stdarg.h or equivalent"
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#error "Need string.h or equivalent"
#endif

namespace ESB {

ConsoleLogger::ConsoleLogger() : _severity(None) {}

ConsoleLogger::~ConsoleLogger() {}

bool ConsoleLogger::isLoggable(Severity severity) {
  return !(severity > _severity);
}

void ConsoleLogger::setSeverity(Severity severity) { _severity = severity; }

Error ConsoleLogger::log(Severity severity, const char *format, ...) {
  if (!format) {
    return ESB_NULL_POINTER;
  }

  if (severity > _severity) {
    return ESB_SUCCESS;
  }

#if defined HAVE_VA_START && defined HAVE_VA_END && defined HAVE_VFPRINTF
  va_list vaList;
  va_start(vaList, format);
  vfprintf(stderr, format, vaList);
  va_end(vaList);
#else
#error "va_start, vfprintf, and va_end or equivalent is required"
#endif

  return ESB_SUCCESS;
}

}  // namespace ESB
