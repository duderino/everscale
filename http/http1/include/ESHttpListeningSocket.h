#ifndef ES_HTTP_LISTENING_SOCKET_H
#define ES_HTTP_LISTENING_SOCKET_H

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

#ifndef ESB_LISTENING_TCP_SOCKET_H
#include <ESBListeningTCPSocket.h>
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
  // ESB::MultiplexedSocket
  //

  virtual bool wantAccept();

  virtual bool wantConnect();

  virtual bool wantRead();

  virtual bool wantWrite();

  virtual bool isIdle();

  virtual ESB::Error handleAccept();

  virtual ESB::Error handleConnect();

  virtual ESB::Error handleReadable();

  virtual ESB::Error handleWritable();

  virtual bool handleError(ESB::Error errorCode);

  virtual bool handleRemoteClose();

  virtual bool handleIdle();

  virtual bool handleRemove();

  virtual SOCKET socketDescriptor() const;

  virtual ESB::CleanupHandler *cleanupHandler();

  virtual const char *name() const;

  ESB::Error initialize(ESB::ListeningTCPSocket &socket);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  // Disabled
  HttpListeningSocket(const HttpListeningSocket &);
  HttpListeningSocket &operator=(const HttpListeningSocket &);

  ESB::ListeningTCPSocket _socket;
  HttpMultiplexerExtended &_multiplexer;
  HttpServerHandler &_handler;
  ESB::CleanupHandler &_cleanupHandler;
};

}  // namespace ES

#endif /* ! ES_HTTP_LISTENING_SOCKET_H */
