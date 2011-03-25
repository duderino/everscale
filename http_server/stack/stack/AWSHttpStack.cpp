/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_STACK_H
#include <AWSHttpStack.h>
#endif

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#include <errno.h>

static const int MaxSockets = ESFSocketMultiplexerDispatcher::GetMaximumSockets();

AWSHttpStack::AWSHttpStack(AWSHttpServerHandler *serverHandler, AWSHttpResolver *resolver, int port, int threads, AWSHttpClientCounters *clientCounters,
        AWSHttpServerCounters *serverCounters, ESFLogger *logger) :
    _port(0 > port ? 80 : port), _threads(0 >= threads ? 1 : threads), _state(AWS_HTTP_STACK_IS_DESTROYED), _resolver(resolver), _logger(logger ? logger
            : ESFNullLogger::GetInstance()), _serverHandler(serverHandler), _serverCounters(serverCounters), _clientCounters(clientCounters), _discardAllocator(
            4000, ESFSystemAllocator::GetInstance()), _rootAllocator(&_discardAllocator), _rootAllocatorCleanupHandler(&_rootAllocator), _epollFactory(
            "EpollMultiplexer", _logger, &_rootAllocator), _listeningSocket(_port, ESF_UINT16_MAX, false), _serverSocketFactory(_serverCounters, _logger),
            _clientSocketFactory(_clientCounters, _logger), _clientTransactionFactory(), _dispatcher(MaxSockets, _threads,
                    &_epollFactory, &_rootAllocator, "EpollDispatcher", _logger) {
}

AWSHttpStack::AWSHttpStack(AWSHttpResolver *resolver, int threads, AWSHttpClientCounters *clientCounters, ESFLogger *logger) :
    _port(-1), _threads(0 >= threads ? 1 : threads), _state(AWS_HTTP_STACK_IS_DESTROYED), _resolver(resolver), _logger(logger ? logger
            : ESFNullLogger::GetInstance()), _serverHandler(0), _serverCounters(0), _clientCounters(clientCounters), _discardAllocator(4000,
            ESFSystemAllocator::GetInstance()), _rootAllocator(&_discardAllocator), _rootAllocatorCleanupHandler(&_rootAllocator), _epollFactory(
            "EpollMultiplexer", _logger, &_rootAllocator), _listeningSocket(_port, ESF_UINT16_MAX, false), _serverSocketFactory(_serverCounters, _logger),
            _clientSocketFactory(_clientCounters, _logger), _clientTransactionFactory(), _dispatcher(MaxSockets, _threads,
                    &_epollFactory, &_rootAllocator, "EpollDispatcher", _logger) {
}

AWSHttpStack::~AWSHttpStack() {
}

ESFError AWSHttpStack::initialize() {
    ESF_ASSERT(AWS_HTTP_STACK_IS_DESTROYED == _state);

    if (_logger->isLoggable(ESFLogger::Notice)) {
        _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[stack] Maximum sockets %d", MaxSockets);
    }

    ESFError error = _rootAllocator.initialize();

    if (ESF_SUCCESS != error) {
        if (_logger->isLoggable(ESFLogger::Critical)) {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[stack] Cannot initialize root allocator: %s", buffer);
        }

        return error;
    }

    error = _clientSocketFactory.initialize();

    if (ESF_SUCCESS != error) {
        if (_logger->isLoggable(ESFLogger::Critical)) {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[stack] Cannot initialize client socket factory: %s", buffer);
        }

        return error;
    }

    error = _clientTransactionFactory.initialize();

    if (ESF_SUCCESS != error) {
        if (_logger->isLoggable(ESFLogger::Critical)) {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[stack] Cannot initialize client transaction factory: %s", buffer);
        }

        return error;
    }

    if (0 > _port || 0 == _serverHandler) {
        _state = AWS_HTTP_STACK_IS_INITIALIZED;

        return ESF_SUCCESS;
    }

    error = _listeningSocket.bind();

    if (ESF_SUCCESS != error) {
        if (_logger->isLoggable(ESFLogger::Critical)) {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[stack] Cannot bind to port %d: %s", _port, buffer);
        }

        return error;
    }

    error = _listeningSocket.listen();

    if (ESF_SUCCESS != error) {
        if (_logger->isLoggable(ESFLogger::Critical)) {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[stack] Cannot listen on port %d: %s", _port, buffer);
        }

        return error;
    }

    error = _serverSocketFactory.initialize();

    if (ESF_SUCCESS != error) {
        if (_logger->isLoggable(ESFLogger::Critical)) {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[stack] Cannot initialize server socket factory: %s", buffer);
        }

        return error;
    }

    _state = AWS_HTTP_STACK_IS_INITIALIZED;

    return ESF_SUCCESS;
}

ESFError AWSHttpStack::start() {
    ESF_ASSERT(AWS_HTTP_STACK_IS_INITIALIZED == _state);

    ESFError error = _dispatcher.start();

    if (ESF_SUCCESS != error) {
        if (_logger->isLoggable(ESFLogger::Critical)) {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[stack] Cannot start multiplexer dispatcher: %s", buffer);
        }

        return error;
    }

    if (0 > _port || 0 == _serverHandler) {
        _state = AWS_HTTP_STACK_IS_STARTED;

        if (_logger->isLoggable(ESFLogger::Notice)) {
            _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[stack] started");
        }

        return ESF_SUCCESS;
    }

    AWSHttpListeningSocket *socket = 0;

    for (int i = 0; i < _threads; ++i) {
        socket = new (&_rootAllocator) AWSHttpListeningSocket(_serverHandler, &_listeningSocket, &_dispatcher, &_serverSocketFactory, _logger,
                &_rootAllocatorCleanupHandler, _serverCounters);

        if (!socket) {
            if (_logger->isLoggable(ESFLogger::Critical)) {
                _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[stack] Cannot allocate new listening socket");
            }

            return ESF_OUT_OF_MEMORY;
        }

        error = _dispatcher.addMultiplexedSocket(i, socket);

        if (ESF_SUCCESS != error) {
            if (_logger->isLoggable(ESFLogger::Critical)) {
                char buffer[100];

                ESFDescribeError(error, buffer, sizeof(buffer));

                _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[stack] Cannot add listening socket to multiplexer: %s", buffer);
            }

            return error;
        }
    }

    _state = AWS_HTTP_STACK_IS_STARTED;

    if (_logger->isLoggable(ESFLogger::Notice)) {
        _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[stack] started");
    }

    return ESF_SUCCESS;
}

ESFError AWSHttpStack::stop() {
    ESF_ASSERT(AWS_HTTP_STACK_IS_STARTED == _state);

    _state = AWS_HTTP_STACK_IS_STOPPED;

    if (_logger->isLoggable(ESFLogger::Notice)) {
        _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[stack] stopping");
    }

    _dispatcher.stop();

    if (_logger->isLoggable(ESFLogger::Notice)) {
        _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[stack] stopped");
    }

    return ESF_SUCCESS;
}

void AWSHttpStack::destroy() {
    ESF_ASSERT(AWS_HTTP_STACK_IS_STOPPED == _state);

    _state = AWS_HTTP_STACK_IS_DESTROYED;

    _clientSocketFactory.destroy();
    _clientTransactionFactory.destroy();

    if (0 > _port || 0 == _serverHandler) {
        return;
    }

    _serverSocketFactory.destroy();
}

AWSHttpClientTransaction *AWSHttpStack::createClientTransaction(AWSHttpClientHandler *clientHandler) {
    if (AWS_HTTP_STACK_IS_DESTROYED == _state) {
        return 0;
    }

    AWSHttpClientTransaction *transaction = _clientTransactionFactory.create(clientHandler);

    if (!transaction) {
        if (_logger->isLoggable(ESFLogger::Critical)) {
            _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[stack] Cannot allocate new transaction");
        }

        return 0;
    }

    return transaction;
}

ESFError AWSHttpStack::executeClientTransaction(AWSHttpClientTransaction *transaction) {
    if (!transaction) {
        return ESF_NULL_POINTER;
    }

    if (AWS_HTTP_STACK_IS_STARTED != _state) {
        return ESF_SHUTDOWN;
    }

    // don't uncomment the end handler callbacks until after the processing goes asynch

    transaction->setStartTime();

    // TODO Make resolver async

    ESFError error = _resolver->resolve(transaction->getRequest(), transaction->getPeerAddress());

    if (ESF_SUCCESS != error) {
        _clientCounters->getFailures()->addObservation(transaction->getStartTime());

        //transaction->getHandler()->end(transaction,
        //                               AWSHttpClientHandler::AWS_HTTP_CLIENT_HANDLER_RESOLVE);

        return error;
    }

    AWSHttpClientSocket *socket = _clientSocketFactory.create(this, transaction);

    if (!socket) {
        _clientCounters->getFailures()->addObservation(transaction->getStartTime());

        //transaction->getHandler()->end(transaction,
        //                               AWSHttpClientHandler::AWS_HTTP_CLIENT_HANDLER_CONNECT);

        if (_logger->isLoggable(ESFLogger::Critical)) {
            _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[stack] Cannot allocate new client socket");
        }

        return 0;
    }

    if (false == socket->isConnected()) {
        for (int i = 0; i < 10; ++i) {
            // non-blocking connect
            error = socket->connect();

            if (EADDRNOTAVAIL == error) {
                if (_logger->isLoggable(ESFLogger::Warning)) {
                    _logger->log(ESFLogger::Warning, __FILE__, __LINE__, "[stack] EADDRNOTAVAIL on connect - check /proc/sys/net/ipv4/tcp_tw_recycle");
                }

                continue;
            }

            break;
        }

        if (ESF_SUCCESS != error) {
            _clientCounters->getFailures()->addObservation(transaction->getStartTime());

            //transaction->getHandler()->end(transaction,
            //                               AWSHttpClientHandler::AWS_HTTP_CLIENT_HANDLER_CONNECT);

            if (_logger->isLoggable(ESFLogger::Critical)) {
                char buffer[100];

                ESFDescribeError(error, buffer, sizeof(buffer));

                _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[stack] Cannot connect to peer: %s", buffer);
            }

            socket->close();

            _clientSocketFactory.release(socket);

            return error;
        }
    }

    error = _dispatcher.addMultiplexedSocket(socket);

    if (ESF_SUCCESS != error) {
        _clientCounters->getFailures()->addObservation(transaction->getStartTime());

        socket->close();

        //transaction->getHandler()->end(transaction,
        //                               AWSHttpClientHandler::AWS_HTTP_CLIENT_HANDLER_CONNECT);

        if (_logger->isLoggable(ESFLogger::Critical)) {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[stack] Cannot add client socket to multiplexer: %s", buffer);
        }

        _clientSocketFactory.release(socket);

        return error;
    }

    return ESF_SUCCESS;
}

void AWSHttpStack::destroyClientTransaction(AWSHttpClientTransaction *transaction) {
    if (AWS_HTTP_STACK_IS_DESTROYED == _state) {
        return;
    }

    _clientTransactionFactory.release(transaction);
}

