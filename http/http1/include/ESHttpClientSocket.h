#ifndef ES_HTTP_CLIENT_SOCKET_H
#define ES_HTTP_CLIENT_SOCKET_H

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

#ifndef ESB_CONNECTED_TCP_SOCKET_H
#include <ESBConnectedTCPSocket.h>
#endif

#ifndef ES_HTTP_CLIENT_COUNTERS_H
#include <ESHttpClientCounters.h>
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
class HttpClientSocket : public ESB::MultiplexedSocket, public HttpClientStream {
 public:
  /** Constructor
   */
  HttpClientSocket(HttpClientHandler &handler, HttpMultiplexerExtended &multiplexer, ESB::SocketAddress &peerAddress,
                   HttpClientCounters &counters, ESB::CleanupHandler &cleanupHandler);

  /** Destructor.
   */
  virtual ~HttpClientSocket();

  /** Reset the client socket
   *
   * @param reused true if the socket is being reused for a new transaction
   * @param transaction The client transaction object.  Many client transactions
   *  can be carried across the same http client socket with connection reuse.
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error reset(bool reused, HttpClientTransaction *transaction);

  inline void close() { _socket.close(); }

  inline ESB::Error connect() { return _socket.connect(); }

  inline bool isConnected() { return _socket.isConnected(); }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept { return allocator.allocate(size); }

  static inline void SetReuseConnections(bool reuseConnections) { _ReuseConnections = reuseConnections; }

  static inline bool GetReuseConnections() { return _ReuseConnections; }

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
  // ES::HttpClientStream
  //

  virtual ESB::Error sendRequestBody(unsigned const char *chunk, ESB::UInt32 bytesOffered, ESB::UInt32 *bytesConsumed);
  virtual ESB::Error responseBodyAvailable(ESB::UInt32 *bytesAvailable, ESB::UInt32 *bufferOffset);
  virtual ESB::Error readResponseBody(unsigned char *chunk, ESB::UInt32 bytesRequested, ESB::UInt32 bufferOffset);

  //
  // ESB::EmbeddedMapElement (for connection pool lookups)
  //

  virtual const void *key() const { return &_socket.peerAddress(); }

 private:
  // Disabled
  HttpClientSocket(const HttpClientSocket &);
  HttpClientSocket &operator=(const HttpClientSocket &);

  ESB::Error parseResponseHeaders();
  ESB::Error parseResponseBody();
  ESB::Error formatRequestHeaders();
  ESB::Error formatRequestBody();
  ESB::Error formatRequestBody(unsigned const char *chunk, ESB::UInt32 bytesOffered, ESB::UInt32 *bytesConsumed);
  ESB::Error currentChunkBytesAvailable(ESB::UInt32 *bytesAvailable, ESB::UInt32 *bufferOffset);
  ESB::Error formatStartChunk(ESB::UInt32 chunkSize, ESB::UInt32 *maxChunkSize);
  ESB::Error formatEndChunk();
  ESB::Error formatEndBody();
  ESB::Error fillReceiveBuffer();
  ESB::Error flushSendBuffer();

  int _state;
  int _bodyBytesWritten;
  HttpMultiplexerExtended &_multiplexer;
  HttpClientHandler &_handler;
  HttpClientTransaction *_transaction;
  HttpClientCounters &_counters;
  ESB::CleanupHandler &_cleanupHandler;
  ESB::Buffer *_recvBuffer;
  ESB::Buffer *_sendBuffer;
  ESB::ConnectedTCPSocket _socket;
  static bool _ReuseConnections;
};

}  // namespace ES

#endif
