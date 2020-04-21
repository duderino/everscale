#ifndef ES_HTTP_CLIENT_STACK_H
#define ES_HTTP_CLIENT_STACK_H

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

namespace ES {

class HttpClientStack {
 public:
  HttpClientStack();
  virtual ~HttpClientStack();

  virtual bool isRunning() = 0;

  virtual HttpClientTransaction *createClientTransaction() = 0;

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
  virtual ESB::Error executeTransaction(HttpClientTransaction *transaction) = 0;

  virtual void destroyTransaction(HttpClientTransaction *transaction) = 0;

  /** Get a buffer suitable for i/o operations.
   */
  virtual ESB::Buffer *acquireBuffer() = 0;

  /**
   * Return an i/o buffer for later reuse.
   */
  virtual void releaseBuffer(ESB::Buffer *buffer) = 0;

 private:
  // Disabled
  HttpClientStack(const HttpClientStack &);
  HttpClientStack &operator=(const HttpClientStack &);
};

}  // namespace ES

#endif
