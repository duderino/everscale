/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_API_LOGGER_H
#define AWS_HTTP_API_LOGGER_H

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

class AWSHttpApiLogger : public ESFLogger {
 public:
  AWSHttpApiLogger(ESFLogger::Severity severity, aws_http_logger loggerCallback, void *loggerContext);

  virtual ~AWSHttpApiLogger();

  virtual bool isLoggable(ESFLogger::Severity severity);

  virtual void setSeverity(ESFLogger::Severity severity);

  virtual ESFError log(ESFLogger::Severity severity, const char *file, int line, const char *format, ...);

 private:
  // Disabled
  AWSHttpApiLogger(const AWSHttpApiLogger &logger);
  void operator(const AWSHttpApiLogger &logger);

  ESFLogger::Severity _severity;
  aws_http_logger _loggerCallback;
  void *_loggerContext;
};

#endif
