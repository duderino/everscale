#ifndef ES_HTTP_SERVER_H
#define ES_HTTP_SERVER_H

#ifndef ES_HTTP_SERVER_MULTIPLEXER_H
#include <ESHttpServerMultiplexer.h>
#endif

#ifndef ES_HTTP_SERVER_SIMPLE_COUNTERS_H
#include <ESHttpServerSimpleCounters.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

#ifndef ESB_THREAD_POOL_H
#include <ESBThreadPool.h>
#endif

#ifndef ESB_LIST_H
#include <ESBList.h>
#endif

namespace ES {

class HttpServer {
 public:
  /**
   * Constructor
   */
  HttpServer(ESB::UInt32 threads, ESB::UInt16 port,
             HttpServerHandler &serverHandler,
             ESB::Allocator &allocator = ESB::SystemAllocator::Instance());

  virtual ~HttpServer();

  inline ESB::UInt16 port() {
    return _listeningSocket.listeningAddress().port();
  }

  ESB::Error initialize();

  ESB::Error start();

  ESB::Error stop();

  void destroy();

  inline const HttpServerCounters &counters() const { return _serverCounters; }

 private:
  // disabled
  HttpServer(const HttpServer &);
  void operator=(const HttpServer &);

  typedef enum {
    ES_HTTP_SERVER_IS_INITIALIZED = 0,
    ES_HTTP_SERVER_IS_STARTED = 1,
    ES_HTTP_SERVER_IS_STOPPED = 2,
    ES_HTTP_SERVER_IS_DESTROYED = 3
  } HttpServerState;

  ESB::UInt32 _threads;
  ESB::SharedInt _state;
  ESB::Allocator &_allocator;
  HttpServerHandler &_serverHandler;
  ESB::List _multiplexers;
  ESB::ThreadPool _threadPool;
  ESB::ListeningTCPSocket _listeningSocket;
  HttpServerSimpleCounters _serverCounters;
};

}  // namespace ES

#endif
