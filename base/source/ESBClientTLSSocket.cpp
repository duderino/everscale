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

SSL_CTX *ClientTLSSocket::_Context = NULL;

Error ClientTLSSocket::Initialize(const char *caCertificatePath, int maxVerifyDepth) {
  if (_Context) {
    return ESB_INVALID_STATE;
  }

  if (!caCertificatePath) {
    return ESB_NULL_POINTER;
  }

  if (0 >= maxVerifyDepth) {
    return ESB_INVALID_ARGUMENT;
  }

  Error error = TLSSocket::Initialize();
  if (ESB_SUCCESS != error) {
    return error;
  }

  _Context = SSL_CTX_new(TLS_method());

  if (!_Context) {
    ESB_LOG_TLS_ERROR("Cannot create TLS client context");
    return ESB_GENERAL_TLS_ERROR;
  }

  if (!SSL_CTX_load_verify_locations(_Context, caCertificatePath, NULL)) {
    ESB_LOG_TLS_ERROR("Cannot load CA certificate into client context");
    return ESB_GENERAL_TLS_ERROR;
  }

  SSL_CTX_set_verify_depth(_Context, maxVerifyDepth);

  return ESB_SUCCESS;
}

ClientTLSSocket::ClientTLSSocket(const HostAddress &peerAddress, const char *namePrefix, bool isBlocking)
    : TLSSocket(namePrefix, isBlocking), _peerAddress(peerAddress) {
}

ClientTLSSocket::~ClientTLSSocket() {}

const SocketAddress &ClientTLSSocket::peerAddress() const {
  return _peerAddress;
}

const void *ClientTLSSocket::key() const { return &_peerAddress; }

Error ClientTLSSocket::startHandshake() {
  if (!_Context) {
    return ESB_INVALID_STATE;
  }

  if (!_ssl) {
    _ssl = SSL_new(_Context);
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

    // This verifies the fqdn matches either the CN or the SANs.  X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS suppresses
    // support for "*" as wildcard pattern in labels that have a prefix or suffix, such as: "www*" or "*www"
    X509_VERIFY_PARAM *verifyParams = SSL_get0_param(_ssl);
    X509_VERIFY_PARAM_set_hostflags(verifyParams, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    if (!X509_VERIFY_PARAM_set1_host(verifyParams, _peerAddress.host(), strlen(_peerAddress.host()))) {
      SSL_free(_ssl);  // this also frees _bio
      _bio = NULL;
      _ssl = NULL;
      ESB_LOG_TLS_ERROR("[%s] cannot enable cert verification for '%s'", name(), _peerAddress.host());
      return ESB_GENERAL_TLS_ERROR;
    }
    SSL_set_verify(_ssl, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
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
        ESB_LOG_TLS_INFO("[%s] cannot complete client TLS handshake", name());
        return ESB_NON_RECOVERABLE_TLS_SESION_ERROR;
      default:
        ESB_LOG_TLS_ERROR("[%s] cannot complete client TLS handshake", name());
        return ESB_NON_RECOVERABLE_TLS_SESION_ERROR;
    }
  }

  // TODO check     SSL_get_verify_result() ?
  ESB_LOG_DEBUG("[%s] client TLS handshake successful", name());

  _flags |= ESB_TLS_FLAG_ESTABLISHED;
  _flags &= ~ESB_TLS_FLAG_WANT_READ;
  _flags &= ~ESB_TLS_FLAG_WANT_WRITE;

  return ESB_SUCCESS;
}

}  // namespace ESB
