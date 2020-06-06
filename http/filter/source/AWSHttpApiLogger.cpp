/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_API_LOGGER_H
#include <AWSHIttpApiLogger.h>
#endif

AWSHttpApiLogger::AWSHttpApiLogger(ESFLogger::Severity severity, aws_http_logger loggerCallback, void *loggerContext)
    : _severity(severity), _loggerCallback(loggerCallback), _loggerContext(loggerContext) {}

AWSHttpApiLogger::~AWSHttpApiLogger() {}

bool AWSHttpApiLogger::isLoggable(ESFLogger::Severity severity) { return severity > _severity ? false : true; }

void AWSHttpApiLogger::setSeverity(ESFLogger::Severity severity) { _severity = severity; }

ESFError AWSHttpApiLogger::log(ESFLogger::Severity severity, const char *file, int line, const char *format, ...) {
  if (0 == file || 0 == format) {
    return ESF_NULL_POINTER;
  }

  if (0 == _loggerCallback) {
    return ESF_INVALID_STATE;
  }

  if (false == isLoggable(severity)) {
    return ESF_SUCCESS;
  }

  va_list vaList;

  va_start(vaList, format);

  _loggerCallback(_loggerContext, (aws_http_log_level)severity, file, line, format, vaList);

  va_end(vaList);

  return ESF_SUCCESS;
}
