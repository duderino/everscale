/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_API_H
#define AWS_HTTP_API_H

#ifndef AWS_HTTP_H
#include <aws_http.h>
#endif

#ifndef AWS_HTTP_API_LOGGER_H
#include <AWSHttpApiLogger.h>
#endif

#ifndef AWS_HTTP_STACK_H
#include <AWSHttpStack.h>
#endif

class AWSHttpApi {
 public:
  AWSHttpApi(aws_http_server *server, aws_http_server_config *server_config, aws_http_log_level log_level,
             void *log_context, aws_http_logger logger);

  virtual ~AWSHttpApi();

  inline ESFError initialize() { return _stack.initialize(); }

  inline ESFError start() { return _stack.start(); }

  inline ESFError stop() { return _stack.stop(); }

  inline void destroy() { return _stack.destroy(); }

  inline void setLogLevel(ESFLogger::Severity logLevel) { _logger.setSeverity(logLevel); }

  inline AWSHttpTransaction *createTransaction() { return _stack.createTransaction(); }

  inline ESFError sendTransaction(AWSHttpTransaction *transaction) { return _stack.sendTransaction(transaction); }

  inline void destroyTransaction(AWSHttpTransaction *transaction) { _stack.destroyTransaction(transaction); }

 private:
  // Disabled
  AWSHttpApi(const AWSHttpApi &api);
  void operator=(const AWSHttpApi &api);

  AWSHttpApiLogger _logger;
  AWSHttpStack _stack;
};

#endif
