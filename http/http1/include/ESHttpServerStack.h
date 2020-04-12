#ifndef ES_HTTP_SERVER_STACK_H
#define ES_HTTP_SERVER_STACK_H

#ifndef ES_HTTP_SERVER_TRANSACTION_H
#include <ESHttpServerTransaction.h>
#endif

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

namespace ES {

class HttpServerStack {
 public:
  HttpServerStack();
  virtual ~HttpServerStack();

  virtual bool isRunning() = 0;

  virtual HttpServerTransaction *createTransaction() = 0;

  virtual void destroyTransaction(HttpServerTransaction *transaction) = 0;

  /** Get a buffer suitable for i/o operations.
   */
  virtual ESB::Buffer *acquireBuffer() = 0;

  /**
   * Return an i/o buffer for later reuse.
   */
  virtual void releaseBuffer(ESB::Buffer *buffer) = 0;

 private:
  // Disabled
  HttpServerStack(const HttpServerStack &);
  HttpServerStack &operator=(const HttpServerStack &);
};

}  // namespace ES

#endif
