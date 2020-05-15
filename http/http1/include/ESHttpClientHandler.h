#ifndef ES_HTTP_CLIENT_HANDLER_H
#define ES_HTTP_CLIENT_HANDLER_H

#ifndef ES_HTTP_MULTIPLEXER_H
#include <ESHttpMultiplexer.h>
#endif

#ifndef ES_HTTP_CLIENT_STREAM_H
#include <ESHttpClientStream.h>
#endif

namespace ES {

class HttpClientHandler {
 public:
  typedef enum {
    ES_HTTP_CLIENT_HANDLER_BEGIN = 0,   /**< Transaction starting */
    ES_HTTP_CLIENT_HANDLER_RESOLVE = 1, /**< Trying to resolve client address */
    ES_HTTP_CLIENT_HANDLER_CONNECT = 2, /**< Trying to connect to client */
    ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS =
        3, /**< Trying to send request headers */
    ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY =
        4, /**< Trying to send request body */
    ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS =
        5, /**< Trying to receive/parse response headers */
    ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY =
        6, /**< Trying to receive response body */
    ES_HTTP_CLIENT_HANDLER_END =
        7 /**< Response received / transaction completed */
  } State;

  HttpClientHandler();

  virtual ~HttpClientHandler();

  /**
   * Determine the max chunk size the handler can fill.  The stack will
   * subsequently call fillRequestChunk with an empty buffer of size less than
   * or equal to the max chunk size.   The handler may try to read or generate
   * more chunk data as a side effect.
   *
   * Note: to finish the body/send the last chunk, the handler should return
   * ESB_SUCCESS and set maxChunkSize to 0.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The client stream, including request and response objects
   * @param offeredChunkSize The max chunk size the handler can generate should
   * be written here.
   * @return ESB_SUCCESS if successful, another error code otherwise.  Any
   * return value other than ESB_SUCCESS and ESB_AGAIN will abort the current
   * transaction.
   */
  virtual ESB::Error offerRequestChunk(HttpMultiplexer &multiplexer,
                                       HttpClientStream &stream,
                                       ESB::UInt32 *offeredChunkSize) = 0;

  /**
   * Fill a request body chunk with data.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The client stream, including request and response objects
   * @param chunk A buffer to fill
   * @param chunkSize The size of the buffer to fill.  This may be less than the
   * size requested by the requestRequestChunk method.
   * @return ESB_SUCCESS to continue processing or any other value to
   * immediately close the socket.
   */
  virtual ESB::Error takeResponseChunk(HttpMultiplexer &multiplexer,
                                       HttpClientStream &stream,
                                       unsigned char *chunk,
                                       ESB::UInt32 chunkSize) = 0;

  /**
   * Process a request's HTTP headers.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param transaction The http transaction - contains request and response.
   * @return ESB_SUCCESS to continue processing or any other value to
   * immediately close the socket.
   */
  virtual ESB::Error receiveResponseHeaders(HttpMultiplexer &multiplexer,
                                            HttpClientStream &stream) = 0;

  /**
   * Determine the max chunk size the handler is ready to receive.  The stack
   * will subsequently call receiveRequestChunk with chunk data of size less
   * than or equal to the max chunk size.   The handler may try to make more
   * free space available as a side effect.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The client stream, including request and response objects
   * @param maxChunkSize The max chunk size the handler can receive should be
   * written here.
   * @return ESB_SUCCESS if successful, another error code otherwise (if, for
   * instance, an error occurred flushing data to a socket to make more room).
   * Any return value other than ESB_SUCCESS and ESB_AGAIN will abort the
   * current transaction.
   */
  virtual ESB::Error responseChunkCapacity(HttpMultiplexer &multiplexer,
                                           HttpClientStream &stream,
                                           ESB::UInt32 *maxChunkSize) = 0;

  /**
   * Incrementally process a response body.  This will be called 1+ times as the
   * HTTP response body is received.  The body is finished when a chunk_size of
   * 0 is passed to the callback.  If there is no body, this callback will still
   * be called once with a chunk_size of 0.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The client stream, including request and response objects
   * @param chunk A buffer to drain
   * @param chunkSize The size of the buffer to drain, or 0 if the body is
   * finished.
   * @return ESB_SUCCESS to continue processing or any other value to
   * immediately close the socket.
   */
  virtual ESB::Error receiveResponseChunk(HttpMultiplexer &multiplexer,
                                          HttpClientStream &stream,
                                          unsigned const char *chunk,
                                          ESB::UInt32 chunkSize) = 0;

  /**
   * Handle the end of a transaction.  This is called regardless of the
   * transaction's success or failure.  Put your cleanup code here.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The client stream, including request and response objects
   * @param state The state at which the transaction ended.
   * ES_HTTP_CLIENT_HANDLER_END means the transaction was successfully
   * completed, any other state indicates error - reason will be in the server
   * logs.
   */
  virtual void endTransaction(HttpMultiplexer &multiplexer,
                              HttpClientStream &stream, State state) = 0;

 private:
  // Disabled
  HttpClientHandler(const HttpClientHandler &clientHandler);
  void operator=(const HttpClientHandler &clientHandler);
};

}  // namespace ES

#endif
