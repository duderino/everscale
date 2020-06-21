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
    ES_HTTP_CLIENT_HANDLER_BEGIN = 0,                 /**< Transaction starting */
    ES_HTTP_CLIENT_HANDLER_RESOLVE = 1,               /**< Trying to resolve client address */
    ES_HTTP_CLIENT_HANDLER_CONNECT = 2,               /**< Trying to connect to client */
    ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS = 3,  /**< Trying to send request headers */
    ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY = 4,     /**< Trying to send request body */
    ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS = 5, /**< Trying to receive/parse response headers */
    ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY = 6,    /**< Trying to receive response body */
    ES_HTTP_CLIENT_HANDLER_END = 7                    /**< Response received / transaction completed */
  } State;

  HttpClientHandler();

  virtual ~HttpClientHandler();

  /**
   * Process the HTTP headers of a received response.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param clientStream The client stream, including request and response objects
   * @return ESB_SUCCESS if successful, another error code otherwise.  Any
   * return value other than ESB_SUCCESS will abort the current transaction.
   */
  virtual ESB::Error receiveResponseHeaders(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) = 0;

  /**
   * Offer body bytes to the caller, if necessary producing more data as a
   * side-effect.  Return ESB_SUCCESS + 0 bytesAvailable to end the body. Return
   * ESB_AGAIN + 0 bytesAvailable to relay backpressure.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param clientStream The client stream, including request and response objects
   * @param bytesAvailable The bytes of body data that can be immediately
   * consumed.
   * @return ESB_SUCCESS if successful, another error code otherwise.  Any
   * return value other than ESB_SUCCESS and ESB_AGAIN will abort the current
   * transaction.
   */
  virtual ESB::Error offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                      ESB::UInt32 *bytesAvailable) = 0;
  /**
   * Copy body bytes to the caller, which consumes it in the process.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param clientStream The client stream, including request and response objects
   * @param body A buffer to fill with body data
   * @param bytesRequested Fill the buffer with this many bytes of body data.
   * bytesRequested will be <= result of offerRequestChunk().
   * @return ESB_SUCCESS if successful, another error code otherwise.  Any
   * return value other than ESB_SUCCESS will abort the current transaction.
   */
  virtual ESB::Error produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                        unsigned char *body, ESB::UInt32 bytesRequested) = 0;

  /**
   * Incrementally process a response body.  This will be called 1+ times as the
   * HTTP response body is received.  The body is finished when a chunk_size of
   * 0 is passed to the callback.  If there is no body, this callback will still
   * be called once with a chunk_size of 0.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param clientStream The client stream, including request and response objects
   * @param body A buffer to drain
   * @param bytesOffered The size of the buffer to drain, or 0 if the body is
   * finished.
   * @param bytesConsumed impl should write the number of bytes read from the
   * buffer here.  Must be <= chunkSize.
   * @return ESB_SUCCESS if successful, another error code otherwise.  Any
   * return value other than ESB_SUCCESS and ESB_AGAIN will abort the current
   * transaction.
   */
  virtual ESB::Error consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                         const unsigned char *body, ESB::UInt32 bytesOffered,
                                         ESB::UInt32 *bytesConsumed) = 0;

  /**
   * Handle the end of a request (request body has been fully sent)
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param clientStream The client stream, including request and response objects.
   * @return ESB_SUCCESS if successful, another error code otherwise.  Any
   * return value other than ESB_SUCCESS will abort the current transaction.
   */
  virtual ESB::Error endRequest(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) = 0;

  /**
   * Handle the end of a transaction.  This is called regardless of the
   * transaction's success or failure.
   *
   * @param multiplexer An API for the thread's multiplexer
   * @param clientStream The client stream, including request and response objects
   * @param state The state at which the transaction ended.
   * ES_HTTP_CLIENT_HANDLER_END means the transaction was successfully
   * completed, any other state indicates error - reason will be in the server
   * logs.
   */
  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream, State state) = 0;

 private:
  // Disabled
  HttpClientHandler(const HttpClientHandler &clientHandler);
  void operator=(const HttpClientHandler &clientHandler);
};

}  // namespace ES

#endif
