#ifndef ES_HTTP_TRANSACTION_H
#include <ESHttpTransaction.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

namespace ES {

HttpTransaction::HttpTransaction(ESB::CleanupHandler *cleanupHandler)
    : _allocator(ESB::SystemConfig::Instance().pageSize(),
                 ESB::SystemConfig::Instance().cacheLineSize(),
                 ESB::SystemAllocator::Instance()),
      _context(0),
      _cleanupHandler(cleanupHandler),
      _start(),
      _peerAddress(),
      _request(),
      _response() {}

HttpTransaction::HttpTransaction(ESB::SocketAddress *peerAddress,
                                 ESB::CleanupHandler *cleanupHandler)
    : _allocator(ESB::SystemConfig::Instance().pageSize(),
                 ESB::SystemConfig::Instance().cacheLineSize(),
                 ESB::SystemAllocator::Instance()),
      _context(0),
      _cleanupHandler(cleanupHandler),
      _start(),
      _peerAddress(*peerAddress),
      _request(),
      _response() {}

HttpTransaction::~HttpTransaction() {}

void HttpTransaction::reset() {
  _context = 0;

  ESB::SocketAddress peerAddress;

  _peerAddress = peerAddress;

  _allocator.reset();
  _request.reset();
  _response.reset();
  //_ioBuffer.compact();
  _start = 0;
}

ESB::CleanupHandler *HttpTransaction::cleanupHandler() {
  return _cleanupHandler;
}

}  // namespace ES
