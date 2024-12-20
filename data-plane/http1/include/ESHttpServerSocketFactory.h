#ifndef ES_HTTP_SERVER_SOCKET_FACTORY_H
#define ES_HTTP_SERVER_SOCKET_FACTORY_H

#ifndef ES_HTTP_SERVER_SOCKET_H
#include <ESHttpServerSocket.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ES_HTTP_CONNECTION_METRICS_H
#include <ESHttpConnectionMetrics.h>
#endif

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

#ifndef ESB_BUFFER_POOL_H
#include <ESBBufferPool.h>
#endif

#ifndef ES_HTTP_MULTIPLEXER_EXTENDED_H
#include <ESHttpMultiplexerExtended.h>
#endif

#ifndef ESB_SERVER_TLS_CONTEXT_INDEX_H
#include <ESBServerTLSContextIndex.h>
#endif

namespace ES {

/** A factory that creates and reuses HttpServerSockets
 *
 */
class HttpServerSocketFactory {
 public:
  HttpServerSocketFactory(HttpMultiplexerExtended &multiplexer, HttpServerHandler &handler,
                          HttpConnectionMetrics &connectionMetrics, ESB::ServerTLSContextIndex &contextIndex,
                          ESB::Allocator &allocator);

  virtual ~HttpServerSocketFactory();

  HttpServerSocket *create(ESB::Socket::State &state);

  void release(HttpServerSocket *socket);

 private:
  void releaseSocket(ESB::ConnectedSocket *socket);

  class CleanupHandler : public ESB::CleanupHandler {
   public:
    /** Constructor
     */
    CleanupHandler(HttpServerSocketFactory &factory);

    /** Destructor
     */
    virtual ~CleanupHandler();

    /** Destroy an object
     *
     * @param object The object to destroy
     */
    virtual void destroy(ESB::Object *object);

   private:
    HttpServerSocketFactory &_factory;

    ESB_DISABLE_AUTO_COPY(CleanupHandler);
  };

  ESB::ServerTLSContextIndex &_contextIndex;
  HttpMultiplexerExtended &_multiplexer;
  HttpServerHandler &_handler;
  HttpConnectionMetrics &_connectionMetrics;
  ESB::Allocator &_allocator;
  ESB::EmbeddedList _deconstructedHTTPSockets;
  ESB::EmbeddedList _deconstructedClearSockets;
  ESB::EmbeddedList _deconstructedTLSSockets;
  CleanupHandler _cleanupHandler;

  ESB_DEFAULT_FUNCS(HttpServerSocketFactory);
};

}  // namespace ES

#endif
