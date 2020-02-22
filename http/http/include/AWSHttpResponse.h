/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_RESPONSE_H
#define AWS_HTTP_RESPONSE_H

#ifndef AWS_HTTP_MESSAGE_H
#include <AWSHttpMessage.h>
#endif

/**
 * A HTTP Response as defined in RFC 2616 and RFC 2396
 */
class AWSHttpResponse : public AWSHttpMessage {
 public:
  AWSHttpResponse();

  virtual ~AWSHttpResponse();

  void reset();

  inline void setStatusCode(int statusCode) { _statusCode = statusCode; }

  inline int getStatusCode() const { return _statusCode; }

  inline void setReasonPhrase(const unsigned char *reasonPhrase) {
    _reasonPhrase = reasonPhrase;
  }

  inline const unsigned char *getReasonPhrase() const { return _reasonPhrase; }

 private:
  // Disabled
  AWSHttpResponse(const AWSHttpResponse &);
  void operator=(const AWSHttpResponse &);

  int _statusCode;
  unsigned const char *_reasonPhrase;
};

#endif
