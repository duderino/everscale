#ifndef ESB_SERVER_TLS_SOCKET_H
#include <ESBServerTLSSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#include <openssl/err.h>

namespace ESB {

ServerTLSSocket::ServerTLSSocket(const Socket::State &acceptState, const char *namePrefix,
                                 ServerTLSContextIndex &contextIndex)
    : TLSSocket(acceptState, namePrefix), _contextIndex(contextIndex), _peerAddress(acceptState.peerAddress()) {}

ServerTLSSocket::~ServerTLSSocket() {}

const SocketAddress &ServerTLSSocket::peerAddress() const { return _peerAddress; }
const void *ServerTLSSocket::key() const { return &_peerAddress; }

Error ServerTLSSocket::startHandshake() {
  if (!_ssl) {
    _ssl = SSL_new(_contextIndex.defaultContext()->rawContext());
    if (!_ssl) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] cannot create SSL context", name());
      return ESB_OUT_OF_MEMORY;
    }

    _bio = BIO_new_socket(_sockFd, BIO_NOCLOSE);
    if (!_bio) {
      SSL_free(_ssl);
      _ssl = NULL;
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] cannot create SSL BIO", name());
      return ESB_OUT_OF_MEMORY;
    }
    SSL_set_bio(_ssl, _bio, _bio);

    if (_contextIndex.defaultContext()->verifyPeerCertificate()) {
      SSL_set_verify(_ssl, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    } else {
      SSL_set_verify(_ssl, SSL_VERIFY_NONE, NULL);
    }
  }

  int ret = SSL_accept(_ssl);
  if (0 >= ret) {
    switch (ret = SSL_get_error(_ssl, ret)) {
      case SSL_ERROR_WANT_READ:
        _flags |= ESB_TLS_FLAG_WANT_READ;
        return ESB_AGAIN;
      case SSL_ERROR_WANT_WRITE:
        _flags |= ESB_TLS_FLAG_WANT_WRITE;
        return ESB_AGAIN;
      case SSL_ERROR_WANT_CONNECT:
        assert(!"SSL_ERROR_WANT_CONNECT but already connected");
        // fall through
      case SSL_ERROR_ZERO_RETURN:
      case SSL_ERROR_SYSCALL:
      case SSL_ERROR_SSL:
      default:
        ESB_LOG_TLS_INFO("[%s] cannot complete server TLS handshake (%d)", name(), ret);
        return ESB_TLS_HANDSHAKE_ERROR;
    }
  }

  ESB_LOG_DEBUG("[%s] server TLS handshake successful", name());

  _flags |= ESB_TLS_FLAG_ESTABLISHED;
  _flags &= ~ESB_TLS_FLAG_WANT_READ;
  _flags &= ~ESB_TLS_FLAG_WANT_WRITE;

  return ESB_SUCCESS;
}

}  // namespace ESB
