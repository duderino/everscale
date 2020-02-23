/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_ECHO_SERVER_HANDLER_H
#define AWS_HTTP_ECHO_SERVER_HANDLER_H

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

#ifndef AWS_HTTP_SERVER_HANDLER_H
#include <AWSHttpServerHandler.h>
#endif

class AWSHttpEchoServerHandler : public AWSHttpServerHandler {
 public:
  AWSHttpEchoServerHandler(ESFLogger *logger);

  virtual ~AWSHttpEchoServerHandler();

  /**
   * Accept a new connection.
   *
   * @param address The IP address and port of the client.
   * @return a result code
   */
  virtual Result acceptConnection(ESFSocketAddress *address);

  /**
   * Handle the beginning of a transaction.  This will be called 1+ times after
   * a connection has been accepted (>1 when connections are reused).  Put your
   * initialization code here.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc
   * @return a result code
   */
  virtual Result begin(AWSHttpTransaction *transaction);

  /**
   * Process a request's HTTP headers.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc.
   * @return a result code
   */
  virtual Result receiveRequestHeaders(AWSHttpTransaction *transaction);

  /**
   * Incrementally process a request's body.  This will be called 1+ times as
   * the request's HTTP body is received.  The body is finished when a
   * chunk_size of 0 is passed to the callback.  If there is no body, this
   * callback will still be called once with a chunk_size of 0.  The HTTP
   * response object must be populated before the chunk_size 0 call returns.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc.
   * @param chunk A buffer to drain
   * @param chunkSize The size of the buffer to drain, or 0 if the body is
   * finished.
   * @return a result code
   */
  virtual Result receiveRequestBody(AWSHttpTransaction *transaction,
                                    unsigned const char *chunk,
                                    unsigned int chunkSize);

  /**
   * Request a buffer of up to n bytes to fill with response body data.  The
   * stack will subsequently call fillResponseChunk with the actual size of the
   * buffer available.  The actual size of the buffer may be less than the
   * requested size.
   *
   * A good implementation will always return the known size of remaining body
   * data.  This function may be called multiple times before fillResponseChunk
   * actually writes the data to the buffer if there is insufficient space in
   * the underlying tcp buffers.  Don't 'deduct' the amount requested from the
   * remaining amount until fillResponseChunk is called.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc.
   * @return The buffer size requested.  Returning 0 ends the body.  Returning
   * -1 or less immediately closes the connection
   */
  virtual int reserveResponseChunk(AWSHttpTransaction *transaction);

  /**
   * Fill a response body chunk with data.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc.
   * @param chunk A buffer to fill
   * @param chunkSize The size of the buffer to fill.  This may be less than the
   * size requested by the requestResponseChunk method.
   */
  virtual void fillResponseChunk(AWSHttpTransaction *transaction,
                                 unsigned char *chunk, unsigned int chunkSize);

  /**
   * Handle the end of a transaction.  This is called regardless of the
   * transaction's success or failure.  Put your cleanup code here.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc
   * @param state The state at which the transaction ended
   */
  virtual void end(AWSHttpTransaction *transaction, State state);

 private:
  // Disabled
  AWSHttpEchoServerHandler(const AWSHttpEchoServerHandler &serverHandler);
  void operator=(const AWSHttpEchoServerHandler &serverHandler);

  ESFLogger *_logger;
};

#endif
