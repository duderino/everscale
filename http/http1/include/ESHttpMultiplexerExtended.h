#ifndef ES_HTTP_MULTIPLEXER_EXTENDED_H
#define ES_HTTP_MULTIPLEXER_EXTENDED_H

#ifndef ES_HTTP_MULTIPLEXER_H
#include <ESHttpMultiplexer.h>
#endif

#ifndef ES_HTTP_SERVER_TRANSACTION_H
#include <ESHttpServerTransaction.h>
#endif

#ifndef ES_HTTP_SERVER_COUNTERS_H
#include <ESHttpServerCounters.h>
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

class HttpMultiplexerExtended : public HttpMultiplexer {
 public:
  HttpMultiplexerExtended();
  virtual ~HttpMultiplexerExtended();

  /** Get a buffer suitable for i/o operations.
   */
  virtual ESB::Buffer *acquireBuffer() = 0;

  /**
   * Return an i/o buffer for later reuse.
   */
  virtual void releaseBuffer(ESB::Buffer *buffer) = 0;

  virtual HttpServerTransaction *createServerTransaction() = 0;

  virtual void destroyServerTransaction(HttpServerTransaction *transaction) = 0;

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

  virtual ESB::SocketMultiplexer &multiplexer() = 0;

 private:
  // Disabled
  HttpMultiplexerExtended(const HttpMultiplexerExtended &);
  HttpMultiplexerExtended &operator=(const HttpMultiplexerExtended &);
};

}  // namespace ES

#endif
