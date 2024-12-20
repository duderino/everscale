#ifndef ES_HTTP_CLIENT_SOCKET_FACTORY_H
#include <ESHttpClientSocketFactory.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ES_HTTP_CONFIG_H
#include <ESHttpConfig.h>
#endif

namespace ES {

class NullHttpClientStream : public HttpClientStream {
 public:
  virtual bool secure() const { assert(0 == "NullHttpClientStream called"); }
  virtual ESB::Error abort(bool updateMultiplexer) { assert(0 == "NullHttpClientStream called"); }
  virtual ESB::Error pauseRecv(bool updateMultiplexer) { assert(0 == "NullHttpClientStream called"); }
  virtual ESB::Error resumeRecv(bool updateMultiplexer) { assert(0 == "NullHttpClientStream called"); }
  virtual ESB::Error pauseSend(bool updateMultiplexer) { assert(0 == "NullHttpClientStream called"); }
  virtual ESB::Error resumeSend(bool updateMultiplexer) { assert(0 == "NullHttpClientStream called"); }
  virtual ESB::Allocator &allocator() { assert(0 == "NullHttpClientStream called"); }
  virtual const HttpRequest &request() const { assert(0 == "NullHttpClientStream called"); }
  virtual HttpRequest &request() { assert(0 == "NullHttpClientStream called"); }
  virtual const HttpResponse &response() const { assert(0 == "NullHttpClientStream called"); }
  virtual HttpResponse &response() { assert(0 == "NullHttpClientStream called"); }
  virtual void setContext(void *context) { assert(0 == "NullHttpClientStream called"); }
  virtual void *context() { assert(0 == "NullHttpClientStream called"); }
  virtual const void *context() const { assert(0 == "NullHttpClientStream called"); }
  virtual const ESB::SocketAddress &peerAddress() const { assert(0 == "NullHttpClientStream called"); }
  virtual const char *logAddress() const { assert(0 == "NullHttpClientStream called"); }
  virtual const ESB::Date &transactionStartTime() const { assert(0 == "NullHttpClientStream called"); }
  virtual ESB::Error sendRequestBody(const unsigned char *body, ESB::UInt64 bytesOffered, ESB::UInt64 *bytesConsumed) {
    assert(0 == "NullHttpClientStream called");
  }
  virtual ESB::Error responseBodyAvailable(ESB::UInt64 *bytesAvailable) { assert(0 == "NullHttpClientStream called"); }
  virtual ESB::Error readResponseBody(unsigned char *body, ESB::UInt64 bytesRequested, ESB::UInt64 *bytesRead) {
    assert(0 == "NullHttpClientStream called");
  }
};

NullHttpClientStream NullHttpClientStream;

HttpClientSocketFactory::HttpClientSocketFactory(HttpMultiplexerExtended &multiplexer, HttpClientHandler &handler,
                                                 HttpConnectionMetrics &connectionMetrics,
                                                 ESB::ClientTLSContextIndex &contextIndex, ESB::Allocator &allocator)
    : _multiplexer(multiplexer),
      _handler(handler),
      _connectionMetrics(connectionMetrics),
      _allocator(allocator),
      _connectionPool(multiplexer.multiplexer().name(), HttpConfig::Instance().connectionPoolBuckets(), 0,
                      contextIndex),
      _deconstructedHttpSockets(),
      _cleanupHandler(*this) {}

HttpClientSocketFactory::~HttpClientSocketFactory() {
  _connectionPool.clear();

  for (ESB::EmbeddedListElement *head = _deconstructedHttpSockets.removeFirst(); head;
       head = _deconstructedHttpSockets.removeFirst()) {
    _allocator.deallocate(head);
  }
}

ESB::Error HttpClientSocketFactory::create(HttpClientTransaction *transaction, HttpClientSocket **socket) {
  if (!transaction || !socket) {
    return ESB_NULL_POINTER;
  }

  ESB::ConnectedSocket *connection = NULL;
  bool reused = false;
  ESB::Error error = ESB_OTHER_ERROR;

  if (transaction->peerAddress().type() != ESB::SocketAddress::TLS) {
    error = _connectionPool.acquireClearSocket(transaction->peerAddress(), &connection, &reused);
  } else {
    char hostname[ESB_MAX_HOSTNAME + 1];
    ESB::UInt16 port = 0U;
    bool secure = false;  // ignore this in favor of the socket transport type

    error = transaction->request().parsePeerAddress(hostname, sizeof(hostname) - 1, &port, &secure);
    if (ESB_SUCCESS != error) {
      if (ESB_DEBUG_LOGGABLE) {
        char presentationAddress[ESB_IPV6_PRESENTATION_SIZE];
        transaction->peerAddress().presentationAddress(presentationAddress, sizeof(presentationAddress));
        ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot connect to [%s:%u]", name(), presentationAddress,
                            transaction->peerAddress().port());
      }
      return error;
    }

    error = _connectionPool.acquireTLSSocket(hostname, transaction->peerAddress(), &connection, &reused);
  }

  if (ESB_SUCCESS != error) {
    if (ESB_DEBUG_LOGGABLE) {
      char presentationAddress[ESB_IPV6_PRESENTATION_SIZE];
      transaction->peerAddress().presentationAddress(presentationAddress, sizeof(presentationAddress));
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot connect to [%s:%u]", name(), presentationAddress,
                          transaction->peerAddress().port());
    }
    return error;
  }

  HttpClientSocket *httpSocket = NULL;

  {
    HttpClientSocket *memory = (HttpClientSocket *)_deconstructedHttpSockets.removeLast();
    if (memory) {
      httpSocket = new (memory)
          HttpClientSocket(reused, transaction, connection, _handler, _multiplexer, _connectionMetrics, _cleanupHandler);
    } else {
      httpSocket = new (_allocator)
          HttpClientSocket(reused, transaction, connection, _handler, _multiplexer, _connectionMetrics, _cleanupHandler);
    }
  }

  if (!httpSocket) {
    if (ESB_ERROR_LOGGABLE) {
      char presentationAddress[ESB_IPV6_PRESENTATION_SIZE];
      transaction->peerAddress().presentationAddress(presentationAddress, sizeof(presentationAddress));
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] cannot connect to [%s:%u]", name(), presentationAddress,
                          transaction->peerAddress().port());
    }
    return ESB_OUT_OF_MEMORY;
  }

  if (reused) {
    ESB_LOG_DEBUG("[%s] connection reused", httpSocket->logAddress());
  } else {
    ESB_LOG_DEBUG("[%s] connection created", httpSocket->logAddress());
  }

  *socket = httpSocket;
  return ESB_SUCCESS;
}

void HttpClientSocketFactory::release(HttpClientSocket *httpSocket) {
  if (!httpSocket) {
    return;
  }

  if (httpSocket->connected()) {
    ESB_LOG_DEBUG("[%s] returned connection to pool", httpSocket->logAddress());
  } else {
    ESB_LOG_DEBUG("[%s] did not return connection to pool", httpSocket->logAddress());
    httpSocket->close();
  }

  _connectionPool.release(httpSocket->socket());
  httpSocket->~HttpClientSocket();
  _deconstructedHttpSockets.addLast(httpSocket);
}

ESB::Error HttpClientSocketFactory::executeClientTransaction(HttpClientTransaction *transaction) {
  // don't uncomment the end handler callbacks until after the processing goes asynch
  assert(transaction);
  if (!transaction) {
    return ESB_NULL_POINTER;
  }

  transaction->setStartTime();

  HttpClientSocket *socket = NULL;
  ESB::Error error = create(transaction, &socket);
  if (ESB_SUCCESS != error) {
    _handler.endTransaction(_multiplexer, NullHttpClientStream, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
    ESB_LOG_CRITICAL_ERRNO(error, "[%s] cannot create new client socket", name());
    return error;
  }

  if (socket->connected()) {
    error = _handler.beginTransaction(_multiplexer, *socket);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborted transaction immediately after connecting", socket->name());
      socket->close();
      socket->clearTransaction();
      release(socket);
      return ESB_AGAIN == error ? ESB_OTHER_ERROR : error;
    }
  } else {
    error = socket->connect();
    if (ESB_SUCCESS != error) {
      ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot connect", socket->logAddress());
      _handler.endTransaction(_multiplexer, *socket, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
      socket->close();
      socket->clearTransaction();
      release(socket);
      return error;
    }
  }

  error = _multiplexer.multiplexer().addMultiplexedSocket(socket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "[%s] Cannot add client socket to multiplexer", socket->logAddress());
    _handler.endTransaction(_multiplexer, *socket, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
    socket->close();
    socket->clearTransaction();
    release(socket);
    return error;
  }

  return ESB_SUCCESS;
}

const char *HttpClientSocketFactory::name() const { return _multiplexer.multiplexer().name(); }

HttpClientSocketFactory::CleanupHandler::CleanupHandler(HttpClientSocketFactory &factory)
    : ESB::CleanupHandler(), _factory(factory) {}

HttpClientSocketFactory::CleanupHandler::~CleanupHandler() {}

void HttpClientSocketFactory::CleanupHandler::destroy(ESB::Object *object) {
  _factory.release((HttpClientSocket *)object);
}

}  // namespace ES
