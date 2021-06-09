#ifndef ES_HTTP_CONFIG_H
#define ES_HTTP_CONFIG_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

namespace ES {

class HttpConfig {
 public:
  static inline HttpConfig &Instance() { return _Instance; }
  virtual ~HttpConfig();
  inline ESB::UInt32 ioBufferSize() const { return _ioBufferSize; }
  inline ESB::UInt32 ioBufferChunkSize() const { return _ioBufferChunkSize; }
  inline ESB::UInt32 connectionPoolBuckets() const { return _connectionPoolBuckets; };
  inline ESB::UInt32 tlsContextBuckets() const { return _connectionPoolBuckets; };
  inline ESB::UInt32 tlsContextLocks() const { return MIN(47, _connectionPoolBuckets); };

 private:
  // Singleton
  HttpConfig();

  inline ESB::UInt32 idleTimeoutSeconds() const { return _idleTimeoutSeconds; }

  inline HttpConfig &setIdleTimeoutSeconds(ESB::UInt32 seconds) {
    _idleTimeoutSeconds = seconds;
    return *this;
  }

  ESB::UInt32 _ioBufferSize;
  ESB::UInt32 _ioBufferChunkSize;
  ESB::UInt32 _connectionPoolBuckets;
  ESB::UInt32 _idleTimeoutSeconds;
  static HttpConfig _Instance;

  ESB_DEFAULT_FUNCS(HttpConfig);
};

}  // namespace ES

#endif
