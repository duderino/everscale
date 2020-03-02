#ifndef ESB_SOCKET_MULTIPLEXER_DISPATCHER_H
#include <ESBSocketMultiplexerDispatcher.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
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

namespace ESB {

SocketMultiplexerDispatcher::SocketMultiplexerDispatcher(
    UInt32 maximumSockets, UInt16 multiplexerCount,
    SocketMultiplexerFactory *factory, Allocator *allocator, const char *name,
    Logger *logger)
    : _maximumSockets(maximumSockets > 1 ? maximumSockets : 1),
      _multiplexerCount(multiplexerCount > 1 ? multiplexerCount : 1),
      _name(name ? name : "SocketMultiplexerDispatcher"),
      _logger(logger ? logger : NullLogger::GetInstance()),
      _factory(factory),
      _allocator(allocator ? allocator : SystemAllocator::GetInstance()),
      _multiplexers(0),
      _threadPool(_name, _multiplexerCount, _logger, _allocator) {}

SocketMultiplexerDispatcher::~SocketMultiplexerDispatcher() {}

Error SocketMultiplexerDispatcher::start() {
  if (!_factory) {
    return ESB_INVALID_STATE;
  }

  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s:dispatcher] starting",
                 _name);
  }

  Error error = createMultiplexers();

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(Logger::Critical)) {
      char buffer[100];

      DescribeError(error, buffer, sizeof(buffer));

      _logger->log(Logger::Critical, __FILE__, __LINE__,
                   "[%s:dispatcher] cannot create multiplexers: %s", _name,
                   buffer);
    }

    return error;
  }

  error = _threadPool.start();

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(Logger::Critical)) {
      char buffer[100];

      DescribeError(error, buffer, sizeof(buffer));

      _logger->log(Logger::Critical, __FILE__, __LINE__,
                   "[%s:dispatcher] cannot start threadpool: %s", _name,
                   buffer);
    }

    destroyMultiplexers();

    return error;
  }

  int runningMultiplexers = 0;

  for (int i = 0; i < _multiplexerCount; ++i) {
    error = _multiplexers[i]->initialize();

    if (ESB_SUCCESS != error) {
      if (_logger->isLoggable(Logger::Warning)) {
        char buffer[100];

        DescribeError(error, buffer, sizeof(buffer));

        _logger->log(Logger::Warning, __FILE__, __LINE__,
                     "[%s:dispatcher] cannot initialize multiplexer: %s", _name,
                     buffer);
      }

      continue;
    }

    error = _threadPool.execute(_multiplexers[i]);

    if (ESB_SUCCESS != error) {
      if (_logger->isLoggable(Logger::Warning)) {
        char buffer[100];

        DescribeError(error, buffer, sizeof(buffer));

        _logger->log(Logger::Warning, __FILE__, __LINE__,
                     "[%s:dispatcher] cannot execute multiplexer: %s", _name,
                     buffer);
      }

      continue;
    }

    ++runningMultiplexers;
  }

  if (0 == runningMultiplexers) {
    if (_logger->isLoggable(Logger::Critical)) {
      _logger->log(Logger::Critical, __FILE__, __LINE__,
                   "[%s:dispatcher] could not start any multiplexers", _name);
    }

    _threadPool.stop();

    destroyMultiplexers();

    return ESB_OTHER_ERROR;
  }

  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s:dispatcher] started",
                 _name);
  }

  return ESB_SUCCESS;
}

void SocketMultiplexerDispatcher::stop() {
  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s:dispatcher] stopping",
                 _name);
  }

  _threadPool.stop();

  for (int i = 0; i < _multiplexerCount; ++i) {
    _multiplexers[i]->destroy();
  }

  destroyMultiplexers();

  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s:dispatcher] stopped",
                 _name);
  }
}

Error SocketMultiplexerDispatcher::addMultiplexedSocket(
    MultiplexedSocket *multiplexedSocket) {
  if (!_multiplexers) {
    return ESB_INVALID_STATE;
  }

  if (!multiplexedSocket) {
    return ESB_NULL_POINTER;
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

  if (_logger->isLoggable(Logger::Debug)) {
    _logger->log(
        Logger::Debug, __FILE__, __LINE__,
        "[%s:dispatcher] adding socket to multiplexer %d with count %d", _name,
        minIndex + 1, minCount);
  }

  return _multiplexers[minIndex]->addMultiplexedSocket(multiplexedSocket);
}

Error SocketMultiplexerDispatcher::addMultiplexedSocket(
    int multiplexerIndex, MultiplexedSocket *multiplexedSocket) {
  if (0 > multiplexerIndex || multiplexerIndex >= _multiplexerCount) {
    return ESB_OUT_OF_BOUNDS;
  }

  if (_logger->isLoggable(Logger::Debug)) {
    _logger->log(Logger::Debug, __FILE__, __LINE__,
                 "[%s:dispatcher] adding socket to multiplexer %d", _name,
                 multiplexerIndex);
  }

  return _multiplexers[multiplexerIndex]->addMultiplexedSocket(
      multiplexedSocket);
}

int SocketMultiplexerDispatcher::getCurrentSockets() {
  int count = 0;

  for (int i = 0; i < _multiplexerCount; ++i) {
    count += _multiplexers[i]->getCurrentSockets();
  }

  return count;
}

int SocketMultiplexerDispatcher::getMaximumSockets() {
  int count = 0;

  for (int i = 0; i < _multiplexerCount; ++i) {
    count += _multiplexers[i]->getMaximumSockets();
  }

  return count;
}

Error SocketMultiplexerDispatcher::createMultiplexers() {
  _multiplexers = (SocketMultiplexer **)_allocator->allocate(
      _multiplexerCount * sizeof(SocketMultiplexer *));

  if (!_multiplexers) {
    return ESB_OUT_OF_MEMORY;
  }

#ifdef HAVE_MEMSET
  memset(_multiplexers, 0, _multiplexerCount * sizeof(SocketMultiplexer *));
#else
#error "memset or equivalent is required"
#endif

  for (int i = 0; i < _multiplexerCount; ++i) {
    _multiplexers[i] = _factory->create(_maximumSockets / _multiplexerCount);

    if (!_multiplexers[i]) {
      destroyMultiplexers();

      return ESB_OUT_OF_MEMORY;
    }
  }

  return ESB_SUCCESS;
}

void SocketMultiplexerDispatcher::destroyMultiplexers() {
  if (!_multiplexers) {
    return;
  }

  for (int i = 0; i < _multiplexerCount; ++i) {
    _factory->destroy(_multiplexers[i]);
  }

  _allocator->deallocate(_multiplexers);
  _multiplexers = 0;
}

}  // namespace ESB
