#ifndef ESB_TLS_SOCKET_H
#define ESB_TLS_SOCKET_H

#ifndef ESB_CONNECTED_SOCKET_H
#include <ESBConnectedSocket.h>
#endif

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#include <openssl/ssl.h>

#define ESB_DEFAULT_CA_BUNDLE_PATH "/etc/ssl/certs/ca-certificates.crt"

#define ESB_TLS_FLAG_ESTABLISHED (ESB_SOCK_FLAG_MAX << 1)
#define ESB_TLS_FLAG_DEAD (ESB_SOCK_FLAG_MAX << 2)
#define ESB_TLS_FLAG_WANT_READ (ESB_SOCK_FLAG_MAX << 3)
#define ESB_TLS_FLAG_WANT_WRITE (ESB_SOCK_FLAG_MAX << 4)
#define ESB_TLS_FLAG_ALL (ESB_TLS_FLAG_ESTABLISHED | ESB_TLS_FLAG_DEAD | ESB_TLS_FLAG_WANT_READ | ESB_TLS_FLAG_WANT_WRITE)

namespace ESB {

/** TLSSocket is an abstract base class for client and server TLS Sockets.
 *
 *  @ingroup network
 */
class TLSSocket : public ConnectedSocket {
 public:
  /** Construct a new server TLSSocket.
   *
   * @param acceptState init parameters created by the ListeningSocket
   * @param namePrefix A name prefix to be incorporated into log messages
   */
  TLSSocket(const Socket::State &acceptState, const char *namePrefix);

  /** Construct a new client TLSSocket.
   *
   * @param namePrefix A name prefix to be incorporated into log messages
   * @param isBlocking whether or not this socket is blocking.
   */
  TLSSocket(const char *namePrefix, bool isBlocking = false);

  virtual ~TLSSocket();
  //
  // ESB::ConnectedSocket
  //

  virtual void close();
  virtual bool secure() const;
  virtual SSize receive(char *buffer, Size bufferSize);
  virtual SSize send(const char *buffer, Size bufferSize);
  virtual bool wantRead();
  virtual bool wantWrite();

 protected:
  static Error Initialize();
  virtual Error startHandshake() = 0;

  SSL *_ssl;
  BIO *_bio;

 private:
  // Disabled
  TLSSocket(const TLSSocket &);
  TLSSocket &operator=(const TLSSocket &);

  static bool _Initialized;
};

void DescribeTLSError(char *buffer, int size);

#define ESB_LOG_TLS_ERROR(FORMAT, ...)                                                                        \
  do {                                                                                                              \
    if (ESB_ERROR_LOGGABLE) {                                                                                     \
        char _tls_buffer_[256];                                                                                \
        DescribeTLSError(_tls_buffer_, sizeof(_tls_buffer_)); \
        ESB::Logger::Instance().log(ESB::Logger::Err, ESB_ERROR_LOG_PREFIX FORMAT ": %s" ESB_LOG_SUFFIX,            \
                                    ESB::Time::Instance().nowSec(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__,  \
                                    _tls_buffer_);                                                                   \
    }                                                                                                               \
  } while (0)

#define ESB_LOG_TLS_WARNING(FORMAT, ...)                                                                      \
  do {                                                                                                              \
    if (ESB_WARNING_LOGGABLE) {                                                                                     \
        char _tls_buffer_[256];                                                                                \
      DescribeTLSError(_tls_buffer_, sizeof(_tls_buffer_)); \
        ESB::Logger::Instance().log(ESB::Logger::Warning, ESB_WARNING_LOG_PREFIX FORMAT ": %s" ESB_LOG_SUFFIX,      \
                                    ESB::Time::Instance().nowSec(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__,  \
                                    _tls_buffer_);                                                                   \
    }                                                                                                               \
  } while (0)

#define ESB_LOG_TLS_INFO(FORMAT, ...)                                                                         \
  do {                                                                                                              \
    if (ESB_INFO_LOGGABLE) {                                                                                        \
        char _tls_buffer_[256];                                                                                \
      DescribeTLSError(_tls_buffer_, sizeof(_tls_buffer_)); \
        ESB::Logger::Instance().log(ESB::Logger::Info, ESB_INFO_LOG_PREFIX FORMAT ": %s" ESB_LOG_SUFFIX,            \
                                    ESB::Time::Instance().nowSec(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__,  \
                                    _tls_buffer_);                                                                   \
    }                                                                                                               \
  } while (0)

#define ESB_LOG_TLS_DEBUG(FORMAT, ...)                                                                        \
  do {                                                                                                              \
    if (ESB_DEBUG_LOGGABLE) {                                                                                       \
        char _tls_buffer_[256];                                                                                \
      DescribeTLSError(_tls_buffer_, sizeof(_tls_buffer_)); \
        ESB::Logger::Instance().log(ESB::Logger::Debug, ESB_DEBUG_LOG_PREFIX FORMAT ": %s" ESB_LOG_SUFFIX,          \
                                    ESB::Time::Instance().nowSec(), ESB::Thread::CurrentThreadId(), ##__VA_ARGS__,  \
                                    _tls_buffer_);                                                                   \
    }                                                                                                               \
  } while (0)

}  // namespace ESB

#endif
