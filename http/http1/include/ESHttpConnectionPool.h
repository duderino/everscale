#ifndef ES_HTTP_CONNECTION_POOL_H
#define ES_HTTP_CONNECTION_POOL_H

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

#ifndef ES_HTTP_CLIENT_HANDLER_H
#include <ESHttpClientHandler.h>
#endif

namespace ES {

class HttpConnectionPool {
 public:
  HttpConnectionPool();

  virtual ~HttpConnectionPool();

  /**
   * Create a new client transaction
   *
   * @param handler The handler
   * @return a new client transaction if successful, null otherwise
   */
  virtual HttpClientTransaction *createClientTransaction(
      HttpClientHandler *clientHandler) = 0;

  /**
   * Execute the client transaction.  If this method returns ESB_SUCCESS, then
   * the transaction will be cleaned up automatically after it finishes.  If
   * this method returns anything else then the caller should clean it up with
   * destroyClientTransaction
   *
   * @param transaction The transaction
   * @return ESB_SUCCESS if the transaction was successfully started, another
   * error code otherwise.  If error, cleanup the transaction with the
   * destroyClientTransaction method.
   */
  virtual ESB::Error executeClientTransaction(
      HttpClientTransaction *transaction) = 0;

  /**
   * Cleanup the client transaction.  Note that this will not free any
   * app-specific context.  Call this only if executeClientTransaction doesn't
   * return ESB_SUCCESS
   *
   * @param transaction The transaction to cleanup.
   */
  virtual void destroyClientTransaction(HttpClientTransaction *transaction) = 0;

 private:
  // disabled
  HttpConnectionPool(const HttpConnectionPool &);
  void operator=(const HttpConnectionPool &);
};

}  // namespace ES

#endif
