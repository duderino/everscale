#ifndef ES_HTTP_MULTIPLEXER_H
#define ES_HTTP_MULTIPLEXER_H

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_EPOLL_MULTIPLEXER_H
#include <ESBEpollMultiplexer.h>
#endif

#ifndef ESB_BUFFER_POOL_H
#include <ESBBufferPool.h>
#endif

namespace ES {

class HttpMultiplexer : public ESB::SocketMultiplexer {
 public:
  HttpMultiplexer(ESB::UInt32 maxSockets);

  virtual ~HttpMultiplexer();

  virtual ESB::Error addMultiplexedSocket(
      ESB::MultiplexedSocket *multiplexedSocket);
  virtual int currentSockets() const;
  virtual int maximumSockets() const;
  virtual bool isRunning() const;

 private:
  // disabled
  HttpMultiplexer(const HttpMultiplexer &);
  void operator=(const HttpMultiplexer &);

  ESB::DiscardAllocator _ioBufferPoolAllocator;

 protected:
  ESB::BufferPool _ioBufferPool;
  ESB::DiscardAllocator _factoryAllocator;
  ESB::EpollMultiplexer _epollMultiplexer;
};

}  // namespace ES

#endif
