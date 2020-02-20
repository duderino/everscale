#ifndef ESB_CONSOLE_LOGGER_H
#include <ESBConsoleLogger.h>
#endif

#ifdef ALLOW_CONSOLE_LOGGING

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

#ifndef ESB_CONSOLE_LOGGER_BUFFER_SIZE
#define ESB_CONSOLE_LOGGER_BUFFER_SIZE 256
#endif

static const char *EmergencyString = "EMERGENCY";
static const char *AlertString = "ALERT";
static const char *CriticalString = "CRITICAL";
static const char *ErrorString = "ERROR";
static const char *WarningString = "WARNING";
static const char *NoticeString = "NOTICE";
static const char *InfoString = "INFO";
static const char *DebugString = "DEBUG";

static const char *CodeToString(Logger::Severity severity) {
  switch (severity) {
    case Logger::Emergency:
      return EmergencyString;
    case Logger::Alert:
      return AlertString;
    case Logger::Critical:
      return CriticalString;
    case Logger::Err:
      return ErrorString;
    case Logger::Warning:
      return WarningString;
    case Logger::Notice:
      return NoticeString;
    case Logger::Info:
      return InfoString;
    case Logger::Debug:
      return DebugString;
    default:
      return "";
  }
}

#endif /* defined ALLOW_CONSOLE_LOGGING */

ConsoleLogger ConsoleLogger::_Instance;

Error ConsoleLogger::Initialize(Severity severity) {
  _Instance._severity = severity;

#ifdef ALLOW_CONSOLE_LOGGING
  return ESB_SUCCESS;
#else
  return ESB_OPERATION_NOT_SUPPORTED;
#endif
}

Error ConsoleLogger::Destroy() {
#ifdef ALLOW_CONSOLE_LOGGING
  return ESB_SUCCESS;
#else
  return ESB_OPERATION_NOT_SUPPORTED;
#endif
}

ConsoleLogger *ConsoleLogger::Instance() { return &_Instance; }

ConsoleLogger::ConsoleLogger() : _severity(None) {}

ConsoleLogger::~ConsoleLogger() {}

bool ConsoleLogger::isLoggable(Severity severity) {
  return severity > _severity ? false : true;
}

void ConsoleLogger::setSeverity(Severity severity) { _severity = severity; }

Error ConsoleLogger::log(Severity severity, const char *file, int line,
                         const char *format, ...) {
#ifdef ALLOW_CONSOLE_LOGGING
  if (!file || !format) {
    return ESB_NULL_POINTER;
  }

  if (severity > _severity) {
    return ESB_SUCCESS;
  }

#ifdef HAVE_STRLEN
  int length = strlen(format);
#else
#error "strlen or equivalent is required"
#endif

#ifdef HAVE_VA_START
  va_list vaList;
  va_start(vaList, format);
#else
#error "va_start or equivalent is required"
#endif

  //  55 is a guess for the length of the data we prepend to the message.
  if (length + 55 >= ESB_CONSOLE_LOGGER_BUFFER_SIZE) {
#ifdef HAVE_VFPRINTF
    vfprintf(stderr, format, vaList);
#else
#error "vfprintf or equivalent is required"
#endif

#ifdef HAVE_VA_END
    va_end(vaList);
#else
#error "va_end or equivalent is required"
#endif
    fprintf(stderr, "\n");

    return ESB_SUCCESS;
  }

  char buffer[ESB_CONSOLE_LOGGER_BUFFER_SIZE];

#ifdef HAVE_SNPRINTF
  snprintf(buffer, sizeof(buffer), "%s,%s,%d,%" ESB_THREAD_ID_FORMAT " ",
           CodeToString(severity), file, line, Thread::GetThreadId());
#else
#error "snprintf or equivalent is required"
#endif

#ifdef HAVE_STRLEN
  if (length + strlen(buffer) >= ESB_CONSOLE_LOGGER_BUFFER_SIZE)
#else
#error "strlen or equivalent is required"
#endif
  {
#ifdef HAVE_VFPRINTF
    vfprintf(stderr, format, vaList);
#else
#error "vfprintf or equivalent is required"
#endif

#ifdef HAVE_VA_END
    va_end(vaList);
#else
#error "va_end or equivalent is required"
#endif

    fprintf(stderr, "\n");

    return ESB_SUCCESS;
  }

#ifdef HAVE_STRCAT
  strcat(buffer, format);
#else
#error "strcat or equivalent is required"
#endif

#ifdef HAVE_VFPRINTF
  vfprintf(stderr, buffer, vaList);
#else
#error "vfprintf or equivalent is required"
#endif

#ifdef HAVE_VA_END
  va_end(vaList);
#else
#error "va_end or equivalent is required"
#endif

  fprintf(stderr, "\n");

  return ESB_SUCCESS;

#else /* ! defined ALLOW_CONSOLE_LOGGING */

  return ESB_OPERATION_NOT_SUPPORTED;

#endif
}
}
