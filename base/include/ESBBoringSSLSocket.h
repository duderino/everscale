#ifndef ESB_BORING_SSL_SOCKET_H
#define ESB_BORING_SSL_SOCKET_H

#ifndef ESB_CONNECTED_SOCKET_H
#include <ESBConnectedSocket.h>
#endif

namespace ESB {

/** BoringSSLSockets are used to connect to other SSL/TLS endpoints and
 *  send and receive data across the resulting channel.
 *
 *  @ingroup network
 */
class BoringSSLSocket : public ConnectedSocket {
 public:
  /** Construct a new server socket.  This instance's
   *  peer will be left uninitialized by this call.
   *
   *  @param isBlocking whether or not this socket is blocking.
   */
  BoringSSLSocket(const char *namePrefix, const char *nameSuffix, bool isBlocking = false);

  /** Construct a new client socket.  This instance will
   *  connect (attempt to connect) to the peer identified by the
   *  SocketAddress instance.
   *
   *  @param peer The peer that this socket will attempt to connect to.
   *  @param isBlocking whether or not this socket is blocking.
   */
  BoringSSLSocket(const char *namePrefix, const char *nameSuffix, const SocketAddress &peer, bool isBlocking = false);

  /** Destroy the connected socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~BoringSSLSocket();

  virtual bool secure();

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

 private:
  // Disabled
  BoringSSLSocket(const BoringSSLSocket &);
  BoringSSLSocket &operator=(const BoringSSLSocket &);
};

}  // namespace ESB

#endif
