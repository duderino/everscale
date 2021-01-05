#ifndef ESB_CLEAR_SOCKET_H
#define ESB_CLEAR_SOCKET_H

#ifndef ESB_CONNECTED_SOCKET_H
#include <ESBConnectedSocket.h>
#endif

namespace ESB {

/** ClearSocket is an unencrypted TCP connection
 *
 *  @ingroup network
 */
class ClearSocket : public ConnectedSocket {
 public:
  /** Construct a new server ClearSocket.
   *
   * @param acceptState init parameters created by the ListeningSocket
   * @param namePrefix A name prefix to be incorporated into log messages
   */
  ClearSocket(const Socket::State &acceptState, const char *namePrefix);

  /** Construct a new client ClearSocket.  This instance will immediately connect to the peer.
   *
   * @param peer The peer that this socket will attempt to connect to.
   * @param namePrefix A name prefix to be incorporated into log messages
   * @param isBlocking whether or not this socket is blocking.
   */
  ClearSocket(const SocketAddress &peer, const char *namePrefix, bool isBlocking = false);

  /** Destroy the socket, closing it if it has not already been closed.
   */
  virtual ~ClearSocket();

  virtual const void *key() const;

  virtual const SocketAddress &peerAddress() const;

  virtual bool secure() const;

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
  ClearSocket(const ClearSocket &);
  ClearSocket &operator=(const ClearSocket &);

  SocketAddress _peerAddress;
};

}  // namespace ESB

#endif
