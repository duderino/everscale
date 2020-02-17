#ifndef ESB_EPOLL_MULTIPLEXER_FACTORY_H
#include <ESBEpollMultiplexerFactory.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {

EpollMultiplexerFactory::EpollMultiplexerFactory(const char *name,
                                                 Logger *logger,
                                                 Allocator *allocator)
    : _name(name),
      _logger(logger ? logger : NullLogger::GetInstance()),
      _allocator(allocator ? allocator : SystemAllocator::GetInstance()) {}

EpollMultiplexerFactory::~EpollMultiplexerFactory() {}

SocketMultiplexer *EpollMultiplexerFactory::create(int maxSockets) {
  return new (_allocator)
      EpollMultiplexer(_name, maxSockets, _logger, _allocator);
}

void EpollMultiplexerFactory::destroy(SocketMultiplexer *multiplexer) {
  if (!multiplexer) {
    return;
  }

  multiplexer->~SocketMultiplexer();
  _allocator->deallocate(multiplexer);
}

}  // namespace ESB
