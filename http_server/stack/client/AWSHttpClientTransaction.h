/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_CLIENT_TRANSACTION_H
#define AWS_HTTP_CLIENT_TRANSACTION_H

#ifndef AWS_HTTP_TRANSACTION_H
#include <AWSHttpTransaction.h>
#endif

#ifndef AWS_HTTP_CLIENT_HANDLER_H
#include <AWSHttpClientHandler.h>
#endif

#ifndef AWS_HTTP_REQUEST_FORMATTER_H
#include <AWSHttpRequestFormatter.h>
#endif

#ifndef AWS_HTTP_RESPONSE_PARSER_H
#include <AWSHttpResponseParser.h>
#endif

class AWSHttpClientTransaction : public AWSHttpTransaction {
 public:
  AWSHttpClientTransaction(AWSHttpClientHandler *clientHandler,
                           ESFCleanupHandler *cleanupHandler);

  AWSHttpClientTransaction(AWSHttpClientHandler *clientHandler,
                           ESFSocketAddress *peerAddress,
                           ESFCleanupHandler *cleanupHandler);

  virtual ~AWSHttpClientTransaction();

  virtual void reset();

  inline void setHandler(AWSHttpClientHandler *clientHandler) {
    _clientHandler = clientHandler;
  }

  inline const AWSHttpClientHandler *getHandler() const {
    return _clientHandler;
  }

  inline AWSHttpClientHandler *getHandler() { return _clientHandler; }

  inline AWSHttpResponseParser *getParser() { return &_parser; }

  inline const AWSHttpResponseParser *getParser() const { return &_parser; }

  inline AWSHttpRequestFormatter *getFormatter() { return &_formatter; }

  inline const AWSHttpRequestFormatter *getFormatter() const {
    return &_formatter;
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  // Disabled
  AWSHttpClientTransaction(const AWSHttpClientTransaction &transaction);
  void operator=(const AWSHttpClientTransaction &transaction);

  AWSHttpClientHandler *_clientHandler;
  AWSHttpResponseParser _parser;
  AWSHttpRequestFormatter _formatter;
};

#endif
