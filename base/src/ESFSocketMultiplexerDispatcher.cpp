/** @file ESFSocketMultiplexerDispatcher.cpp
 *  @brief A dispatcher that evenly distributes sockets across multiple socket multiplexers.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_SOCKET_MULTIPLEXER_DISPATCHER_H
#include <ESFSocketMultiplexerDispatcher.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

ESFSocketMultiplexerDispatcher::ESFSocketMultiplexerDispatcher(ESFUInt16 maximumSockets, ESFUInt16 multiplexerCount,
        ESFSocketMultiplexerFactory *factory, ESFAllocator *allocator, const char *name, ESFLogger *logger) :
    _maximumSockets(maximumSockets > 1 ? maximumSockets : 1), _multiplexerCount(multiplexerCount > 1 ? multiplexerCount
            : 1), _name(name ? name : "SocketMultiplexerDispatcher"), _logger(logger ? logger
            : ESFNullLogger::GetInstance()), _factory(factory), _allocator(allocator ? allocator
            : ESFSystemAllocator::GetInstance()), _multiplexers(0), _threadPool(_name, _multiplexerCount, _logger,
            _allocator) {
}

ESFSocketMultiplexerDispatcher::~ESFSocketMultiplexerDispatcher() {
}

ESFError ESFSocketMultiplexerDispatcher::start() {
    if (!_factory) {
        return ESF_INVALID_STATE;
    }

    if (_logger->isLoggable(ESFLogger::Notice)) {
        _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s:dispatcher] starting", _name);
    }

    ESFError error = createMultiplexers();

    if (ESF_SUCCESS != error) {
        if (_logger->isLoggable(ESFLogger::Critical)) {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[%s:dispatcher] cannot create multiplexers: %s",
                    _name, buffer);
        }

        return error;
    }

    error = _threadPool.start();

    if (ESF_SUCCESS != error) {
        if (_logger->isLoggable(ESFLogger::Critical)) {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[%s:dispatcher] cannot start threadpool: %s", _name,
                    buffer);
        }

        destroyMultiplexers();

        return error;
    }

    int runningMultiplexers = 0;

    for (int i = 0; i < _multiplexerCount; ++i) {
        error = _multiplexers[i]->initialize();

        if (ESF_SUCCESS != error) {
            if (_logger->isLoggable(ESFLogger::Warning)) {
                char buffer[100];

                ESFDescribeError(error, buffer, sizeof(buffer));

                _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                        "[%s:dispatcher] cannot initialize multiplexer: %s", _name, buffer);
            }

            continue;
        }

        error = _threadPool.execute(_multiplexers[i]);

        if (ESF_SUCCESS != error) {
            if (_logger->isLoggable(ESFLogger::Warning)) {
                char buffer[100];

                ESFDescribeError(error, buffer, sizeof(buffer));

                _logger->log(ESFLogger::Warning, __FILE__, __LINE__, "[%s:dispatcher] cannot execute multiplexer: %s",
                        _name, buffer);
            }

            continue;
        }

        ++runningMultiplexers;
    }

    if (0 == runningMultiplexers) {
        if (_logger->isLoggable(ESFLogger::Critical)) {
            _logger->log(ESFLogger::Critical, __FILE__, __LINE__, "[%s:dispatcher] could not start any multiplexers",
                    _name);
        }

        _threadPool.stop();

        destroyMultiplexers();

        return ESF_OTHER_ERROR;
    }

    if (_logger->isLoggable(ESFLogger::Notice)) {
        _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s:dispatcher] started", _name);
    }

    return ESF_SUCCESS;
}

void ESFSocketMultiplexerDispatcher::stop() {
    if (_logger->isLoggable(ESFLogger::Notice)) {
        _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s:dispatcher] stopping", _name);
    }

    _threadPool.stop();

    for (int i = 0; i < _multiplexerCount; ++i) {
        _multiplexers[i]->destroy();
    }

    destroyMultiplexers();

    if (_logger->isLoggable(ESFLogger::Notice)) {
        _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s:dispatcher] stopped", _name);
    }
}

ESFError ESFSocketMultiplexerDispatcher::addMultiplexedSocket(ESFMultiplexedSocket *multiplexedSocket) {
    if (!_multiplexers) {
        return ESF_INVALID_STATE;
    }

    if (!multiplexedSocket) {
        return ESF_NULL_POINTER;
    }

    int minCount = -1;
    int minIndex = 0;
    int count = 0;

    for (int i = 0; i < _multiplexerCount; ++i) {
        count = _multiplexers[i]->getCurrentSockets();

        if (-1 == minCount) {
            minCount = count;
            minIndex = i;
        } else if (count < minCount) {
            minCount = count;
            minIndex = i;
        }
    }

    if (_logger->isLoggable(ESFLogger::Debug)) {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
        "[%s:dispatcher] adding socket to multiplexer %d with count %d", _name, minIndex + 1, minCount);
    }

    return _multiplexers[minIndex]->addMultiplexedSocket(multiplexedSocket);
}

ESFError ESFSocketMultiplexerDispatcher::addMultiplexedSocket(int multiplexerIndex,
        ESFMultiplexedSocket *multiplexedSocket) {
    if (0 > multiplexerIndex || multiplexerIndex >= _multiplexerCount) {
        return ESF_OUT_OF_BOUNDS;
    }

    if (_logger->isLoggable(ESFLogger::Debug)) {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__, "[%s:dispatcher] adding socket to multiplexer %d", _name,
                multiplexerIndex);
    }

    return _multiplexers[multiplexerIndex]->addMultiplexedSocket(multiplexedSocket);
}

int ESFSocketMultiplexerDispatcher::getCurrentSockets() {
    int count = 0;

    for (int i = 0; i < _multiplexerCount; ++i) {
        count += _multiplexers[i]->getCurrentSockets();
    }

    return count;
}

int ESFSocketMultiplexerDispatcher::getMaximumSockets() {
    int count = 0;

    for (int i = 0; i < _multiplexerCount; ++i) {
        count += _multiplexers[i]->getMaximumSockets();
    }

    return count;
}

ESFUInt16 ESFSocketMultiplexerDispatcher::GetMaximumSockets() {
#if defined HAVE_STRUCT_RLIMIT && defined HAVE_GETRLIMIT
    struct rlimit rLimit;

    if (0 != getrlimit(RLIMIT_NOFILE, &rLimit)) {
        return ESF_UINT16_MAX;
    }

    if (0 >= rLimit.rlim_cur) {
        return ESF_UINT16_MAX;
    }

    if (ESF_UINT16_MAX < rLimit.rlim_cur) {
        return ESF_UINT16_MAX;
    }

    return RLIM_INFINITY == rLimit.rlim_cur ? ESF_UINT16_MAX : rLimit.rlim_cur;
#else
#error "getrlimit() or equivalent is required"
#endif
}

ESFError ESFSocketMultiplexerDispatcher::createMultiplexers() {
    _multiplexers = (ESFSocketMultiplexer **) _allocator->allocate(_multiplexerCount * sizeof(ESFSocketMultiplexer *));

    if (!_multiplexers) {
        return ESF_OUT_OF_MEMORY;
    }

#ifdef HAVE_MEMSET
    memset(_multiplexers, 0, _multiplexerCount * sizeof(ESFSocketMultiplexer *));
#else
#error "memset or equivalent is required"
#endif

    for (int i = 0; i < _multiplexerCount; ++i) {
        _multiplexers[i] = _factory->create(_maximumSockets / _multiplexerCount);

        if (!_multiplexers[i]) {
            destroyMultiplexers();

            return ESF_OUT_OF_MEMORY;
        }
    }

    return ESF_SUCCESS;
}

void ESFSocketMultiplexerDispatcher::destroyMultiplexers() {
    if (!_multiplexers) {
        return;
    }

    for (int i = 0; i < _multiplexerCount; ++i) {
        _factory->destroy(_multiplexers[i]);
    }

    _allocator->deallocate(_multiplexers);
    _multiplexers = 0;
}

