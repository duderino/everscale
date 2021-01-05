#ifndef ESB_CLIENT_TLS_SOCKET_H
#define ESB_CLIENT_TLS_SOCKET_H

#ifndef ESB_TLS_SOCKET_H
#include <ESBTLSSocket.h>
#endif

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_HOST_ADDRESS_H
#include <ESBHostAddress.h>
#endif

namespace ESB {

/** ClientTLSSockets are client TLS connections
 *
 *  @ingroup network
 */
class ClientTLSSocket : public TLSSocket {
 public:
  static Error Initialize(const char *caCertificatePath, int maxVerifyDepth);

  /** Construct a new client TLS Connection
   *
   *  @param peerAddress The socket will try to connect to this TLS peer
   *  @param isBlocking whether or not this socket is blocking.
   */
  ClientTLSSocket(const HostAddress &peerAddress, const char *namePrefix,
                  bool isBlocking = false);

  /** Destroy the connected socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~ClientTLSSocket();

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
  ClientTLSSocket(const ClientTLSSocket &);
  ClientTLSSocket &operator=(const ClientTLSSocket &);

  static SSL_CTX *_Context;
  HostAddress _peerAddress;
};

}  // namespace ESB

#endif
