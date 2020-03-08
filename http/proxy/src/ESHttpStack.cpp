#ifndef ES_HTTP_STACK_H
#include <ESHttpStack.h>
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
                     HttpServerCounters *serverCounters)
    : _port(0 > port ? 80 : port),
      _threads(0 >= threads ? 1 : threads),
      _state(ES_HTTP_STACK_IS_DESTROYED),
      _dnsClient(dnsClient),
      _serverHandler(serverHandler),
      _serverCounters(serverCounters),
      _clientCounters(clientCounters),
      _discardAllocator(4000, ESB::SystemAllocator::GetInstance()),
      _rootAllocator(&_discardAllocator),
      _rootAllocatorCleanupHandler(&_rootAllocator),
      _epollFactory("HttpMultiplexer", &_rootAllocator),
      _listeningSocket(_port, ESB_UINT16_MAX, false),
      _serverSocketFactory(_serverCounters),
      _clientSocketFactory(_clientCounters),
      _clientTransactionFactory(),
      _dispatcher(ESB::ProcessLimits::GetSocketSoftMax(), _threads,
                  &_epollFactory, &_rootAllocator, "HttpDispatcher") {}

HttpStack::HttpStack(ESB::DnsClient *dnsClient, int threads,
                     HttpClientCounters *clientCounters)
    : _port(-1),
      _threads(0 >= threads ? 1 : threads),
      _state(ES_HTTP_STACK_IS_DESTROYED),
      _dnsClient(dnsClient),
      _serverHandler(0),
      _serverCounters(0),
      _clientCounters(clientCounters),
      _discardAllocator(4000, ESB::SystemAllocator::GetInstance()),
      _rootAllocator(&_discardAllocator),
      _rootAllocatorCleanupHandler(&_rootAllocator),
      _epollFactory("HttpMultiplexer", &_rootAllocator),
      _listeningSocket(_port, ESB_UINT16_MAX, false),
      _serverSocketFactory(_serverCounters),
      _clientSocketFactory(_clientCounters),
      _clientTransactionFactory(),
      _dispatcher(ESB::ProcessLimits::GetSocketSoftMax(), _threads,
                  &_epollFactory, &_rootAllocator, "HttpDispatcher") {}

HttpStack::~HttpStack() {}

ESB::Error HttpStack::initialize() {
  assert(ES_HTTP_STACK_IS_DESTROYED == _state.get());

  ESB_LOG_NOTICE("Maximum sockets %u", ESB::ProcessLimits::GetSocketSoftMax());

  ESB::Error error = _rootAllocator.initialize();

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERRNO_CRITICAL(error, "Cannot initialize root allocator");
    return error;
  }

  error = _clientSocketFactory.initialize();

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERRNO_CRITICAL(error, "Cannot initialize client socket factory");
    return error;
  }

  error = _clientTransactionFactory.initialize();

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERRNO_CRITICAL(error, "Cannot initialize client trans factory");
    return error;
  }

  if (0 > _port || 0 == _serverHandler) {
    _state.set(ES_HTTP_STACK_IS_INITIALIZED);
    return ESB_SUCCESS;
  }

  error = _listeningSocket.bind();

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERRNO_CRITICAL(error, "Cannot bind to port %d", _port);
    return error;
  }

  error = _listeningSocket.listen();

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERRNO_CRITICAL(error, "Cannot listen on port %d", _port);
    return error;
  }

  error = _serverSocketFactory.initialize();

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERRNO_CRITICAL(error, "Cannot initialize server socket factory");
    return error;
  }

  _state.set(ES_HTTP_STACK_IS_INITIALIZED);
  return ESB_SUCCESS;
}

ESB::Error HttpStack::start() {
  assert(ES_HTTP_STACK_IS_INITIALIZED == _state.get());

  ESB::Error error = _dispatcher.start();

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERRNO_CRITICAL(error, "Cannot start multiplexer dispatcher");
    return error;
  }

  if (0 > _port || 0 == _serverHandler) {
    _state.set(ES_HTTP_STACK_IS_STARTED);
    ESB_LOG_NOTICE("Started");
    return ESB_SUCCESS;
  }

  HttpListeningSocket *socket = 0;

  for (int i = 0; i < _threads; ++i) {
    socket = new (&_rootAllocator) HttpListeningSocket(
        _serverHandler, &_listeningSocket, &_dispatcher, &_serverSocketFactory,
        &_rootAllocatorCleanupHandler, _serverCounters);

    if (!socket) {
      ESB_LOG_CRITICAL("Cannot allocate new listening socket");
      return ESB_OUT_OF_MEMORY;
    }

    error = _dispatcher.addMultiplexedSocket(i, socket);

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERRNO_CRITICAL(error, "Cannot add listening socket to multiplexer");
      return error;
    }
  }

  _state.set(ES_HTTP_STACK_IS_STARTED);
  ESB_LOG_NOTICE("Started");
  return ESB_SUCCESS;
}

ESB::Error HttpStack::stop() {
  assert(ES_HTTP_STACK_IS_STARTED == _state.get());
  _state.set(ES_HTTP_STACK_IS_STOPPED);

  ESB_LOG_NOTICE("Stopping");
  _dispatcher.stop();
  ESB_LOG_NOTICE("Stopped");

  return ESB_SUCCESS;
}

void HttpStack::destroy() {
  assert(ES_HTTP_STACK_IS_STOPPED == _state.get());

  _state.set(ES_HTTP_STACK_IS_DESTROYED);

  _clientSocketFactory.destroy();
  _clientTransactionFactory.destroy();
  _serverSocketFactory.destroy();
}

HttpClientTransaction *HttpStack::createClientTransaction(
    HttpClientHandler *clientHandler) {
  if (ES_HTTP_STACK_IS_DESTROYED == _state.get()) {
    return 0;
  }

  HttpClientTransaction *transaction =
      _clientTransactionFactory.create(clientHandler);

  if (!transaction) {
    ESB_LOG_CRITICAL("Cannot allocate new client transaction");
    return 0;
  }

  return transaction;
}

ESB::Error HttpStack::executeClientTransaction(
    HttpClientTransaction *transaction) {
  if (!transaction) {
    return ESB_NULL_POINTER;
  }

  if (ES_HTTP_STACK_IS_STARTED != _state.get()) {
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
    ESB_LOG_DEBUG("Cannot extract hostname from request");
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
    ESB_LOG_CRITICAL("Cannot allocate new client socket");
    return 0;
  }

  if (false == socket->isConnected()) {
    error = socket->connect();

    if (ESB_SUCCESS!=error) {
      _clientCounters->getFailures()->addObservation(
          transaction->getStartTime(), ESB::Date::Now());
      ESB_LOG_ERRNO_WARNING(error, "Cannot connect to %s:%d", hostname, port);
      // transaction->getHandler()->end(transaction,
      //                               HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
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
    ESB_LOG_ERRNO_CRITICAL(error, "Cannot add cient socket to multiplexer");
    _clientSocketFactory.release(socket);
    return error;
  }

  return ESB_SUCCESS;
}

void HttpStack::destroyClientTransaction(HttpClientTransaction *transaction) {
  if (ES_HTTP_STACK_IS_DESTROYED == _state.get()) {
    return;
  }

  _clientTransactionFactory.release(transaction);
}

}  // namespace ES
