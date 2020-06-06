#ifndef ES_HTTP_CLIENT_H
#define ES_HTTP_CLIENT_H

#ifndef ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H
#include <ESHttpClientHistoricalCounters.h>
#endif

#ifndef ES_HTTP_CLIENT_HANDLER_H
#include <ESHttpClientHandler.h>
#endif

#ifndef ES_HTTP_CLIENT_COMMAND_H
#include <ESHttpClientCommand.h>
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

class HttpClient {
 public:
  /**
   * Create a client stack.
   */
  HttpClient(ESB::UInt32 threads, HttpClientHandler &clientHandler,
             ESB::Allocator &allocator = ESB::SystemAllocator::Instance());

  virtual ~HttpClient();

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
  ESB::Error push(HttpClientCommand *command, int idx = -1);

  inline ESB::UInt32 threads() { return _threads; }

  ESB::Error initialize();

  ESB::Error start();

  ESB::Error stop();

  void destroy();

  inline const HttpClientCounters &clientCounters() const { return _clientCounters; }

 private:
  // disabled
  HttpClient(const HttpClient &);
  void operator=(const HttpClient &);

  typedef enum {
    ES_HTTP_CLIENT_IS_INITIALIZED = 0,
    ES_HTTP_CLIENT_IS_STARTED = 1,
    ES_HTTP_CLIENT_IS_STOPPED = 2,
    ES_HTTP_CLIENT_IS_DESTROYED = 3
  } HttpClientState;

  ESB::UInt32 _threads;
  ESB::SharedInt _state;
  ESB::Allocator &_allocator;
  HttpClientHandler &_clientHandler;
  ESB::List _multiplexers;
  ESB::ThreadPool _threadPool;
  ESB::Rand _rand;
  HttpClientHistoricalCounters _clientCounters;
};

}  // namespace ES

#endif
