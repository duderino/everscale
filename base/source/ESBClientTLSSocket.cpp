#ifndef ESB_CLIENT_TLS_SOCKET_H
#include <ESBClientTLSSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#include <openssl/err.h>
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>

//
// This code shares immutable SSL_CTX objects across threads without any locking (BoringSSL locks internally).
//
// This should be safe according to these comments on the threadsafety of SSL_CTX from one of the maintainers
// (davidben@davidben.net):
//
// From https://github.com/openssl/openssl/issues/2165#issuecomment-270007943
//
// An SSL_CTX may be used on multiple threads provided it is not reconfigured. (Even without threads, reconfiguring an
// SSL_CTX after calling SSL_new will behave weirdly in places.) Observe that the session cache is locked and
// everything. Also observe that an RSA object goes through a lot of trouble to work around RSA_new + setters (a
// better API pattern would be functions like RSA_new_private and RSA_new_public which take all their parameters in a
// single shot and then remove all RSA_set* functions) with BN_MONT_set_locked so that two threads may concurrently
// perform operations. This is, in part, so that two SSLs on different threads may sign with that shared key in the
// SSL_CTX.
//
// The API typically considers "logically immutable" use of an object to be safe across threads (otherwise there would
// be little point in even thread-safe refcounts, much less CRYPTO_THREAD_*_lock), but "logically mutable"
// reconfiguring an object to not be. Of course, the key bits here are "typically" and the scare quotes, so better
// documentation is probably worthwhile.
//
// A bit below that from https://github.com/openssl/openssl/issues/2165#issuecomment-270012533
//
// A thread may not reconfigure an SSL_CTX while another thread is accessing it.
//
// Even stronger, if there is an SSL attached to the SSL_CTX, reconfiguring the SSL_CTX should probably be undefined
// (except when documented otherwise). This is how BoringSSL is documented to behave. This is because some fields are
// copied from SSL_CTX to SSL while others are referenced directly off of SSL_CTX. Reconfiguring the SSL_CTX will
// behave differently depending on this. If you look at the set of APIs there are, it's not clear either
// interpretation makes particular sense as universal. I think it's best to just say you don't get to do that. Less
// combinatorial explosion of cases to test.
//

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

void ClientTLSSocket::Destroy() {
  if (!_Context) {
    return;
  }

  SSL_CTX_free(_Context);
  _Context = NULL;
}

ClientTLSSocket::ClientTLSSocket(const HostAddress &peerAddress, const char *namePrefix, bool isBlocking)
    : TLSSocket(namePrefix, isBlocking), _peerAddress(peerAddress) {}

ClientTLSSocket::~ClientTLSSocket() {}

const SocketAddress &ClientTLSSocket::peerAddress() const { return _peerAddress; }

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
