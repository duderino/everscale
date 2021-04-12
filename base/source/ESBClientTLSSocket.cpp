#ifndef ESB_CLIENT_TLS_SOCKET_H
#include <ESBClientTLSSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#include <openssl/err.h>
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>

namespace ESB {

ClientTLSSocket::ClientTLSSocket(const char *fqdn, const SocketAddress &peerAddress, const char *namePrefix,
                                 TLSContextPointer &context, bool isBlocking)
    : TLSSocket(namePrefix, isBlocking),
      _peerAddress(peerAddress),
      _peerCertificate(),
      _context(context),
      _key(_peerAddress, SocketKey::TLS_OBJECT, this) {
  strncpy(_fqdn, fqdn, sizeof(_fqdn) - 1);
  _fqdn[sizeof(_fqdn) - 1] = 0;
}

ClientTLSSocket::~ClientTLSSocket() {}

const SocketAddress &ClientTLSSocket::peerAddress() const { return _peerAddress; }

Error ClientTLSSocket::peerCertificate(X509Certificate **cert) {
  if (!_peerCertificate.initialized()) {
    X509 *peer_certificate = SSL_get_peer_certificate(_ssl);
    if (!peer_certificate) {
      return ESB_CANNOT_FIND;
    }
    Error error = _peerCertificate.initialize(peer_certificate, true);
    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  *cert = &_peerCertificate;
  return ESB_SUCCESS;
}

const void *ClientTLSSocket::key() const { return &_key; }

Error ClientTLSSocket::startHandshake() {
  if (_context.isNull()) {
    return ESB_INVALID_STATE;
  }

  if (!_ssl) {
    _ssl = SSL_new(_context->rawContext());
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

    // This verifies the fqdn matches either the CN or the SANs.
    X509_VERIFY_PARAM *verifyParams = SSL_get0_param(_ssl);
    if (!X509_VERIFY_PARAM_set1_host(verifyParams, _fqdn, strlen(_fqdn))) {
      SSL_free(_ssl);  // this also frees _bio
      _bio = NULL;
      _ssl = NULL;
      ESB_LOG_TLS_ERROR("[%s] cannot enable cert verification for '%s'", name(), _fqdn);
      return ESB_GENERAL_TLS_ERROR;
    }

    if (_context->verifyPeerCertificate()) {
      SSL_set_verify(_ssl, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    }

    // Set SNI
    if (0 >= SSL_set_tlsext_host_name(_ssl, _fqdn)) {
      SSL_free(_ssl);  // this also frees _bio
      _bio = NULL;
      _ssl = NULL;
      ESB_LOG_TLS_ERROR("[%s] cannot set SNI for '%s'", name(), _fqdn);
      return ESB_GENERAL_TLS_ERROR;
    }
  }

  int ret = SSL_connect(_ssl);
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
        ESB_LOG_TLS_INFO("[%s] cannot complete client TLS handshake (%d)", name(), ret);
        return ESB_TLS_HANDSHAKE_ERROR;
    }
  }

  ESB_LOG_DEBUG("[%s] client TLS handshake successful", name());

  _flags |= ESB_TLS_FLAG_ESTABLISHED;
  _flags &= ~ESB_TLS_FLAG_WANT_READ;
  _flags &= ~ESB_TLS_FLAG_WANT_WRITE;

  return ESB_SUCCESS;
}

}  // namespace ESB
