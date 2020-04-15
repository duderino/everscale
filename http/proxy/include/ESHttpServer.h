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

#ifndef ESB_RAND_H
#include <ESBRand.h>
#endif

namespace ES {

class HttpServer {
 public:
  /**
   * Constructor
   */
  HttpServer(ESB::UInt32 threads, HttpServerHandler &serverHandler,
             ESB::Allocator &allocator = ESB::SystemAllocator::Instance());

  virtual ~HttpServer();

  /**
   * Enqueue a command to be run on a multiplexer thread.  If the
   * command has a cleanup handler, the multiplexer will call its cleanup
   * handler after the command finishes.
   *
   * @param command The command to execute
   * @param idx the index of a specific multiplexer ranging from 0 to
   * threads()-1 inclusive.  If -1 then a random multiplexer will be picked.
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error push(HttpServerCommand *command, int idx = -1);

  ESB::Error addListener(ESB::ListeningTCPSocket &listener);

  inline ESB::UInt32 threads() { return _threads; }

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
  ESB::Rand _rand;
  HttpServerSimpleCounters _serverCounters;
};

}  // namespace ES

#endif
