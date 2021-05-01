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
    ES_HTTP_SERVER_HANDLER_BEGIN = 0,                 /**< Connection accepted or reused */
    ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS = 1,  /**< Trying to parse request's http headers */
    ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY = 2,     /**< Trying to read request's http body */
    ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS = 3, /**< Trying to format and send response's http headers */
    ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY = 4,    /**< Trying to send response's http body */
    ES_HTTP_SERVER_HANDLER_END = 5                    /**< Response sent / transaction completed */
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
  virtual ESB::Error acceptConnection(HttpMultiplexer &multiplexer, ESB::SocketAddress *address) = 0;

  /**
   * Handle the beginning of a transaction.  This will be called 1+ times after
   * a connection has been accepted (>1 when connections are reused).  Put your
   * initialization code here.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param serverStream The server stream, including request and response objects
   * @return ESB_SUCCESS to continue processing, ESB_SEND_RESPONSE to send
   *  a HTTP response now, or any other value to immediately close the socket.
   */
  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream) = 0;

  /**
   * Process a request's HTTP headers.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param serverStream The server stream, including request and response objects
   * @return ESB_SUCCESS to continue processing, ESB_SEND_RESPONSE to send
   *  a HTTP response now, or any other value to immediately close the socket.
   */
  virtual ESB::Error receiveRequestHeaders(HttpMultiplexer &multiplexer, HttpServerStream &serverStream) = 0;

  /**
   * Incrementally process a request's body.  This will be called 1+ times as
   * the request's HTTP body is received.  The body is finished when a
   * chunk_size of 0 is passed to the callback.  If there is no body, this
   * callback will still be called once with a chunk_size of 0.  The HTTP
   * response object must be populated before the chunk_size 0 call returns.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param serverStream The server stream, including request and response objects
   * @param body A buffer to drain
   * @param bytesOffered The size of the buffer to drain, or 0 if the body is
   * finished.
   * @param bytesConsumed impl should write the number of bytes read from the
   * buffer here.  Must be <= chunkSize.
   * @return ESB_SUCCESS if successful, another error code otherwise.  Any
   * return value other than ESB_SUCCESS and ESB_AGAIN will abort the current
   * transaction.
   */
  virtual ESB::Error consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                        unsigned const char *body, ESB::UInt64 bytesOffered,
                                        ESB::UInt64 *bytesConsumed) = 0;

  /**
   * Offer body bytes to the caller, if necessary producing more data as a
   * side-effect.  Return ESB_SUCCESS + 0 bytesAvailable to end the body. Return
   * ESB_AGAIN + 0 bytesAvailable to relay backpressure.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param serverStream The server stream, including request and response objects
   * @param bytesAvailable The bytes of body data that can be immediately
   * consumed.
   * @return ESB_SUCCESS if successful, another error code otherwise.  Any
   * return value other than ESB_SUCCESS and ESB_AGAIN will abort the current
   * transaction.
   */
  virtual ESB::Error offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                       ESB::UInt64 *bytesAvailable) = 0;
  /**
   * Copy body bytes to the caller, which consumes it in the process.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param serverStream The server stream, including request and response objects
   * @param body A buffer to fill with body data
   * @param bytesRequested Fill the buffer with this many bytes of body data.
   * bytesRequested will be <= result of offerResponseChunk().
   * @return ESB_SUCCESS if successful, another error code otherwise.  Any
   * return value other than ESB_SUCCESS will abort the current transaction.
   */
  virtual ESB::Error produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                         unsigned char *body, ESB::UInt64 bytesRequested) = 0;

  /**
   * Handle the end of a transaction.  This is called regardless of the
   * transaction's success or failure.  Put your cleanup code here.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param serverStream The server stream, including request and response objects
   * @param state The state at which the transaction ended.
   * ES_HTTP_SERVER_HANDLER_END means the transaction was successfully
   * completed, any other state indicates error - reason will be in the server
   * logs.
   */
  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream, State state) = 0;

 private:
  // Disabled
  HttpServerHandler(const HttpServerHandler &serverHandler);
  void operator=(const HttpServerHandler &serverHandler);
};

}  // namespace ES

#endif
