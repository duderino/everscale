#ifndef ES_HTTP_MULTIPLEXER_H
#define ES_HTTP_MULTIPLEXER_H

#ifndef ES_HTTP_SERVER_TRANSACTION_H
#include <ESHttpServerTransaction.h>
#endif

#ifndef ES_HTTP_SERVER_COUNTERS_H
#include <ESHttpServerCounters.h>
#endif

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#ifndef ESB_TCP_SOCKET_H
#include <ESBTCPSocket.h>
#endif

#ifndef ESB_LISTENING_TCP_SOCKET_H
#include <ESBListeningTCPSocket.h>
#endif

#ifndef ESB_SOCKET_MULTIPLEXER_H
#include <ESBSocketMultiplexer.h>
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

  /** Get a buffer suitable for i/o operations.
   */
  virtual ESB::Buffer *acquireBuffer() = 0;

  /**
   * Return an i/o buffer for later reuse.
   */
  virtual void releaseBuffer(ESB::Buffer *buffer) = 0;

  virtual HttpServerTransaction *createServerTransaction() = 0;

  virtual void destroyTransaction(HttpServerTransaction *transaction) = 0;

  virtual HttpServerCounters &serverCounters() = 0;

  /**
   * Construct a new server socket and immediately add it to a multiplexer.
   *
   * @param state The os-level socket state including a live file descriptor.
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual ESB::Error addServerSocket(ESB::TCPSocket::State &state) = 0;

  /**
   * Construct a new listening socket and immediately add it to the multiplexer
   *
   * @param socket A live TCP listening socket (post bind and post listen)
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual ESB::Error addListeningSocket(ESB::ListeningTCPSocket &socket) = 0;

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

  virtual ESB::SocketMultiplexer &multiplexer() = 0;

 private:
  // Disabled
  HttpMultiplexer(const HttpMultiplexer &);
  HttpMultiplexer &operator=(const HttpMultiplexer &);
};

}  // namespace ES

#endif
