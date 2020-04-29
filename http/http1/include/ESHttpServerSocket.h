#ifndef ES_HTTP_SERVER_SOCKET_H
#define ES_HTTP_SERVER_SOCKET_H

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

#ifndef ESB_CONNECTED_TCP_SOCKET_H
#include <ESBConnectedTCPSocket.h>
#endif

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

#ifndef ES_HTTP_SERVER_TRANSACTION_H
#include <ESHttpServerTransaction.h>
#endif

#ifndef ES_HTTP_SERVER_COUNTERS_H
#include <ESHttpServerCounters.h>
#endif

#ifndef ES_HTTP_SERVER_STACK_H
#include <ESHttpServerStack.h>
#endif

#ifndef ES_HTTP_STREAM_H
#include <ESHttpStream.h>
#endif

namespace ES {

/** A socket that receives and echoes back HTTP requests
 *
 * TODO implement idle check
 */
class HttpServerSocket : public ESB::MultiplexedSocket, public HttpStream {
 public:
  /** Constructor
   */
  HttpServerSocket(HttpServerHandler &handler, HttpServerStack &stack,
                   HttpServerCounters &counters,
                   ESB::CleanupHandler &cleanupHandler);

  /** Destructor.
   */
  virtual ~HttpServerSocket();

  /** Reset the server socket
   *
   * @param state An object created populated by ESB::ListeningTCPSockets
   *  when accepting a new connection.
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error reset(ESB::TCPSocket::State &state);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

  //
  // ESB::MultiplexedSocket
  //

  virtual bool wantAccept();

  virtual bool wantConnect();

  virtual bool wantRead();

  virtual bool wantWrite();

  virtual bool isIdle();

  virtual ESB::Error handleAccept();

  virtual bool handleConnect();

  virtual bool handleReadable();

  virtual bool handleWritable();

  virtual bool handleError(ESB::Error errorCode);

  virtual bool handleRemoteClose();

  virtual bool handleIdle();

  virtual bool handleRemove();

  virtual SOCKET socketDescriptor() const;

  virtual ESB::CleanupHandler *cleanupHandler();

  virtual const char *name() const;

  //
  // ES::HttpStream
  //

  virtual bool isPaused();

  virtual ESB::Error resume();

  virtual ESB::Error cancel();

  virtual ESB::Allocator &allocator();

  virtual const HttpRequest &request() const;

  virtual HttpRequest &request();

  virtual const HttpResponse &response() const;

  virtual HttpResponse &response();

  virtual void setContext(void *context);

  virtual void *context();

  virtual const void *context() const;

  virtual const ESB::SocketAddress &peerAddress() const;

  virtual const char *logAddress() const;

 private:
  // Disabled
  HttpServerSocket(const HttpServerSocket &);
  HttpServerSocket &operator=(const HttpServerSocket &);

  ESB::Error parseRequestHeaders();
  ESB::Error parseRequestBody();
  ESB::Error skipTrailer();
  ESB::Error formatResponseHeaders();
  ESB::Error formatResponseBody();
  ESB::Error flushBuffer();
  bool sendResponse();
  bool sendBadRequestResponse();
  bool sendInternalServerErrorResponse();

  int _state;
  int _bodyBytesWritten;
  int _requestsPerConnection;
  HttpServerStack &_stack;
  HttpServerHandler &_handler;
  HttpServerTransaction *_transaction;
  HttpServerCounters &_counters;
  ESB::CleanupHandler &_cleanupHandler;
  ESB::Buffer *_recvBuffer;
  ESB::Buffer *_sendBuffer;
  ESB::ConnectedTCPSocket _socket;
};

}  // namespace ES

#endif
