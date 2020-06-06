#ifndef ESB_EPOLL_MULTIPLEXER_FACTORY_H
#include <ESBEpollMultiplexerFactory.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {

EpollMultiplexerFactory::EpollMultiplexerFactory(const char *name, Allocator &allocator)
    : _name(name), _allocator(allocator) {}

EpollMultiplexerFactory::~EpollMultiplexerFactory() {}

SocketMultiplexer *EpollMultiplexerFactory::create(int maxSockets) {
  return new (_allocator) EpollMultiplexer(maxSockets, _allocator);
}

void EpollMultiplexerFactory::destroy(SocketMultiplexer *multiplexer) {
  if (!multiplexer) {
    return;
  }

  multiplexer->~SocketMultiplexer();
  _allocator.deallocate(multiplexer);
}

}  // namespace ESB
