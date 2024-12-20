#ifndef ES_HTTP_SERVER_SOCKET_H
#define ES_HTTP_SERVER_SOCKET_H

#ifndef ES_HTTP_SOCKET_H
#include <ESHttpSocket.h>
#endif

#ifndef ESB_CONNECTED_SOCKET_H
#include <ESBConnectedSocket.h>
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

#ifndef ES_HTTP_CONNECTION_METRICS_H
#include <ESHttpConnectionMetrics.h>
#endif

namespace ES {

/** A socket that receives and echoes back HTTP requests
 */
class HttpServerSocket : public HttpSocket, public HttpServerStream {
 public:
  /** Constructor
   */
  HttpServerSocket(ESB::ConnectedSocket *socket, HttpServerHandler &handler, HttpMultiplexerExtended &multiplexer,
                   HttpConnectionMetrics &connectionMetrics, ESB::CleanupHandler &cleanupHandler);

  /** Destructor.
   */
  virtual ~HttpServerSocket();

  virtual const void *key() const;

  inline ESB::ConnectedSocket *socket() { return _socket; }

  //
  // ESB::MultiplexedSocket
  //

  virtual bool permanent();

  virtual bool wantAccept();

  virtual bool wantConnect();

  virtual bool wantRead();

  virtual bool wantWrite();

  virtual ESB::Error handleAccept();

  virtual ESB::Error handleConnect();

  virtual ESB::Error handleReadable();

  virtual ESB::Error handleWritable();

  virtual void handleError(ESB::Error errorCode);

  virtual void handleRemoteClose();

  virtual void handleIdle();

  virtual void handleRemove();

  virtual SOCKET socketDescriptor() const;

  virtual ESB::CleanupHandler *cleanupHandler();

  virtual const char *name() const;

  virtual void markDead();

  virtual bool dead() const;

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

  virtual const ESB::Date &transactionStartTime() const;

  //
  // ES::HttpServerStream
  //

  virtual bool secure() const;

  virtual ESB::Error sendEmptyResponse(int statusCode, const char *reasonPhrase);
  virtual ESB::Error sendResponse(const HttpResponse &response,
                                  HttpMessage::HeaderCopyFilter filter = HttpMessage::HeaderCopyAll);

  virtual ESB::Error sendResponseBody(unsigned const char *chunk, ESB::UInt64 bytesOffered, ESB::UInt64 *bytesConsumed);
  virtual ESB::Error requestBodyAvailable(ESB::UInt64 *bytesAvailable);
  virtual ESB::Error readRequestBody(unsigned char *chunk, ESB::UInt64 bytesRequested, ESB::UInt64 *bytesRead);

 private:
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
#define SERVER_LAST_CHUNK_RECEIVED (1 << 13)
#define SERVER_DEAD (1 << 14)

  // Useful socket flag masks

#define SERVER_STATE_MASK                                                                                \
  (SERVER_INACTIVE | SERVER_TRANSACTION_BEGIN | SERVER_PARSING_HEADERS | SERVER_PARSING_BODY |           \
   SERVER_SKIPPING_TRAILER | SERVER_FORMATTING_HEADERS | SERVER_FORMATTING_BODY | SERVER_FLUSHING_BODY | \
   SERVER_TRANSACTION_END)
#define SERVER_FLAG_MASK (~SERVER_STATE_MASK)
#define SERVER_RECV_STATE_MASK (SERVER_PARSING_BODY | SERVER_PARSING_HEADERS | SERVER_SKIPPING_TRAILER)
#define SERVER_SEND_STATE_MASK (SERVER_FORMATTING_HEADERS | SERVER_FORMATTING_BODY | SERVER_FLUSHING_BODY)

  const char *describeState() const;

  const char *describeFlags() const;

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

  ESB::Error currentChunkBytesAvailable(ESB::UInt64 *bytesAvailable);
  ESB::Error formatStartChunk(ESB::UInt64 chunkSize, ESB::UInt64 *maxChunkSize);
  ESB::Error formatEndChunk();
  ESB::Error fillReceiveBuffer();
  ESB::Error flushSendBuffer();
  ESB::Error setResponse(int statusCode, const char *reasonPhrase);

  int _state;
  int _requestsPerConnection;
  ESB::UInt64 _bodyBytesWritten;
  ESB::UInt64 _bytesAvailable;
  HttpMultiplexerExtended &_multiplexer;
  HttpServerHandler &_handler;
  HttpConnectionMetrics &_connectionMetrics;
  HttpServerTransaction *_transaction;
  ESB::CleanupHandler &_cleanupHandler;
  ESB::Buffer *_recvBuffer;
  ESB::Buffer *_sendBuffer;
  ESB::ConnectedSocket *_socket;

  ESB_DEFAULT_FUNCS(HttpServerSocket);
};

}  // namespace ES

#endif
