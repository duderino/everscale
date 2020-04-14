#ifndef ES_HTTP_CLIENT_H
#define ES_HTTP_CLIENT_H

#ifndef ES_HTTP_CLIENT_MULTIPLEXER_H
#include <ESHttpClientMultiplexer.h>
#endif

#ifndef ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H
#include <ESHttpClientHistoricalCounters.h>
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

class HttpClient {
 public:
  /**
   * Create a client stack.
   */
  HttpClient(ESB::UInt32 threads, HttpClientHandler &clientHandler,
             ESB::Allocator &allocator = ESB::SystemAllocator::Instance());

  virtual ~HttpClient();

  /**
   * Enqueue a command to be run on all multiplexer threads.  NB: the command's
   * cleanup handler must return NULL, else multiple multiplexers will try to
   * clean it up.
   *
   * @param command The command to execute
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error pushAll(HttpClientCommand *command);

  ESB::Error initialize();

  ESB::Error start();

  ESB::Error stop();

  void destroy();

  inline const HttpClientCounters &counters() const { return _clientCounters; }

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
  HttpClientHistoricalCounters _clientCounters;
};

}  // namespace ES

#endif
