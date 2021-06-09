#ifndef ES_HTTP_ORIGIN_CONTEXT_H
#define ES_HTTP_ORIGIN_CONTEXT_H

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

namespace ES {

class HttpOriginContext {
 public:
  HttpOriginContext();

  virtual ~HttpOriginContext();

  inline ESB::UInt64 bytesSent() const { return _bytesSent; }

  inline void setBytesSent(ESB::UInt64 bytesSent) { _bytesSent = bytesSent; }

  inline ESB::UInt64 bytesReceived() const { return _bytesReceived; }

  inline void setBytesReceived(ESB::UInt64 bytesReceived) { _bytesReceived = bytesReceived; }

 private:
  ESB::UInt64 _bytesSent;
  ESB::UInt64 _bytesReceived;

  ESB_DEFAULT_FUNCS(HttpOriginContext);
};

}  // namespace ES

#endif
