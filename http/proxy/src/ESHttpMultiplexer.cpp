#ifndef ES_HTTP_MULTIPLEXER_H
#include <ESHttpMultiplexer.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#include <errno.h>

namespace ES {

HttpMultiplexer::HttpMultiplexer(ESB::UInt32 maxSockets,
                                 ESB::Allocator &allocator)
    : _sourceAllocator(allocator),
      _factoryAllocator(ESB::SystemConfig::Instance().pageSize() * 1000,
                        ESB::SystemConfig::Instance().cacheLineSize(),
                        _sourceAllocator),
      _epollMultiplexer(maxSockets, _sourceAllocator) {}

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
