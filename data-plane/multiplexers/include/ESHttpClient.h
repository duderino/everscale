#ifndef ES_HTTP_CLIENT_H
#define ES_HTTP_CLIENT_H

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

#ifndef ESB_CLIENT_TLS_CONTEXT_INDEX_H
#include <ESBClientTLSContextIndex.h>
#endif

namespace ES {

class HttpClient {
 public:
  /**
   * Create a client stack.
   */
  HttpClient(const char *namePrefix, ESB::UInt32 threads, ESB::UInt32 idleTimeoutMsec, HttpClientHandler &clientHandler,
             ESB::Allocator &allocator = ESB::SystemAllocator::Instance());

  virtual ~HttpClient();

  inline ESB::ClientTLSContextIndex &clientTlsContextIndex() { return _clientContextIndex; }

  inline const ESB::ClientTLSContextIndex &clientTlsContextIndex() const { return _clientContextIndex; }

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

  void stop();

  ESB::Error join();

  void destroy();

 private:
  typedef enum {
    ES_HTTP_CLIENT_IS_INITIALIZED = 0,
    ES_HTTP_CLIENT_IS_STARTED = 1,
    ES_HTTP_CLIENT_IS_STOPPED = 2,
    ES_HTTP_CLIENT_IS_JOINED = 3,
    ES_HTTP_CLIENT_IS_DESTROYED = 4
  } HttpClientState;

  ESB::UInt32 _threads;
  ESB::UInt32 _idleTimeoutMsec;
  ESB::SharedInt _state;
  ESB::Allocator &_allocator;
  HttpClientHandler &_clientHandler;
  ESB::List _multiplexers;
  ESB::ThreadPool _threadPool;
  ESB::Rand _rand;
  ESB::ClientTLSContextIndex _clientContextIndex;
  char _name[ESB_NAME_PREFIX_SIZE];

  ESB_DEFAULT_FUNCS(HttpClient);
};

}  // namespace ES

#endif
