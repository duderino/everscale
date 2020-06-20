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

#ifndef ES_HTTP_MULTIPLEXER_EXTENDED_H
#include <ESHttpMultiplexerExtended.h>
#endif

#ifndef ES_HTTP_SERVER_STREAM_H
#include <ESHttpServerStream.h>
#endif

namespace ES {

/** A socket that receives and echoes back HTTP requests
 *
 * TODO implement idle check
 */
class HttpServerSocket : public ESB::MultiplexedSocket, public HttpServerStream {
 public:
  /** Constructor
   */
  HttpServerSocket(HttpServerHandler &handler, HttpMultiplexerExtended &multiplexer, HttpServerCounters &counters,
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
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept { return allocator.allocate(size); }

  //
  // ESB::MultiplexedSocket
  //

  virtual bool wantAccept();

  virtual bool wantConnect();

  virtual bool wantRead();

  virtual bool wantWrite();

  virtual bool isIdle();

  virtual ESB::Error handleAccept();

  virtual ESB::Error handleConnect();

  virtual ESB::Error handleReadable();

  virtual ESB::Error handleWritable();

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

  virtual ESB::Error abort(bool updateMultiplexer = true);
  virtual ESB::Error pauseRecv(bool updateMultiplexer = true);
  virtual ESB::Error resumeRecv(bool updateMultiplexer = true);
  virtual ESB::Error pauseSend(bool updateMultiplexer = true);
  virtual ESB::Error resumeSend(bool updateMultiplexer = true);

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

  //
  // ES::HttpServerStream
  //

  virtual ESB::Error sendEmptyResponse(int statusCode, const char *reasonPhrase);
  virtual ESB::Error sendResponse(const HttpResponse &response);

  virtual ESB::Error sendResponseBody(unsigned const char *chunk, ESB::UInt32 bytesOffered, ESB::UInt32 *bytesConsumed);
  virtual ESB::Error requestBodyAvailable(ESB::UInt32 *bytesAvailable, ESB::UInt32 *bufferOffset);
  virtual ESB::Error readRequestBody(unsigned char *chunk, ESB::UInt32 bytesRequested, ESB::UInt32 bufferOffset);

 private:
  // Disabled
  HttpServerSocket(const HttpServerSocket &);
  HttpServerSocket &operator=(const HttpServerSocket &);

  int stateMask();
  int flagMask();
  void stateTransition(int state);
  void setFlag(int flag);
  void unsetFlag(int flag);

  /**
   * Perform a state transition only if the handler has not already performed one.
   *
   * @param ifState If the socket is in this state, perform the transition.  Else skip
   * @param newState The new state to transition into, if the condition has been met.
   */
  inline void conditionalStateTransition(int ifState, int newState) {
    if (_state & ifState) {
      stateTransition(newState);
    }
  }

  ESB::Error parseRequestHeaders();
  ESB::Error parseRequestBody();
  ESB::Error skipTrailer();
  ESB::Error formatResponseHeaders();
  ESB::Error formatResponseBody();
  ESB::Error formatResponseBody(unsigned const char *chunk, ESB::UInt32 bytesOffered, ESB::UInt32 *bytesConsumed);
  ESB::Error currentChunkBytesAvailable(ESB::UInt32 *bytesAvailable, ESB::UInt32 *bufferOffset);
  ESB::Error formatStartChunk(ESB::UInt32 chunkSize, ESB::UInt32 *maxChunkSize);
  ESB::Error formatEndChunk();
  ESB::Error formatEndBody();
  ESB::Error fillReceiveBuffer();
  ESB::Error flushSendBuffer();
  ESB::Error sendResponse();
  ESB::Error sendBadRequestResponse();
  ESB::Error sendInternalServerErrorResponse();

  int _state;
  int _bodyBytesWritten;
  int _requestsPerConnection;
  HttpMultiplexerExtended &_multiplexer;
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
