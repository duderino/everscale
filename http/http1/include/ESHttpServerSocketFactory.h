#ifndef ES_HTTP_SERVER_SOCKET_FACTORY_H
#define ES_HTTP_SERVER_SOCKET_FACTORY_H

#ifndef ES_HTTP_SERVER_SOCKET_H
#include <ESHttpServerSocket.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ES_HTTP_SERVER_COUNTERS_H
#include <ESHttpServerCounters.h>
#endif

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

namespace ES {

/** A factory that creates and reuses HttpServerSockets
 */
class HttpServerSocketFactory {
 public:
  HttpServerSocketFactory(HttpServerHandler &handler,
                          HttpServerCounters &counters,
                          ESB::Allocator &allocator);

  virtual ~HttpServerSocketFactory();

  HttpServerSocket *create(ESB::TCPSocket::State &state);

  void release(HttpServerSocket *socket);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  // Disabled
  HttpServerSocketFactory(const HttpServerSocketFactory &);
  HttpServerSocketFactory &operator=(const HttpServerSocketFactory &);

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
    // Disabled
    CleanupHandler(const CleanupHandler &);
    void operator=(const CleanupHandler &);

    HttpServerSocketFactory &_factory;
  };

  HttpServerHandler &_handler;
  HttpServerCounters &_counters;
  ESB::Allocator &_allocator;
  ESB::EmbeddedList _sockets;
  CleanupHandler _cleanupHandler;
};

}  // namespace ES

#endif
