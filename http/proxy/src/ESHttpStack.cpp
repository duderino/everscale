#ifndef ES_HTTP_STACK_H
#include <ESHttpStack.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_PROCESS_LIMITS_H
#include <ESBProcessLimits.h>
#endif

#include <errno.h>

namespace ES {

HttpStack::HttpStack(HttpServerHandler *serverHandler,
                     ESB::DnsClient *dnsClient, int port, int threads,
                     HttpClientCounters *clientCounters,
                     HttpServerCounters *serverCounters, ESB::Logger *logger)
    : _port(0 > port ? 80 : port),
      _threads(0 >= threads ? 1 : threads),
      _state(ES_HTTP_STACK_IS_DESTROYED),
      _dnsClient(dnsClient),
      _logger(logger ? logger : ESB::NullLogger::GetInstance()),
      _serverHandler(serverHandler),
      _serverCounters(serverCounters),
      _clientCounters(clientCounters),
      _discardAllocator(4000, ESB::SystemAllocator::GetInstance()),
      _rootAllocator(&_discardAllocator),
      _rootAllocatorCleanupHandler(&_rootAllocator),
      _epollFactory("HttpMultiplexer", _logger, &_rootAllocator),
      _listeningSocket(_port, ESB_UINT16_MAX, false),
      _serverSocketFactory(_serverCounters, _logger),
      _clientSocketFactory(_clientCounters, _logger),
      _clientTransactionFactory(),
      _dispatcher(ESB::ProcessLimits::GetSocketSoftMax(), _threads, &_epollFactory, &_rootAllocator,"HttpDispatcher", _logger) {}

HttpStack::HttpStack(ESB::DnsClient *dnsClient, int threads,
                     HttpClientCounters *clientCounters, ESB::Logger *logger)
    : _port(-1),
      _threads(0 >= threads ? 1 : threads),
      _state(ES_HTTP_STACK_IS_DESTROYED),
      _dnsClient(dnsClient),
      _logger(logger ? logger : ESB::NullLogger::GetInstance()),
      _serverHandler(0),
      _serverCounters(0),
      _clientCounters(clientCounters),
      _discardAllocator(4000, ESB::SystemAllocator::GetInstance()),
      _rootAllocator(&_discardAllocator),
      _rootAllocatorCleanupHandler(&_rootAllocator),
      _epollFactory("HttpMultiplexer", _logger, &_rootAllocator),
      _listeningSocket(_port, ESB_UINT16_MAX, false),
      _serverSocketFactory(_serverCounters, _logger),
      _clientSocketFactory(_clientCounters, _logger),
      _clientTransactionFactory(),
      _dispatcher(ESB::ProcessLimits::GetSocketSoftMax(), _threads, &_epollFactory, &_rootAllocator,
                  "HttpDispatcher", _logger) {}

HttpStack::~HttpStack() {}

ESB::Error HttpStack::initialize() {
  assert(ES_HTTP_STACK_IS_DESTROYED == _state);

  if (_logger->isLoggable(ESB::Logger::Notice)) {
    _logger->log(ESB::Logger::Notice, __FILE__, __LINE__,"[stack] Maximum sockets %d", ESB::ProcessLimits::GetSocketSoftMax());
  }

  ESB::Error error = _rootAllocator.initialize();

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[stack] Cannot initialize root allocator: %s", buffer);
    }

    return error;
  }

  error = _clientSocketFactory.initialize();

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[stack] Cannot initialize client socket factory: %s",
                   buffer);
    }

    return error;
  }

  error = _clientTransactionFactory.initialize();

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[stack] Cannot initialize client transaction factory: %s",
                   buffer);
    }

    return error;
  }

  if (0 > _port || 0 == _serverHandler) {
    _state = ES_HTTP_STACK_IS_INITIALIZED;

    return ESB_SUCCESS;
  }

  error = _listeningSocket.bind();

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[stack] Cannot bind to port %d: %s", _port, buffer);
    }

    return error;
  }

  error = _listeningSocket.listen();

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[stack] Cannot listen on port %d: %s", _port, buffer);
    }

    return error;
  }

  error = _serverSocketFactory.initialize();

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[stack] Cannot initialize server socket factory: %s",
                   buffer);
    }

    return error;
  }

  _state = ES_HTTP_STACK_IS_INITIALIZED;

  return ESB_SUCCESS;
}

ESB::Error HttpStack::start() {
  assert(ES_HTTP_STACK_IS_INITIALIZED == _state);

  ESB::Error error = _dispatcher.start();

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[stack] Cannot start multiplexer dispatcher: %s", buffer);
    }

    return error;
  }

  if (0 > _port || 0 == _serverHandler) {
    _state = ES_HTTP_STACK_IS_STARTED;

    if (_logger->isLoggable(ESB::Logger::Notice)) {
      _logger->log(ESB::Logger::Notice, __FILE__, __LINE__, "[stack] started");
    }

    return ESB_SUCCESS;
  }

  HttpListeningSocket *socket = 0;

  for (int i = 0; i < _threads; ++i) {
    socket = new (&_rootAllocator) HttpListeningSocket(
        _serverHandler, &_listeningSocket, &_dispatcher, &_serverSocketFactory,
        _logger, &_rootAllocatorCleanupHandler, _serverCounters);

    if (!socket) {
      if (_logger->isLoggable(ESB::Logger::Critical)) {
        _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                     "[stack] Cannot allocate new listening socket");
      }

      return ESB_OUT_OF_MEMORY;
    }

    error = _dispatcher.addMultiplexedSocket(i, socket);

    if (ESB_SUCCESS != error) {
      if (_logger->isLoggable(ESB::Logger::Critical)) {
        char buffer[100];

        ESB::DescribeError(error, buffer, sizeof(buffer));

        _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                     "[stack] Cannot add listening socket to multiplexer: %s",
                     buffer);
      }

      return error;
    }
  }

  _state = ES_HTTP_STACK_IS_STARTED;

  if (_logger->isLoggable(ESB::Logger::Notice)) {
    _logger->log(ESB::Logger::Notice, __FILE__, __LINE__, "[stack] started");
  }

  return ESB_SUCCESS;
}

ESB::Error HttpStack::stop() {
  assert(ES_HTTP_STACK_IS_STARTED == _state);

  _state = ES_HTTP_STACK_IS_STOPPED;

  if (_logger->isLoggable(ESB::Logger::Notice)) {
    _logger->log(ESB::Logger::Notice, __FILE__, __LINE__, "[stack] stopping");
  }

  _dispatcher.stop();

  if (_logger->isLoggable(ESB::Logger::Notice)) {
    _logger->log(ESB::Logger::Notice, __FILE__, __LINE__, "[stack] stopped");
  }

  return ESB_SUCCESS;
}

void HttpStack::destroy() {
  assert(ES_HTTP_STACK_IS_STOPPED == _state);

  _state = ES_HTTP_STACK_IS_DESTROYED;

  _clientSocketFactory.destroy();
  _clientTransactionFactory.destroy();
  _serverSocketFactory.destroy();
}

HttpClientTransaction *HttpStack::createClientTransaction(
    HttpClientHandler *clientHandler) {
  if (ES_HTTP_STACK_IS_DESTROYED == _state) {
    return 0;
  }

  HttpClientTransaction *transaction =
      _clientTransactionFactory.create(clientHandler);

  if (!transaction) {
    if (_logger->isLoggable(ESB::Logger::Critical)) {
      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[stack] Cannot allocate new transaction");
    }

    return 0;
  }

  return transaction;
}

ESB::Error HttpStack::executeClientTransaction(
    HttpClientTransaction *transaction) {
  if (!transaction) {
    return ESB_NULL_POINTER;
  }

  if (ES_HTTP_STACK_IS_STARTED != _state) {
    return ESB_SHUTDOWN;
  }

  // don't uncomment the end handler callbacks until after the processing goes
  // asynch

  transaction->setStartTime();

  // TODO Make resolver async
  unsigned char hostname[1024];
  hostname[0] = 0;
  ESB::UInt16 port = 0;
  bool isSecure = false;

  ESB::Error error = transaction->getRequest()->parsePeerAddress(
      hostname, sizeof(hostname), &port, &isSecure);

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Warning)) {
      _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                   "[resolver] Cannot extract hostname from request");
    }

    return error;
  }

  error = _dnsClient->resolve(transaction->getPeerAddress(), hostname, port,
                              isSecure);

  if (ESB_SUCCESS != error) {
    _clientCounters->getFailures()->addObservation(transaction->getStartTime(),
                                                   ESB::Date::Now());

    // transaction->getHandler()->end(transaction,
    //                               HttpClientHandler::ES_HTTP_CLIENT_HANDLER_RESOLVE);

    return error;
  }

  HttpClientSocket *socket = _clientSocketFactory.create(this, transaction);

  if (!socket) {
    _clientCounters->getFailures()->addObservation(transaction->getStartTime(),
                                                   ESB::Date::Now());

    // transaction->getHandler()->end(transaction,
    //                               HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);

    if (_logger->isLoggable(ESB::Logger::Critical)) {
      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[stack] Cannot allocate new client socket");
    }

    return 0;
  }

  if (false == socket->isConnected()) {
    for (int i = 0; i < 10; ++i) {
      // non-blocking connect
      error = socket->connect();

      if (EADDRNOTAVAIL == error) {
        if (_logger->isLoggable(ESB::Logger::Warning)) {
          _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                       "[stack] EADDRNOTAVAIL on connect - check "
                       "/proc/sys/net/ipv4/tcp_tw_recycle");
        }

        continue;
      }

      break;
    }

    if (ESB_SUCCESS != error) {
      _clientCounters->getFailures()->addObservation(
          transaction->getStartTime(), ESB::Date::Now());

      // transaction->getHandler()->end(transaction,
      //                               HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);

      if (_logger->isLoggable(ESB::Logger::Critical)) {
        char buffer[100];

        ESB::DescribeError(error, buffer, sizeof(buffer));

        _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                     "[stack] Cannot connect to peer: %s", buffer);
      }

      socket->close();

      _clientSocketFactory.release(socket);

      return error;
    }
  }

  error = _dispatcher.addMultiplexedSocket(socket);

  if (ESB_SUCCESS != error) {
    _clientCounters->getFailures()->addObservation(transaction->getStartTime(),
                                                   ESB::Date::Now());

    socket->close();

    // transaction->getHandler()->end(transaction,
    //                               HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);

    if (_logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[stack] Cannot add client socket to multiplexer: %s",
                   buffer);
    }

    _clientSocketFactory.release(socket);

    return error;
  }

  return ESB_SUCCESS;
}

void HttpStack::destroyClientTransaction(HttpClientTransaction *transaction) {
  if (ES_HTTP_STACK_IS_DESTROYED == _state) {
    return;
  }

  _clientTransactionFactory.release(transaction);
}

}  // namespace ES
