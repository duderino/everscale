#ifndef ES_HTTP_CONFIG_H
#define ES_HTTP_CONFIG_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ES {

class HttpConfig {
 public:
  static inline HttpConfig &Instance() { return _Instance; }
  virtual ~HttpConfig();
  inline ESB::UInt32 ioBufferSize() { return _ioBufferSize; }
  inline ESB::UInt32 ioBufferChunkSize() { return _ioBufferChunkSize; }
  inline ESB::UInt32 connectionPoolBuckets() { return _connectionPoolBuckets; };

 private:
  // Singleton
  HttpConfig();
  // Disabled
  HttpConfig(const HttpConfig &);
  void operator=(const HttpConfig &);

  ESB::UInt32 _ioBufferSize;
  ESB::UInt32 _ioBufferChunkSize;
  ESB::UInt32 _connectionPoolBuckets;
  static HttpConfig _Instance;
};

}  // namespace ES

#endif
