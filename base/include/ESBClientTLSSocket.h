#ifndef ESB_CLIENT_TLS_SOCKET_H
#define ESB_CLIENT_TLS_SOCKET_H

#ifndef ESB_TLS_SOCKET_H
#include <ESBTLSSocket.h>
#endif

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_TLS_CONTEXT_H
#include <ESBTLSContext.h>
#endif

namespace ESB {

/** ClientTLSSockets are client TLS connections
 *
 *  @ingroup network
 */
class ClientTLSSocket : public TLSSocket {
 public:
  /** Construct a new client TLS Connection
   *
   *  @param fqdn The DNS name of the peer
   *  @param peerAddress The socket will try to connect to this TLS peer
   *  @param namePrefix prefix to be added to log messages relevant to this socket
   *  @param context TLS context (private key, certificates, etc) for the connection
   *  @param isBlocking whether or not this socket is blocking.
   */
  ClientTLSSocket(const char *fqdn, const SocketAddress &peerAddress, const char *namePrefix,
                  TLSContextPointer &context, bool isBlocking = false);

  /** Destroy the connected socket.  Will close the socket if it has not
   *  already been closed.
   */
  virtual ~ClientTLSSocket();

  inline const char *fqdn() { return _fqdn; }

  virtual const SocketAddress &peerAddress() const;

  virtual const void *key() const;

  /**
   * Get the client TLS context
   *
   * @return The client TLS context
   */
  inline TLSContextPointer &context() { return _context; }

  /**
   * Get the peer/server's certificate
   *
   * @param cert will point to the peer certificate on success
   * @return ESB_SUCCESS if succesful, ESB_CANNOT_FIND if the peer has not (yet) sent its certificate, another error
   * code otherwise.
   */
  Error peerCertificate(X509Certificate **cert);

 protected:
  virtual Error startHandshake();

 private:
  SocketAddress _peerAddress;
  X509Certificate _peerCertificate;
  TLSContextPointer _context;
  SocketKey _key;
  char _fqdn[ESB_MAX_HOSTNAME + 1];

  ESB_DEFAULT_FUNCS(ClientTLSSocket);
};

}  // namespace ESB

#endif
