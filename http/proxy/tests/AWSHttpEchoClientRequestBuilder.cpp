/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_ECHO_CLIENT_REQUEST_BUILDER_H
#include <AWSHttpEchoClientRequestBuilder.h>
#endif

ESFError AWSHttpEchoClientRequestBuilder(
    const char *host, int port, const char *absPath, const char *method,
    const char *contentType, AWSHttpClientTransaction *transaction) {
  if (0 == host || 0 == absPath || 0 == method || 0 == transaction) {
    return ESF_NULL_POINTER;
  }

  if (0 > port || 65536 <= port) {
    return ESF_INVALID_ARGUMENT;
  }

  AWSHttpRequest *request = transaction->getRequest();
  AWSHttpRequestUri *requestUri = request->getRequestUri();

  requestUri->setType(AWSHttpRequestUri::AWS_URI_HTTP);

  requestUri->setAbsPath((const unsigned char *)absPath);

  request->setMethod((const unsigned char *)method);

  ESFError error = request->addHeader(
      transaction->getAllocator(), (const unsigned char *)"Host",
      (const unsigned char *)"%s:%d", host, port);

  if (ESF_SUCCESS != error) {
    return error;
  }

  if (contentType) {
    error = request->addHeader((const unsigned char *)"Content-Type",
                               (const unsigned char *)contentType,
                               transaction->getAllocator());

    if (ESF_SUCCESS != error) {
      return error;
    }
  }

  error = request->addHeader((const unsigned char *)"Transfer-Encoding",
                             (const unsigned char *)"chunked",
                             transaction->getAllocator());

  if (ESF_SUCCESS != error) {
    return error;
  }

  // Body is hardcoded in AWSHttpEchoClientHandler.cpp

  return ESF_SUCCESS;
}
