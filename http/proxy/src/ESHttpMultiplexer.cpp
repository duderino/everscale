#ifndef ES_HTTP_MULTIPLEXER_H
#include <ESHttpMultiplexer.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ES_HTTP_CONFIG_H
#include <ESHttpConfig.h>
#endif

namespace ES {

HttpMultiplexer::HttpMultiplexer(ESB::UInt32 maxSockets)
    : _ioBufferPoolAllocator(HttpConfig::Instance().ioBufferChunkSize(),
                             ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE,
                             ESB::SystemAllocator::Instance()),
      _ioBufferPool(HttpConfig::Instance().ioBufferSize(), 0,
                    ESB::NullLock::Instance(), _ioBufferPoolAllocator),
      _factoryAllocator(
          ESB_PAGE_SIZE * 1000 -
              ESB::DiscardAllocator::SizeofChunk(ESB_CACHE_LINE_SIZE),
          ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE, ESB::SystemAllocator::Instance()),
      _epollMultiplexer(maxSockets, ESB::SystemAllocator::Instance()) {}

HttpMultiplexer::~HttpMultiplexer() {}

ESB::Error HttpMultiplexer::addMultiplexedSocket(
    ESB::MultiplexedSocket *multiplexedSocket) {
  return _epollMultiplexer.addMultiplexedSocket(multiplexedSocket);
}

int HttpMultiplexer::currentSockets() const {
  return _epollMultiplexer.currentSockets();
}

int HttpMultiplexer::maximumSockets() const {
  return _epollMultiplexer.maximumSockets();
}

bool HttpMultiplexer::isRunning() const {
  return _epollMultiplexer.isRunning();
}

}  // namespace ES
