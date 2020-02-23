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

#ifndef ES_HTTP_IO_BUFFER_SIZE
#define ES_HTTP_IO_BUFFER_SIZE 4096
#endif

#ifndef ES_HTTP_WORKING_BUFFER_SIZE
#define ES_HTTP_WORKING_BUFFER_SIZE 2048
#endif

// TODO buffers should not be exposed here.  Move to a subclass that is not
// visible to the client and server handlers.
class HttpTransaction : public ESB::EmbeddedListElement {
 public:
  HttpTransaction(ESB::CleanupHandler *cleanupHandler);

  HttpTransaction(ESB::SocketAddress *peerAddress,
                  ESB::CleanupHandler *cleanupHandler);

  virtual ~HttpTransaction();

  inline const ESB::SocketAddress *getPeerAddress() const {
    return &_peerAddress;
  }

  inline ESB::SocketAddress *getPeerAddress() { return &_peerAddress; }

  inline void setPeerAddress(const ESB::SocketAddress *peerAddress) {
    if (peerAddress) {
      _peerAddress = *peerAddress;
    }
  }

  virtual void reset();

  inline unsigned char *duplicate(unsigned char *value) {
    return HttpUtil::Duplicate(&_allocator, value);
  }

  HttpHeader *createHeader(unsigned const char *name,
                           unsigned const char *value);

  inline ESB::Allocator *getAllocator() { return &_allocator; }

  inline const HttpRequest *getRequest() const { return &_request; }

  inline HttpRequest *getRequest() { return &_request; }

  inline const HttpResponse *getResponse() const { return &_response; }

  inline HttpResponse *getResponse() { return &_response; }

  inline void setApplicationContext(void *appContext) {
    _appContext = appContext;
  }

  inline void *getApplicationContext() { return _appContext; }

  inline const void *getApplicationContext() const { return _appContext; }

  inline ESB::Buffer *getIOBuffer() { return &_ioBuffer; }

  inline const ESB::Buffer *getIOBuffer() const { return &_ioBuffer; }

  inline ESB::Buffer *getWorkingBuffer() { return &_workingBuffer; }

  inline const ESB::Buffer *getWorkingBuffer() const { return &_workingBuffer; }

  /** Return an optional handler that can destroy the element.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual ESB::CleanupHandler *getCleanupHandler();

  inline void setStartTime() { ESB::PerformanceCounter::GetTime(&_start); }

  inline const struct timeval *getStartTime() const { return &_start; }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator *allocator) {
    return allocator->allocate(size);
  }

 protected:
  ESB::DiscardAllocator _allocator;

 private:
  // Disabled
  HttpTransaction(const HttpTransaction &transaction);
  void operator=(const HttpTransaction &transaction);

  void *_appContext;
  ESB::CleanupHandler *_cleanupHandler;
  struct timeval _start;
  ESB::SocketAddress _peerAddress;
  HttpRequest _request;
  HttpResponse _response;
  ESB::Buffer _ioBuffer;
  ESB::Buffer _workingBuffer;
  unsigned char _ioBufferStorage[ES_HTTP_IO_BUFFER_SIZE];
  unsigned char _workingBufferStorage[ES_HTTP_WORKING_BUFFER_SIZE];
};

}  // namespace ES

#endif
