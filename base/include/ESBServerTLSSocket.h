#ifndef ESB_SERVER_TLS_SOCKET_H
#define ESB_SERVER_TLS_SOCKET_H

#ifndef ESB_TLS_SOCKET_H
#include <ESBTLSSocket.h>
#endif

namespace ESB {

/** ServerTLSSocket are used to connect to other SSL/TLS endpoints and
 *  send and receive data across the resulting channel.
 *
 *  @ingroup network
 */
class ServerTLSSocket : public TLSSocket {
 public:
  static Error Initialize(const char *privateKeyPath, const char *certificatePath);

  static void Destroy();

  /** Construct a new server socket.
   *
   * @param acceptState init parameters created by the ListeningSocket
   * @param namePrefix A name prefix to be incorporated into log messages
   */
  ServerTLSSocket(const Socket::State &acceptState, const char *namePrefix);

  /** Destroy the socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~ServerTLSSocket();

  virtual const SocketAddress &peerAddress() const;
  virtual const void *key() const;

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param memory The object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::EmbeddedListElement *memory) noexcept { return memory; }

 protected:
  virtual Error startHandshake();

 private:
  // Disabled
  ServerTLSSocket(const ServerTLSSocket &);
  ServerTLSSocket &operator=(const ServerTLSSocket &);

  SocketAddress _peerAddress;

  static SSL_CTX *_Context;
};

}  // namespace ESB

#endif
