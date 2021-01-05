#ifndef ESB_SERVER_TLS_SOCKET_H
#include <ESBServerTLSSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#include <openssl/err.h>

namespace ESB {

SSL_CTX *ServerTLSSocket::_Context = NULL;

Error ServerTLSSocket::Initialize(const char *privateKeyPath, const char *certificatePath) {
  if (_Context) {
    return ESB_INVALID_STATE;
  }

  if (!privateKeyPath || !certificatePath) {
    return ESB_NULL_POINTER;
  }

  Error error = TLSSocket::Initialize();
  if (ESB_SUCCESS != error) {
    return error;
  }

  _Context = SSL_CTX_new(TLS_method());

  if (!_Context) {
    ESB_LOG_TLS_ERROR("Cannot create TLS server context");
    return ESB_GENERAL_TLS_ERROR;
  }

  if (!SSL_CTX_use_PrivateKey_file(_Context, privateKeyPath, SSL_FILETYPE_PEM)) {
    ESB_LOG_TLS_ERROR("Cannot load private key into server context");
    return ESB_GENERAL_TLS_ERROR;
  }

  ERR_clear_error();
  if (!SSL_CTX_use_certificate_file(_Context, certificatePath, SSL_FILETYPE_PEM)) {
    //ERR_print_errors_fp(stdout);
    ESB_LOG_TLS_ERROR("Cannot load certificate into server context");
    return ESB_GENERAL_TLS_ERROR;
  }

  // TODO use SSL_CTX_set_client_CA_list for mTLS

  return ESB_SUCCESS;
}

ServerTLSSocket::ServerTLSSocket(const Socket::State &acceptState, const char *namePrefix)
    : TLSSocket(acceptState, namePrefix), _peerAddress(acceptState.peerAddress()) {}

ServerTLSSocket::~ServerTLSSocket() {}

const SocketAddress &ServerTLSSocket::peerAddress() const {
  return _peerAddress;
}
const void *ServerTLSSocket::key() const { return &_peerAddress; }

Error ServerTLSSocket::startHandshake() {
  if (!_Context) {
    return ESB_INVALID_STATE;
  }

  if (!_ssl) {
    _ssl = SSL_new(_Context);
    if (!_ssl) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] cannot create SSL context",
                          name());
      return ESB_OUT_OF_MEMORY;
    }

    _bio = BIO_new_socket(_sockFd, BIO_NOCLOSE);
    if (!_bio) {
      SSL_free(_ssl);
      _ssl = NULL;
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] cannot create SSL BIO",
                          name());
      return ESB_OUT_OF_MEMORY;
    }
    SSL_set_bio(_ssl, _bio, _bio);

    // TODO setup mTLS verification
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
        ESB_LOG_TLS_INFO("[%s] cannot complete server TLS handshake", name());
        return ESB_NON_RECOVERABLE_TLS_SESION_ERROR;
      default:
        ESB_LOG_TLS_ERROR("[%s] cannot complete server TLS handshake", name());
        return ESB_NON_RECOVERABLE_TLS_SESION_ERROR;
    }
  }

  ESB_LOG_DEBUG("[%s] server TLS handshake successful", name());

  _flags |= ESB_TLS_FLAG_ESTABLISHED;
  _flags &= ~ESB_TLS_FLAG_WANT_READ;
  _flags &= ~ESB_TLS_FLAG_WANT_WRITE;

  return ESB_SUCCESS;
}

}  // namespace ESB
