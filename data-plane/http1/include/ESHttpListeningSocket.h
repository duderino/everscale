#ifndef ES_HTTP_LISTENING_SOCKET_H
#define ES_HTTP_LISTENING_SOCKET_H

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

#ifndef ESB_LISTENING_SOCKET_H
#include <ESBListeningSocket.h>
#endif

#ifndef ES_HTTP_MULTIPLEXER_EXTENDED_H
#include <ESHttpMultiplexerExtended.h>
#endif

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

namespace ES {

/** A listening socket that creates HttpServerSockets
 */
class HttpListeningSocket : public ESB::MultiplexedSocket {
 public:
  /** Constructor
   */
  HttpListeningSocket(HttpMultiplexerExtended &stack, HttpServerHandler &handler, ESB::CleanupHandler &cleanupHandler);

  /** Destructor.
   */
  virtual ~HttpListeningSocket();

  //
  // ESB::EmbeddedMapElement
  //

  virtual const void *key() const;

  //
  // ESB::MultiplexedSocket
  //

  virtual bool permanent();

  virtual bool wantAccept();

  virtual bool wantConnect();

  virtual bool wantRead();

  virtual bool wantWrite();

  virtual ESB::Error handleAccept();

  virtual ESB::Error handleConnect();

  virtual ESB::Error handleReadable();

  virtual ESB::Error handleWritable();

  virtual void handleError(ESB::Error errorCode);

  virtual void handleRemoteClose();

  virtual void handleIdle();

  virtual void handleRemove();

  virtual SOCKET socketDescriptor() const;

  virtual ESB::CleanupHandler *cleanupHandler();

  virtual const char *name() const;

  virtual void markDead();

  virtual bool dead() const;

  //
  // Local
  //

  ESB::Error initialize(ESB::ListeningSocket &socket);

 private:
  ESB::ListeningSocket _socket;
  HttpMultiplexerExtended &_multiplexer;
  HttpServerHandler &_handler;
  ESB::CleanupHandler &_cleanupHandler;
  bool _dead;

  ESB_DEFAULT_FUNCS(HttpListeningSocket);
};

}  // namespace ES

#endif /* ! ES_HTTP_LISTENING_SOCKET_H */
