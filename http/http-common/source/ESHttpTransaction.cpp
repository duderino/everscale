#ifndef ES_HTTP_TRANSACTION_H
#include <ESHttpTransaction.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

namespace ES {

HttpTransaction::HttpTransaction(ESB::CleanupHandler &cleanupHandler)
    : _allocator(ESB_PAGE_SIZE -
                     ESB::DiscardAllocator::SizeofChunk(ESB_CACHE_LINE_SIZE),
                 ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE,
                 ESB::SystemAllocator::Instance(), true),
      _context(NULL),
      _cleanupHandler(cleanupHandler),
      _start(),
      _peerAddress(),
      _request(),
      _response(),
      _parseBuffer(_parseBufferStorage, sizeof(_parseBufferStorage)) {}

HttpTransaction::HttpTransaction(const ESB::SocketAddress *peerAddress,
                                 ESB::CleanupHandler &cleanupHandler)
    : _allocator(ESB_PAGE_SIZE -
                     ESB::DiscardAllocator::SizeofChunk(ESB_CACHE_LINE_SIZE),
                 ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE,
                 ESB::SystemAllocator::Instance()),
      _context(NULL),
      _cleanupHandler(cleanupHandler),
      _start(),
      _peerAddress(*peerAddress),
      _request(),
      _response(),
      _parseBuffer(_parseBufferStorage, sizeof(_parseBufferStorage)) {}

HttpTransaction::~HttpTransaction() {}

void HttpTransaction::reset() {
  _context = 0;

  ESB::SocketAddress peerAddress;

  _peerAddress = peerAddress;

  _allocator.reset();
  _request.reset();
  _response.reset();
  _parseBuffer.clear();
  _start = 0;
}

ESB::CleanupHandler *HttpTransaction::cleanupHandler() {
  return &_cleanupHandler;
}

}  // namespace ES
