#ifndef ES_HTTP_SERVER_SOCKET_H
#define ES_HTTP_SERVER_SOCKET_H

#ifndef ES_HTTP_SOCKET_H
#include <ESHttpSocket.h>
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
class HttpServerSocket : public HttpSocket, public HttpServerStream {
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

  virtual void handleError(ESB::Error errorCode);

  virtual void handleRemoteClose();

  virtual void handleIdle();

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

  // State machine argument flags

#define SERVER_INITIAL_FILL_RECV_BUFFER (1 << 0)
#define SERVER_INITIAL_DRAIN_SEND_BUFFER (1 << 1)
#define SERVER_UPDATE_MULTIPLEXER (1 << 2)
#define SERVER_ADVANCE_RECV (1 << 3)
#define SERVER_ADVANCE_SEND (1 << 4)

  // Socket States

#define SERVER_INACTIVE (1 << 0)
#define SERVER_TRANSACTION_BEGIN (1 << 1)
#define SERVER_PARSING_HEADERS (1 << 2)
#define SERVER_PARSING_BODY (1 << 3)
#define SERVER_SKIPPING_TRAILER (1 << 4)
#define SERVER_FORMATTING_HEADERS (1 << 5)
#define SERVER_FORMATTING_BODY (1 << 6)
#define SERVER_FLUSHING_BODY (1 << 7)
#define SERVER_TRANSACTION_END (1 << 8)

  // Other socket flags affecting control flow

#define SERVER_CANNOT_REUSE_CONNECTION (1 << 9)
#define SERVER_RECV_PAUSED (1 << 10)
#define SERVER_SEND_PAUSED (1 << 11)
#define SERVER_ABORTED (1 << 12)

  // Useful socket flag masks

#define SERVER_STATE_MASK                                                                                \
  (SERVER_INACTIVE | SERVER_TRANSACTION_BEGIN | SERVER_PARSING_HEADERS | SERVER_PARSING_BODY |           \
   SERVER_SKIPPING_TRAILER | SERVER_FORMATTING_HEADERS | SERVER_FORMATTING_BODY | SERVER_FLUSHING_BODY | \
   SERVER_TRANSACTION_END)
#define SERVER_FLAG_MASK (~SERVER_STATE_MASK)
#define SERVER_RECV_STATE_MASK (SERVER_PARSING_BODY | SERVER_PARSING_HEADERS | SERVER_SKIPPING_TRAILER)
#define SERVER_SEND_STATE_MASK (SERVER_FORMATTING_HEADERS | SERVER_FORMATTING_BODY | SERVER_FLUSHING_BODY)

  /**
   * Advance the socket's state machine until it finishes, needs more data (ESB_AGAIN), or becomes application limited
   * (ESB_PAUSE).
   *
   * @return ESB_SUCCESS if the state machine finishes, ESB_AGAIN if the state machine needs more data from the socket,
   * ESB_PAUSE if the state machine needs more data from the application (e.g., a different socket), or another error
   * code if the state machine encountered a terminal error.  Terminal errors should generally be handled by closing the
   * connection.
   */
  ESB::Error advanceStateMachine(HttpServerHandler &handler, int flags);
  ESB::Error stateBeginTransaction();
  ESB::Error stateReceiveRequestHeaders();
  ESB::Error stateReceiveRequestBody(HttpServerHandler &handler);
  ESB::Error stateSkipTrailer();
  ESB::Error stateSendResponseHeaders();
  ESB::Error stateSendResponseBody(HttpServerHandler &handler);
  ESB::Error stateFlushResponseBody();
  ESB::Error stateEndTransaction();

  void stateTransition(int state);

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

  inline void addFlag(int flag) {
    assert(flag & SERVER_FLAG_MASK);
    assert(!(flag & SERVER_STATE_MASK));
    _state |= flag;
  }

  inline void clearFlag(int flag) {
    assert(flag & SERVER_FLAG_MASK);
    assert(!(flag & SERVER_STATE_MASK));
    _state &= ~flag;
  }

  // TODO use or remove
  ESB::Error updateInterestList(bool updateMultiplexer);

  ESB::Error currentChunkBytesAvailable(ESB::UInt32 *bytesAvailable, ESB::UInt32 *bufferOffset);
  ESB::Error formatStartChunk(ESB::UInt32 chunkSize, ESB::UInt32 *maxChunkSize);
  ESB::Error formatEndChunk();
  ESB::Error fillReceiveBuffer();
  ESB::Error flushSendBuffer();
  ESB::Error setResponse(int statusCode, const char *reasonPhrase);

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
