#ifndef ES_HTTP_SERVER_H
#define ES_HTTP_SERVER_H

#ifndef ES_HTTP_SERVER_SIMPLE_COUNTERS_H
#include <ESHttpServerSimpleCounters.h>
#endif

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

#ifndef ES_HTTP_SERVER_COMMAND_H
#include <ESHttpServerCommand.h>
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

#ifndef ESB_SERVER_TLS_CONTEXT_INDEX_H
#include <ESBServerTLSContextIndex.h>
#endif

namespace ES {

class HttpServer {
 public:
  /**
   * Constructor
   */
  HttpServer(const char *namePrefix, ESB::UInt32 threads, ESB::UInt32 idleTimeoutMsec, HttpServerHandler &serverHandler,
             ESB::Allocator &allocator = ESB::SystemAllocator::Instance());

  virtual ~HttpServer();

  inline ESB::ServerTLSContextIndex &serverTlsContextIndex() { return _serverContextIndex; }

  inline const ESB::ServerTLSContextIndex &serverTlsContextIndex() const { return _serverContextIndex; }

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

  /**
   * Add a bound listening socket to the http server.  This operation will
   * create a duplicate socket descriptor for each multiplexer thread and
   * call listen on it.
   *
   * @param listener A listening socket to add to each multiplexer thread
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error addListener(ESB::ListeningSocket &listener);

  /**
   * Get the number of multiplexer threads (1 multiplexer per thread).
   *
   * @return The number of multiplexers
   */
  inline ESB::UInt32 threads() { return _threads; }

  ESB::Error initialize();

  ESB::Error start();

  void stop();

  ESB::Error join();

  void destroy();

  inline const HttpServerCounters &serverCounters() const { return _serverCounters; }

  class AddListeningSocketCommand : public HttpServerCommand {
   public:
    AddListeningSocketCommand(ESB::ListeningSocket &socket, ESB::CleanupHandler &cleanupHandler)
        : _socket(socket), _cleanupHandler(cleanupHandler) {}

    virtual ~AddListeningSocketCommand(){};

    virtual ESB::Error run(HttpMultiplexerExtended &multiplexer) { return multiplexer.addListeningSocket(_socket); }

    virtual ESB::CleanupHandler *cleanupHandler() { return &_cleanupHandler; }

    virtual const char *name() { return "add listening socket"; }

   private:
    ESB::ListeningSocket &_socket;
    ESB::CleanupHandler &_cleanupHandler;

    ESB_DEFAULT_FUNCS(AddListeningSocketCommand);
  };

 protected:
  virtual ESB::SocketMultiplexer *createMultiplexer();

  virtual void destroyMultiplexer(ESB::SocketMultiplexer *multiplexer);

  typedef enum {
    ES_HTTP_SERVER_IS_INITIALIZED = 0,
    ES_HTTP_SERVER_IS_STARTED = 1,
    ES_HTTP_SERVER_IS_STOPPED = 2,
    ES_HTTP_SERVER_IS_JOINED = 3,
    ES_HTTP_SERVER_IS_DESTROYED = 4
  } HttpServerState;

  ESB::UInt32 _threads;
  ESB::UInt32 _idleTimeoutMsec;
  ESB::SharedInt _state;
  ESB::Allocator &_allocator;
  HttpServerHandler &_serverHandler;
  ESB::List _multiplexers;
  ESB::ThreadPool _threadPool;
  ESB::Rand _rand;
  ESB::ServerTLSContextIndex _serverContextIndex;
  HttpServerSimpleCounters _serverCounters;
  char _name[ESB_NAME_PREFIX_SIZE];

  ESB_DEFAULT_FUNCS(HttpServer);
};

}  // namespace ES

#endif
