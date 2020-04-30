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
    ES_HTTP_SERVER_HANDLER_CONTINUE = 0, /**< Continue processing the request */
    ES_HTTP_SERVER_HANDLER_CLOSE = 1,    /**< Immediately close the socket */
    ES_HTTP_SERVER_HANDLER_SEND_RESPONSE = 2 /**< Send the response now */
  } Result;

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
   * @return a result code
   */
  virtual Result acceptConnection(HttpMultiplexer &multiplexer,
                                  ESB::SocketAddress *address) = 0;

  /**
   * Handle the beginning of a transaction.  This will be called 1+ times after
   * a connection has been accepted (>1 when connections are reused).  Put your
   * initialization code here.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The server stream, including request and response objects
   * @return a result code
   */
  virtual Result beginTransaction(HttpMultiplexer &multiplexer,
                                  HttpServerStream &stream) = 0;

  /**
   * Process a request's HTTP headers.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The server stream, including request and response objects
   * @return a result code
   */
  virtual Result receiveRequestHeaders(HttpMultiplexer &multiplexer,
                                       HttpServerStream &stream) = 0;

  /**
   * Reserve space for the request body.  This will be called 0+ times as the
   * request's HTTP body is received (0 times if the request has no body).
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The server stream, including request and response objects
   * @return The max bytes the handler can receive.  If the calling socket has
   * received more bytes than can be read, the calling socket will have to be
   * resumed by the handler later.
   */
  virtual ESB::UInt32 reserveRequestChunk(HttpMultiplexer &multiplexer,
                                          HttpServerStream &stream) = 0;

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
   * @return a result code
   */
  virtual Result receiveRequestChunk(HttpMultiplexer &multiplexer,
                                     HttpServerStream &stream,
                                     unsigned const char *chunk,
                                     ESB::UInt32 chunkSize) = 0;

  /**
   * If the handler cannot keep up with the received data, the multiplexer will
   * pause the receiving socket.   The handler should call resume() on the
   * paused stream once it's ready to receive more data.  Alternately the
   * handler can call close() on the paused stream.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The server stream, including request and response objects
   */
  virtual void receivePaused(HttpMultiplexer &multiplexer,
                             HttpServerStream &stream) = 0;

  /**
   * Request a buffer of up to n bytes to fill with response body data.  The
   * multiplexer will subsequently call fillResponseChunk with the actual size
   * of the buffer available.  The actual size of the buffer may be less than
   * the requested size.
   *
   * A good implementation will always return the known size of remaining body
   * data.  This function may be called multiple times before fillResponseChunk
   * actually writes the data to the buffer if there is insufficient space in
   * the underlying tcp buffers.  Don't deduct the amount requested from the
   * remaining amount until fillResponseChunk is called.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The server stream, including request and response objects
   * @return The buffer size requested.  Returning 0 ends the body.  Returning
   * -1 or less immediately closes the connection
   */
  virtual ESB::UInt32 reserveResponseChunk(HttpMultiplexer &multiplexer,
                                           HttpServerStream &stream) = 0;

  /**
   * Fill a response body chunk with data.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param stream The server stream, including request and response objects
   * @param chunk A buffer to fill
   * @param chunkSize The size of the buffer to fill.  This may be less than the
   * size requested by the requestResponseChunk method.
   */
  virtual void fillResponseChunk(HttpMultiplexer &multiplexer,
                                 HttpServerStream &stream, unsigned char *chunk,
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
