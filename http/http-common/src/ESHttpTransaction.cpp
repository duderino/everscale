#ifndef ES_HTTP_TRANSACTION_H
#include <ESHttpTransaction.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ES {

#ifndef ES_HTTP_TRANSACTION_ALLOCATOR_CHUNK_SIZE
#define ES_HTTP_TRANSACTION_ALLOCATOR_CHUNK_SIZE 2000
#endif

HttpTransaction::HttpTransaction(ESB::CleanupHandler *cleanupHandler)
    : _allocator(ES_HTTP_TRANSACTION_ALLOCATOR_CHUNK_SIZE,
                 ESB::SystemAllocator::GetInstance()),
      _appContext(0),
      _cleanupHandler(cleanupHandler),
      _peerAddress(),
      _request(),
      _response(),
      _ioBuffer(_ioBufferStorage, sizeof(_ioBufferStorage)),
      _workingBuffer(_workingBufferStorage, sizeof(_workingBufferStorage)) {
  memset(&_start, 0, sizeof(_start));
}

HttpTransaction::HttpTransaction(ESB::SocketAddress *peerAddress,
                                 ESB::CleanupHandler *cleanupHandler)
    : _allocator(ES_HTTP_TRANSACTION_ALLOCATOR_CHUNK_SIZE,
                 ESB::SystemAllocator::GetInstance()),
      _appContext(0),
      _cleanupHandler(cleanupHandler),
      _peerAddress(*peerAddress),
      _request(),
      _response(),
      _ioBuffer(_ioBufferStorage, sizeof(_ioBufferStorage)),
      _workingBuffer(_workingBufferStorage, sizeof(_workingBufferStorage)) {
  memset(&_start, 0, sizeof(_start));
}

HttpTransaction::~HttpTransaction() {}

void HttpTransaction::reset() {
  _appContext = 0;

  ESB::SocketAddress peerAddress;

  _peerAddress = peerAddress;

  _allocator.reset();
  _request.reset();
  _response.reset();
  //_ioBuffer.compact();
  _workingBuffer.clear();

  memset(&_start, 0, sizeof(_start));
}

HttpHeader *HttpTransaction::createHeader(unsigned const char *name,
                                          unsigned const char *value) {
  unsigned char *fieldName = HttpUtil::Duplicate(&_allocator, name);

  if (!fieldName) {
    return 0;
  }

  unsigned char *fieldValue = HttpUtil::Duplicate(&_allocator, value);

  if (!fieldValue) {
    _allocator.deallocate(fieldName);

    return 0;
  }

  HttpHeader *header = new (&_allocator) HttpHeader(fieldName, fieldValue);

  if (!header) {
    _allocator.deallocate(fieldName);
    _allocator.deallocate(fieldValue);

    return 0;
  }

  return header;
}

ESB::CleanupHandler *HttpTransaction::getCleanupHandler() {
  return _cleanupHandler;
}

}  // namespace ES
