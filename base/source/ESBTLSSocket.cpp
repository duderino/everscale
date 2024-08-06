#ifndef ESB_TLS_SOCKET_H
#include <ESBTLSSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include <openssl/err.h>
#include <openssl/ssl.h>

namespace ESB {

class TLSInitializer {
 public:
  TLSInitializer() {
    if (1 != OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, NULL)) {
      fprintf(stderr, "Cannot initialize openssl library\n");
      exit(1);
    }
  }
};

static TLSInitializer Initializer;

TLSSocket::TLSSocket(const Socket::State &acceptState, const char *namePrefix)
    : ConnectedSocket(acceptState, namePrefix), _ssl(NULL), _bio(NULL) {}

TLSSocket::TLSSocket(const char *namePrefix, bool isBlocking)
    : ConnectedSocket(namePrefix, isBlocking), _ssl(NULL), _bio(NULL) {}

TLSSocket::~TLSSocket() { close(); }

bool TLSSocket::secure() const { return true; }

void TLSSocket::close() {
  if (_ssl) {
    if (_flags & ESB_TLS_FLAG_ESTABLISHED) {
      int ret = SSL_shutdown(_ssl);
      if (0 > ret) {
        ESB_LOG_TLS_INFO("[%s] cannot shutdown TLS session", name());
      }
    }
    SSL_free(_ssl);  // also frees _bio
    _ssl = NULL;
    _bio = NULL;
  } else if (_bio) {
    BIO_free(_bio);
    _bio = NULL;
  }

  _flags &= ~ESB_TLS_FLAG_ALL;
  ConnectedSocket::close();
}

SSize TLSSocket::receive(char *buffer, Size bufferSize) {
  assert(!(ESB_TLS_FLAG_DEAD & _flags));
  if (ESB_TLS_FLAG_DEAD & _flags) {
    errno = ESB_INVALID_STATE;
    return -1;
  }

  if (!(ESB_TLS_FLAG_ESTABLISHED & _flags)) {
    Error error = startHandshake();
    if (ESB_SUCCESS != error) {
      errno = error;
      return -1;
    }
  }

  assert(ESB_TLS_FLAG_ESTABLISHED & _flags);

  int ret = SSL_read(_ssl, buffer, bufferSize);
  if (0 < ret) {
    ESB_LOG_DEBUG("[%s] read %d TLS bytes", name(), ret);
    return ret;
  }

  switch (ret = SSL_get_error(_ssl, ret)) {
    case SSL_ERROR_ZERO_RETURN:
      ESB_LOG_DEBUG("[%s] peer closed TLS connection", name());
      return 0;
    case SSL_ERROR_WANT_READ:
      errno = ESB_AGAIN;
      return -1;
    case SSL_ERROR_SYSCALL:
    case SSL_ERROR_SSL:
    default:
      _flags &= ~ESB_TLS_FLAG_ALL;
      _flags |= ESB_TLS_FLAG_DEAD;
      ESB_LOG_TLS_INFO("[%s] cannot read TLS bytes (%d)", name(), ret);
      errno = ESB_TLS_SESSION_ERROR;
      return -1;
  }
}

SSize TLSSocket::send(const char *buffer, Size bufferSize) {
  assert(!(ESB_TLS_FLAG_DEAD & _flags));
  if (ESB_TLS_FLAG_DEAD & _flags) {
    errno = ESB_INVALID_STATE;
    return -1;
  }

  if (!(ESB_TLS_FLAG_ESTABLISHED & _flags)) {
    Error error = startHandshake();
    if (ESB_SUCCESS != error) {
      errno = error;
      return -1;
    }
  }

  assert(ESB_TLS_FLAG_ESTABLISHED & _flags);

  int ret = SSL_write(_ssl, buffer, bufferSize);
  if (0 < ret) {
    ESB_LOG_DEBUG("[%s] wrote %d TLS bytes", name(), ret);
    return ret;
  }

  switch (ret = SSL_get_error(_ssl, ret)) {
    case SSL_ERROR_ZERO_RETURN:
      ESB_LOG_DEBUG("[%s] peer closed TLS connection", name());
      return 0;
    case SSL_ERROR_WANT_WRITE:
      errno = ESB_AGAIN;
      return -1;
    case SSL_ERROR_SYSCALL:
    case SSL_ERROR_SSL:
    default:
      _flags &= ~ESB_TLS_FLAG_ALL;
      _flags |= ESB_TLS_FLAG_DEAD;
      ESB_LOG_TLS_INFO("[%s] cannot write TLS bytes (%d)", name(), ret);
      errno = ESB_TLS_SESSION_ERROR;
      return -1;
  }
}

bool TLSSocket::wantRead() { return _flags & ESB_TLS_FLAG_WANT_READ; }

bool TLSSocket::wantWrite() { return _flags & ESB_TLS_FLAG_WANT_WRITE; }

void DescribeTLSError(char *buffer, int size) {
  const char *file = NULL;
  int line = 0;
  const char *data = NULL;
  int flags = 0;
  ESB::UInt32 length = 0;
  bool first = true;

  while (true) {
    unsigned long code = ERR_get_error_line_data(&file, &line, &data, &flags);
    if (0 == code) {
      if (first) {
        buffer[0] = 0;
      }
      return;
    }

    if (!first) {
      // YUCK, I wish there was a way to avoid ^2 with all these strlen()s.
      length = strlen(buffer);

      if (length >= size - 1) {
        ERR_clear_error();
        return;
      }

      buffer[length++] = '|';
    }

    ERR_error_string_n(code, buffer + length, size - length);
    first = false;
  }
}

}  // namespace ESB
