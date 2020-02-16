/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_ECHO_CLIENT_HANDLER_H
#define AWS_HTTP_ECHO_CLIENT_HANDLER_H

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

#ifndef ESF_SHARED_COUNTER_H
#include <ESFSharedCounter.h>
#endif

#ifndef AWS_PERFORMANCE_COUNTER_H
#include <AWSPerformanceCounter.h>
#endif

#ifndef AWS_HTTP_CLIENT_HANDLER_H
#include <AWSHttpClientHandler.h>
#endif

#ifndef AWS_HTTP_CONNECTION_POOL_H
#include <AWSHttpConnectionPool.h>
#endif

class AWSHttpEchoClientHandler : public AWSHttpClientHandler {
 public:
  AWSHttpEchoClientHandler(const char *absPath, const char *method,
                           const char *contentType, const unsigned char *body,
                           int bodySize, int totalTransactions,
                           AWSHttpConnectionPool *pool, ESFLogger *logger);

  virtual ~AWSHttpEchoClientHandler();

  /**
   * Request a buffer of up to n bytes to fill with request body data.  The
   * stack will subsequently call fillRequestChunk with the actual size of the
   * buffer available.  The actual size of the buffer may be less than the
   * requested size.
   *
   * A good implementation will always return the known size of remaining body
   * data.  This function may be called multiple times before fillRequestChunk
   * actually writes the data to the buffer if there is insufficient space in
   * the underlying tcp buffers.  Don't 'deduct' the amount requested from the
   * remaining amount until fillRequestChunk is called.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc.
   * @return The buffer size requested.  Returning 0 ends the body.  Returning
   * -1 or less immediately closes the connection
   */
  virtual int reserveRequestChunk(AWSHttpTransaction *transaction);

  /**
   * Fill a request body chunk with data.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc.
   * @param chunk A buffer to fill
   * @param chunkSize The size of the buffer to fill.  This may be less than the
   * size requested by the requestRequestChunk method.
   */
  virtual void fillRequestChunk(AWSHttpTransaction *transaction,
                                unsigned char *chunk, unsigned int chunkSize);

  /**
   * Process a request's HTTP headers.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc.
   * @return a result code
   */
  virtual Result receiveResponseHeaders(AWSHttpTransaction *transaction);

  /**
   * Incrementally process a response body.  This will be called 1+ times as the
   * HTTP response body is received.  The body is finished when a chunk_size of
   * 0 is passed to the callback.  If there is no body, this callback will still
   * be called once with a chunk_size of 0.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc.
   * @param chunk A buffer to drain
   * @param chunkSize The size of the buffer to drain, or 0 if the body is
   * finished.
   * @return a result code
   */
  virtual Result receiveResponseBody(AWSHttpTransaction *transaction,
                                     unsigned const char *chunk,
                                     unsigned int chunkSize);

  /**
   * Handle the end of a transaction.  This is called regardless of the
   * transaction's success or failure.  Put your cleanup code here.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc
   * @param state The state at which the transaction ended
   */
  virtual void end(AWSHttpTransaction *transaction, State state);

  inline bool isFinished() const {
    return _totalTransactions <= _completedTransactions.get();
  }

 private:
  // Disabled
  AWSHttpEchoClientHandler(const AWSHttpEchoClientHandler &clientHandler);
  void operator=(const AWSHttpEchoClientHandler &clientHandler);

  const char *_absPath;
  const char *_method;
  const char *_contentType;
  const unsigned char *_body;
  const int _bodySize;
  int _totalTransactions;
  AWSHttpConnectionPool *_pool;
  ESFLogger *_logger;
  ESFSharedCounter _completedTransactions;
};

#endif
