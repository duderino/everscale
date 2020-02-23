#ifndef ES_HTTP_SERVER_HANDLER_H
#define ES_HTTP_SERVER_HANDLER_H

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ES_HTTP_TRANSACTION_H
#include <ESHttpTransaction.h>
#endif

namespace ES {

class HttpServerHandler {
 public:
  typedef enum {
    ES_HTTP_SERVER_HANDLER_CONTINUE = 0, /**< Continue processing the request */
    ES_HTTP_SERVER_HANDLER_CLOSE =
        1, /**< Immediately close the socket / abort the request */
    ES_HTTP_SERVER_HANDLER_SEND_RESPONSE =
        2 /**< Immediately send the response / finish the request */
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
   * @param address The IP address and port of the client.
   * @return a result code
   */
  virtual Result acceptConnection(ESB::SocketAddress *address) = 0;

  /**
   * Handle the beginning of a transaction.  This will be called 1+ times after
   * a connection has been accepted (>1 when connections are reused).  Put your
   * initialization code here.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc
   * @return a result code
   */
  virtual Result begin(HttpTransaction *transaction) = 0;

  /**
   * Process a request's HTTP headers.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc.
   * @return a result code
   */
  virtual Result receiveRequestHeaders(HttpTransaction *transaction) = 0;

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
  virtual Result receiveRequestBody(HttpTransaction *transaction,
                                    unsigned const char *chunk,
                                    unsigned int chunkSize) = 0;

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
  virtual int reserveResponseChunk(HttpTransaction *transaction) = 0;

  /**
   * Fill a response body chunk with data.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc.
   * @param chunk A buffer to fill
   * @param chunkSize The size of the buffer to fill.  This may be less than the
   * size requested by the requestResponseChunk method.
   */
  virtual void fillResponseChunk(HttpTransaction *transaction,
                                 unsigned char *chunk,
                                 unsigned int chunkSize) = 0;

  /**
   * Handle the end of a transaction.  This is called regardless of the
   * transaction's success or failure.  Put your cleanup code here.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc
   * @param state The state at which the transaction ended.
   * ES_HTTP_SERVER_HANDLER_END means the transaction was successfully
   * completed, any other state indicates error - reason will be in the server
   * logs.
   */
  virtual void end(HttpTransaction *transaction, State state) = 0;

 private:
  // Disabled
  HttpServerHandler(const HttpServerHandler &serverHandler);
  void operator=(const HttpServerHandler &serverHandler);
};

}  // namespace ES

#endif
