#ifndef ES_HTTP_CLIENT_SOCKET_H
#define ES_HTTP_CLIENT_SOCKET_H

#ifndef ES_HTTP_SOCKET_H
#include <ESHttpSocket.h>
#endif

#ifndef ESB_CONNECTED_SOCKET_H
#include <ESBConnectedSocket.h>
#endif

#ifndef ES_HTTP_CONNECTION_METRICS_H
#include <ESHttpConnectionMetrics.h>
#endif

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

#ifndef ES_HTTP_CONNECTION_POOL_H
#include <ESHttpConnectionPool.h>
#endif

#ifndef ES_HTTP_CLIENT_STREAM_H
#include <ESHttpClientStream.h>
#endif

#ifndef ES_HTTP_MULTIPLEXER_EXTENDED_H
#include <ESHttpMultiplexerExtended.h>
#endif

namespace ES {

/** A socket that receives and echoes back HTTP requests
 *
 * TODO implement idle check
 */
class HttpClientSocket : public HttpSocket, public HttpClientStream {
 public:
  /** Constructor
   */
  HttpClientSocket(bool reused, HttpClientTransaction *transaction, ESB::ConnectedSocket *socket,
                   HttpClientHandler &handler, HttpMultiplexerExtended &multiplexer,  HttpConnectionMetrics &connectionMetrics,
                   ESB::CleanupHandler &cleanupHandler);

  /** Destructor.
   */
  virtual ~HttpClientSocket();

  inline void close() { _socket->close(); }

  inline ESB::Error connect() { return _socket->connect(); }

  inline bool connected() { return _socket->connected(); }

  inline ESB::ConnectedSocket *socket() { return _socket; }

  inline void clearTransaction() { _transaction = NULL; }

  static inline void SetReuseConnections(bool reuseConnections) { _ReuseConnections = reuseConnections; }

  static inline bool GetReuseConnections() { return _ReuseConnections; }

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
  // ES::HttpClientStream
  //

  virtual bool secure() const;

  virtual ESB::Error sendRequestBody(unsigned const char *chunk, ESB::UInt64 bytesOffered, ESB::UInt64 *bytesConsumed);
  virtual ESB::Error responseBodyAvailable(ESB::UInt64 *bytesAvailable);
  virtual ESB::Error readResponseBody(unsigned char *chunk, ESB::UInt64 bytesRequested, ESB::UInt64 *bytesRead);

  //
  // ESB::EmbeddedMapElement (for connection pool lookups)
  //

  virtual const void *key() const { return &_socket->peerAddress(); }

 private:
  // State machine argument flags

#define INITIAL_FILL_RECV_BUFFER (1 << 0)
#define INITIAL_DRAIN_SEND_BUFFER (1 << 1)
#define UPDATE_MULTIPLEXER (1 << 2)
#define ADVANCE_RECV (1 << 3)
#define ADVANCE_SEND (1 << 4)

  // Socket States

#define INACTIVE (1 << 0)
#define CONNECTING (1 << 1)
#define TRANSACTION_BEGIN (1 << 2)
#define FORMATTING_HEADERS (1 << 3)
#define FORMATTING_BODY (1 << 4)
#define FLUSHING_BODY (1 << 5)
#define PARSING_HEADERS (1 << 6)
#define PARSING_BODY (1 << 7)
#define TRANSACTION_END (1 << 8)

  // Other socket flags affecting control flow

#define FIRST_USE_AFTER_REUSE (1 << 9)
#define RECV_PAUSED (1 << 10)
#define SEND_PAUSED (1 << 11)
#define ABORTED (1 << 12)
#define LAST_CHUNK_RECEIVED (1 << 13)
#define DEAD (1 << 14)

  // Useful socket flag masks

#define STATE_MASK                                                                                    \
  (INACTIVE | CONNECTING | TRANSACTION_BEGIN | FORMATTING_HEADERS | FORMATTING_BODY | FLUSHING_BODY | \
   PARSING_HEADERS | PARSING_BODY | TRANSACTION_END)
#define FLAG_MASK (~STATE_MASK)
#define RECV_STATE_MASK (PARSING_BODY | PARSING_HEADERS)
#define SEND_STATE_MASK (FORMATTING_HEADERS | FORMATTING_BODY | FLUSHING_BODY)

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
  ESB::Error advanceStateMachine(HttpClientHandler &handler, int flags);
  ESB::Error stateBeginTransaction();
  ESB::Error stateSendRequestHeaders();
  ESB::Error stateSendRequestBody(HttpClientHandler &handler);
  ESB::Error stateFlushRequestBody();
  ESB::Error stateReceiveResponseHeaders();
  ESB::Error stateReceiveResponseBody(HttpClientHandler &handler);

  void stateTransition(int state);

  inline void addFlag(int flag) {
    assert(flag & FLAG_MASK);
    assert(!(flag & STATE_MASK));
    _state |= flag;
  }

  inline void clearFlag(int flag) {
    assert(flag & FLAG_MASK);
    assert(!(flag & STATE_MASK));
    _state &= ~flag;
  }

  ESB::Error currentChunkBytesAvailable(ESB::UInt64 *bytesAvailable);
  ESB::Error formatStartChunk(ESB::UInt64 chunkSize, ESB::UInt64 *maxChunkSize);
  ESB::Error formatEndChunk();
  ESB::Error fillReceiveBuffer();
  ESB::Error flushSendBuffer();

  int _state;
  int _requestsPerConnection;
  ESB::UInt64 _bodyBytesWritten;
  ESB::UInt64 _bytesAvailable;
  HttpMultiplexerExtended &_multiplexer;
  HttpClientHandler &_handler;
  HttpConnectionMetrics &_connectionMetrics;
  HttpClientTransaction *_transaction;
  ESB::CleanupHandler &_cleanupHandler;
  ESB::Buffer *_recvBuffer;
  ESB::Buffer *_sendBuffer;
  ESB::ConnectedSocket *_socket;
  static bool _ReuseConnections;

  ESB_DEFAULT_FUNCS(HttpClientSocket);
};

}  // namespace ES

#endif
