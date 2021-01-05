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

HttpClientSocketFactory::HttpClientSocketFactory(HttpMultiplexerExtended &multiplexer, HttpClientHandler &handler,
                                                 HttpClientCounters &counters, ESB::Allocator &allocator)
    : _multiplexer(multiplexer),
      _handler(handler),
      _counters(counters),
      _allocator(allocator),
      _connectionPool(multiplexer.multiplexer().name(), HttpConfig::Instance().connectionPoolBuckets(), 0),
      _deconstructedHttpSockets(),
      _cleanupHandler(*this) {}

HttpClientSocketFactory::~HttpClientSocketFactory() {
  _connectionPool.clear();

  for (ESB::EmbeddedListElement *head = _deconstructedHttpSockets.removeFirst(); head;
       head = _deconstructedHttpSockets.removeFirst()) {
    _allocator.deallocate(head);
  }
}

HttpClientSocket *HttpClientSocketFactory::create(HttpClientTransaction *transaction) {
  if (!transaction) {
    return NULL;
  }

  char hostname[ESB_MAX_HOSTNAME + 1];
  ESB::UInt16 port = 0U;
  bool secure = false;

  ESB::Error error = transaction->request().parsePeerAddress(hostname, sizeof(hostname) - 1, &port, &secure);
  if (ESB_SUCCESS != error) {
    if (ESB_DEBUG_LOGGABLE) {
      char presentationAddress[ESB_IPV6_PRESENTATION_SIZE];
      transaction->peerAddress().presentationAddress(presentationAddress, sizeof(presentationAddress));
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot connect to [%s:%u]", name(), presentationAddress,
                          transaction->peerAddress().port());
    }
    return NULL;
  }

  ESB::ConnectedSocket *connection = NULL;
  bool reused = false;

  if (secure) {
    // TODO reduce the number of hostname copies (parsePeerAddr -> hostname, hostname -> hostAddress, hostAddress -> ClientTLSConnection)
    ESB::HostAddress hostAddress(hostname, transaction->peerAddress());
    error = _connectionPool.acquireTLSSocket(hostAddress, &connection, &reused);
  } else {
    error = _connectionPool.acquireClearSocket(transaction->peerAddress(), &connection, &reused);
  }

  if (ESB_SUCCESS != error) {
    if (ESB_DEBUG_LOGGABLE) {
      char presentationAddress[ESB_IPV6_PRESENTATION_SIZE];
      transaction->peerAddress().presentationAddress(presentationAddress, sizeof(presentationAddress));
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot connect to [%s:%u]", name(), presentationAddress,
                          transaction->peerAddress().port());
    }
    return NULL;
  }

  ESB::EmbeddedListElement *memory = _deconstructedHttpSockets.removeLast();
  HttpClientSocket *httpSocket =
      memory ? new (memory) HttpClientSocket(connection, _handler, _multiplexer, _counters, _cleanupHandler)
             : new (_allocator) HttpClientSocket(connection, _handler, _multiplexer, _counters, _cleanupHandler);

  if (!httpSocket) {
    if (ESB_ERROR_LOGGABLE) {
      char presentationAddress[ESB_IPV6_PRESENTATION_SIZE];
      transaction->peerAddress().presentationAddress(presentationAddress, sizeof(presentationAddress));
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] cannot connect to [%s:%u]", name(), presentationAddress,
                          transaction->peerAddress().port());
    }
    return NULL;
  }

  if (reused) {
    ESB_LOG_DEBUG("[%s] connection reused", httpSocket->logAddress());
  } else {
    ESB_LOG_DEBUG("[%s] connection created", httpSocket->logAddress());
  }

  httpSocket->reset(reused, transaction);
  return httpSocket;
}

void HttpClientSocketFactory::release(HttpClientSocket *httpSocket) {
  if (!httpSocket) {
    return;
  }

  if (httpSocket->connected()) {
    ESB_LOG_DEBUG("[%s] returned connection to pool", httpSocket->logAddress());
  } else {
    ESB_LOG_DEBUG("[%s] did not return connection to pool", httpSocket->logAddress());
  }

  _connectionPool.release(httpSocket->socket());
  httpSocket->~HttpClientSocket();
  _deconstructedHttpSockets.addLast(httpSocket);
}

ESB::Error HttpClientSocketFactory::executeClientTransaction(HttpClientTransaction *transaction) {
  // don't uncomment the end handler callbacks until after the processing goes
  // asynch
  assert(transaction);
  if (!transaction) {
    return ESB_NULL_POINTER;
  }

  transaction->setStartTime();

  HttpClientSocket *socket = create(transaction);

  if (!socket) {
    _counters.getFailures()->record(transaction->startTime(), ESB::Date::Now());
    // transaction->getHandler()->end(transaction,
    //                               HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
    ESB_LOG_CRITICAL("[%s] cannot allocate new client socket", name());
    return 0;
  }

  if (socket->connected()) {
    ESB::Error error = _handler.beginTransaction(_multiplexer, *socket);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborted transaction immediately after connecting", socket->name());
      return ESB_AGAIN == error ? ESB_OTHER_ERROR : error;
    }
  } else {
    ESB::Error error = socket->connect();

    if (ESB_SUCCESS != error) {
      _counters.getFailures()->record(transaction->startTime(), ESB::Date::Now());
      ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot connect", socket->logAddress());
      // transaction->getHandler()->end(transaction,
      //                               HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
      socket->close();
      release(socket);
      return error;
    }
  }

  ESB::Error error = _multiplexer.multiplexer().addMultiplexedSocket(socket);

  if (ESB_SUCCESS != error) {
    _counters.getFailures()->record(transaction->startTime(), ESB::Date::Now());
    ESB_LOG_CRITICAL_ERRNO(error, "[%s] Cannot add client socket to multiplexer", socket->logAddress());
    socket->close();
    // transaction->getHandler()->end(transaction,
    //                               HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
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
