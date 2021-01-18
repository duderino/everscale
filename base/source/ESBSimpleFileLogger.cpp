#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
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

SimpleFileLogger::SimpleFileLogger(FILE *file, Severity severity, bool flushable)
    : _flushable(flushable), _severity(severity), _file(file) {}

SimpleFileLogger::~SimpleFileLogger() {}

bool SimpleFileLogger::isLoggable(Severity severity) { return !(severity > _severity); }

void SimpleFileLogger::setSeverity(Severity severity) { _severity = severity; }

Error SimpleFileLogger::log(Severity severity, const char *format, ...) {
  if (!format) {
    return ESB_NULL_POINTER;
  }

  if (severity > _severity) {
    return ESB_SUCCESS;
  }

#if defined HAVE_VA_START && defined HAVE_VA_END && defined HAVE_VFPRINTF
  va_list vaList;
  va_start(vaList, format);
  vfprintf(_file, format, vaList);
  va_end(vaList);
#else
#error "va_start, vfprintf, and va_end or equivalent is required"
#endif

  return ESB_SUCCESS;
}

void SimpleFileLogger::flush() {
  if (_flushable) {
#ifdef HAVE_FFLUSH
    fflush(_file);
#else
#error "fflush or equivalent is required"
#endif
  }
}

}  // namespace ESB
