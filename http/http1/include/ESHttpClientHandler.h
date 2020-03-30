#ifndef ES_HTTP_CLIENT_HANDLER_H
#define ES_HTTP_CLIENT_HANDLER_H

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

namespace ES {

class HttpClientStack {
 public:
  HttpClientStack();
  virtual ~HttpClientStack();

  virtual HttpClientTransaction *createTransaction() = 0;

  virtual bool isRunning() = 0;

  /**
   * Execute the client transaction.  If this method returns ESB_SUCCESS, then
   * the transaction will be cleaned up automatically after it finishes.  If
   * this method returns anything else then the caller should clean it up with
   * destroyClientTransaction
   *
   * @param transaction The transaction
   * @return ESB_SUCCESS if the transaction was successfully started, another
   * error code otherwise.  If error, cleanup the transaction with the
   * destroyTransaction function.
   */
  virtual ESB::Error executeClientTransaction(
      HttpClientTransaction *transaction) = 0;

  virtual void destroyTransaction(HttpClientTransaction *transaction) = 0;

 private:
  // Disabled
  HttpClientStack(const HttpClientStack &);
  HttpClientStack &operator=(const HttpClientStack &);
};

class HttpClientHandler {
 public:
  typedef enum {
    ES_HTTP_CLIENT_HANDLER_CONTINUE = 0, /**< Continue processing the request */
    ES_HTTP_CLIENT_HANDLER_CLOSE =
        1 /**< Immediately close the socket / abort the request */
  } Result;

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
   * Request a buffer of up to n bytes to fill with request body data.  The
   * stack will subsequently call fillRequestChunk with the actual size of the
   * buffer available.  The actual size of the buffer may be less than the
   * requested size.
   *
   * A good implementation will always return the known size of remaining body
   * data.  This function may be called multiple times before fillRequestChunk
   * actually writes the data to the buffer if there is insufficient space in
   * the underlying tcp buffers.  Don't deduct the amount requested from the
   * remaining amount until fillRequestChunk is called.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc.
   * @return The buffer size requested.  Returning 0 ends the body.  Returning
   * -1 or less immediately closes the connection
   */
  virtual int reserveRequestChunk(HttpClientStack &stack,
                                  HttpClientTransaction *transaction) = 0;

  /**
   * Fill a request body chunk with data.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc.
   * @param chunk A buffer to fill
   * @param chunkSize The size of the buffer to fill.  This may be less than the
   * size requested by the requestRequestChunk method.
   */
  virtual void fillRequestChunk(HttpClientStack &stack,
                                HttpClientTransaction *transaction,
                                unsigned char *chunk,
                                unsigned int chunkSize) = 0;

  /**
   * Process a request's HTTP headers.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc.
   * @return a result code
   */
  virtual Result receiveResponseHeaders(HttpClientStack &stack,
                                        HttpClientTransaction *transaction) = 0;

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
  virtual Result receiveResponseBody(HttpClientStack &stack,
                                     HttpClientTransaction *transaction,
                                     unsigned const char *chunk,
                                     unsigned int chunkSize) = 0;

  /**
   * Handle the end of a transaction.  This is called regardless of the
   * transaction's success or failure.  Put your cleanup code here.
   *
   * @param transaction The http transaction - contains request and response
   * objects, etc
   * @param state The state at which the transaction ended.
   * ES_HTTP_CLIENT_HANDLER_END means the transaction was successfully
   * completed, any other state indicates error - reason will be in the server
   * logs.
   */
  virtual void endClientTransaction(HttpClientStack &stack,
                                    HttpClientTransaction *transaction,
                                    State state) = 0;

 private:
  // Disabled
  HttpClientHandler(const HttpClientHandler &clientHandler);
  void operator=(const HttpClientHandler &clientHandler);
};

}  // namespace ES

#endif
