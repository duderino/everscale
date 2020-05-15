#ifndef ES_HTTP_SERVER_HANDLER_H
#define ES_HTTP_SERVER_HANDLER_H

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ES_HTTP_MULTIPLEXER_H
#include <ESHttpMultiplexer.h>
#endif

#ifndef ES_HTTP_SERVER_STREAM_H
#include <ESHttpServerStream.h>
#endif

namespace ES {

class HttpServerHandler {
 public:
  typedef enum {
    ES_HTTP_SERVER_HANDLER_BEGIN = 0, /**< Connection accepted or reused */
    ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS =
        1, /**< Trying to parse request's http headers */
    ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY =
        2, /**< Trying to read request's http body */
    ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS =
        3, /**< Trying to format and send response's http headers */
    ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY =
        4,                         /**< Trying to send response's http body */
    ES_HTTP_SERVER_HANDLER_END = 5 /**< Response sent / transaction completed */
  } State;

  HttpServerHandler();

  virtual ~HttpServerHandler();

  /**
   * Accept a new connection.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param address The IP address and port of the client peer.
   * @return ESB_SUCCESS to continue processing, ESB_SEND_RESPONSE to send
   *  a HTTP response now, or any other value to immediately close the socket.
   */
  virtual ESB::Error acceptConnection(HttpMultiplexer &multiplexer,
                                      ESB::SocketAddress *address) = 0;

  /**
   * Handle the beginning of a transaction.  This will be called 1+ times after
   * a connection has been accepted (>1 when connections are reused).  Put your
   * initialization code here.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The server stream, including request and response objects
   * @return ESB_SUCCESS to continue processing, ESB_SEND_RESPONSE to send
   *  a HTTP response now, or any other value to immediately close the socket.
   */
  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer,
                                      HttpServerStream &stream) = 0;

  /**
   * Process a request's HTTP headers.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The server stream, including request and response objects
   * @return ESB_SUCCESS to continue processing, ESB_SEND_RESPONSE to send
   *  a HTTP response now, or any other value to immediately close the socket.
   */
  virtual ESB::Error receiveRequestHeaders(HttpMultiplexer &multiplexer,
                                           HttpServerStream &stream) = 0;

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
  virtual ESB::Error requestChunkCapacity(HttpMultiplexer &multiplexer,
                                          HttpServerStream &stream,
                                          ESB::UInt32 *maxChunkSize) = 0;

  /**
   * Incrementally process a request's body.  This will be called 1+ times as
   * the request's HTTP body is received.  The body is finished when a
   * chunk_size of 0 is passed to the callback.  If there is no body, this
   * callback will still be called once with a chunk_size of 0.  The HTTP
   * response object must be populated before the chunk_size 0 call returns.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The server stream, including request and response objects
   * @param chunk A buffer to drain
   * @param chunkSize The size of the buffer to drain, or 0 if the body is
   * finished.
   * @return ESB_SUCCESS to continue processing, ESB_SEND_RESPONSE to send
   *  a HTTP response now, or any other value to immediately close the socket.
   */
  virtual ESB::Error receiveRequestChunk(HttpMultiplexer &multiplexer,
                                         HttpServerStream &stream,
                                         unsigned const char *chunk,
                                         ESB::UInt32 chunkSize) = 0;

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
  virtual ESB::Error offerResponseChunk(HttpMultiplexer &multiplexer,
                                        HttpServerStream &stream,
                                        ESB::UInt32 *offeredChunkSize) = 0;

  /**
   * Fill a response body chunk with data.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The server stream, including request and response objects
   * @param chunk A buffer to fill
   * @param chunkSize The size of the buffer to fill.  This may be less than the
   * size requested by the requestResponseChunk method.
   * @return ESB_SUCCESS to continue processing or any other value to
   * immediately close the socket.
   */
  virtual ESB::Error takeResponseChunk(HttpMultiplexer &multiplexer,
                                       HttpServerStream &stream,
                                       unsigned char *chunk,
                                       ESB::UInt32 chunkSize) = 0;

  /**
   * Handle the end of a transaction.  This is called regardless of the
   * transaction's success or failure.  Put your cleanup code here.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The server stream, including request and response objects
   * @param state The state at which the transaction ended.
   * ES_HTTP_SERVER_HANDLER_END means the transaction was successfully
   * completed, any other state indicates error - reason will be in the server
   * logs.
   */
  virtual void endTransaction(HttpMultiplexer &multiplexer,
                              HttpServerStream &stream, State state) = 0;

 private:
  // Disabled
  HttpServerHandler(const HttpServerHandler &serverHandler);
  void operator=(const HttpServerHandler &serverHandler);
};

}  // namespace ES

#endif
