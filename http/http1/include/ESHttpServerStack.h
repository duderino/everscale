#ifndef ES_HTTP_SERVER_STACK_H
#define ES_HTTP_SERVER_STACK_H

#ifndef ES_HTTP_SERVER_TRANSACTION_H
#include <ESHttpServerTransaction.h>
#endif

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#ifndef ESB_TCP_SOCKET_H
#include <ESBTCPSocket.h>
#endif

#ifndef ES_HTTP_SERVER_COUNTERS_H
#include <ESHttpServerCounters.h>
#endif

namespace ES {

class HttpServerStack {
 public:
  HttpServerStack();
  virtual ~HttpServerStack();

  // TODO this could be split into two separate interfaces, one for
  //  HttpServerSocket (needs create/destroyTransaction and
  //  acquire/releaseBuffer) and one for HttpServerHandler and
  //  HttpListeningSocket (needs addSocket).

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

  virtual HttpServerCounters &counters() = 0;

  /**
   * Construct a new server socket and immediately add it to a multiplexer.
   *
   * @param state The os-level socket state including a live file descriptor.
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual ESB::Error addServerSocket(ESB::TCPSocket::State &state) = 0;

 private:
  // Disabled
  HttpServerStack(const HttpServerStack &);
  HttpServerStack &operator=(const HttpServerStack &);
};

}  // namespace ES

#endif
