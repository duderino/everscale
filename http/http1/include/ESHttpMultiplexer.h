#ifndef ES_HTTP_MULTIPLEXER_H
#define ES_HTTP_MULTIPLEXER_H

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

namespace ES {

class HttpMultiplexer {
 public:
  HttpMultiplexer();
  virtual ~HttpMultiplexer();

  /**
   * Determine whether the multiplexer has shutdown.
   *
   * @return true if the multiplexer has shutdown, false otherwise.
   */
  virtual bool shutdown() = 0;

  virtual HttpClientTransaction *createClientTransaction() = 0;

  /**
   * Execute the client transaction.  If this method returns ESB_SUCCESS, then
   * the transaction will be cleaned up automatically after it finishes.  If
   * this method returns anything else then the caller should clean it up with
   * destroyClientTransaction
   *
   * @param transaction The transaction, with a populated HTTP request.  The
   * request will be sent to the destination returned by peerAddress().
   * @return ESB_SUCCESS if the transaction was successfully started, another
   * error code otherwise.  If error, cleanup the transaction with the
   * destroyTransaction function.
   */
  virtual ESB::Error executeClientTransaction(HttpClientTransaction *transaction) = 0;

  virtual void destroyClientTransaction(HttpClientTransaction *transaction) = 0;

 private:
  // Disabled
  HttpMultiplexer(const HttpMultiplexer &);
  HttpMultiplexer &operator=(const HttpMultiplexer &);
};

}  // namespace ES

#endif
