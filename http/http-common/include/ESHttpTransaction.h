#ifndef ES_HTTP_TRANSACTION_H
#define ES_HTTP_TRANSACTION_H

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

#ifndef ES_HTTP_HEADER_H
#include <ESHttpHeader.h>
#endif

#ifndef ES_HTTP_REQUEST_H
#include <ESHttpRequest.h>
#endif

#ifndef ES_HTTP_RESPONSE_H
#include <ESHttpResponse.h>
#endif

#ifndef ESB_PERFORMANCE_COUNTER_H
#include <ESBPerformanceCounter.h>
#endif

namespace ES {

// TODO buffers should not be exposed here.  Move to a subclass that is not
// visible to the client and server handlers.
class HttpTransaction : public ESB::EmbeddedListElement {
 public:
  HttpTransaction(ESB::CleanupHandler *cleanupHandler);

  HttpTransaction(ESB::SocketAddress *peerAddress,
                  ESB::CleanupHandler *cleanupHandler);

  virtual ~HttpTransaction();

  inline const ESB::SocketAddress &peerAddress() const { return _peerAddress; }

  inline ESB::SocketAddress &peerAddress() { return _peerAddress; }

  inline void setPeerAddress(const ESB::SocketAddress &peerAddress) {
    _peerAddress = peerAddress;
  }

  virtual void reset();

  inline unsigned char *duplicate(unsigned char *value) {
    return HttpUtil::Duplicate(&_allocator, value);
  }

  inline ESB::Allocator &allocator() { return _allocator; }

  inline const HttpRequest &request() const { return _request; }

  inline HttpRequest &request() { return _request; }

  inline const HttpResponse &response() const { return _response; }

  inline HttpResponse &response() { return _response; }

  inline void setContext(void *appContext) { _context = appContext; }

  inline void *context() { return _context; }

  inline const void *context() const { return _context; }

  /** Return an optional handler that can destroy the element.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual ESB::CleanupHandler *cleanupHandler();

  inline void setStartTime() { _start = ESB::Date::Now(); }

  inline const ESB::Date &startTime() const { return _start; }

 protected:
  ESB::DiscardAllocator _allocator;  // 24 from base+48= 72

 private:
  // Disabled
  HttpTransaction(const HttpTransaction &transaction);
  void operator=(const HttpTransaction &transaction);

  void *_context;                        // 72+8 = 80
  ESB::CleanupHandler *_cleanupHandler;  // 80+8 = 88
  ESB::Date _start;                      // 88+16 = 104
  ESB::SocketAddress _peerAddress;       // 104+24 = 128
  HttpRequest _request;                  // 128+128 = 256
  HttpResponse _response;                // 256+64 = 320
};

}  // namespace ES

#endif
