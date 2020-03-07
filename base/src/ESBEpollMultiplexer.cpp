#ifndef ESB_EPOLL_MULTIPLEXER_H
#include <ESBEpollMultiplexer.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

#ifndef ESB_READ_SCOPE_LOCK_H
#include <ESBReadScopeLock.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifndef ESB_CONNECTED_TCP_SOCKET_H
#include <ESBConnectedTCPSocket.h>
#endif

#ifndef HAVE_EPOLL_CREATE
#error "epoll_create is required"
#endif

#ifndef HAVE_EPOLL_CTL
#error "epoll_ctl is required"
#endif

#ifndef HAVE_EPOLL_WAIT
#error "epoll_wait is required"
#endif

#ifndef HAVE_CLOSE
#error "close() or equivalent is required"
#endif

#ifndef HAVE_MEMSET
#error "memset() or equivalent is required"
#endif

#ifndef HAVE_TIME
#error "time() or equilvalent is required"
#endif

namespace ESB {

#define EPOLL_TIMEOUT_MILLIS 1000
#define MIN_MAX_SOCKETS 1
#define IDLE_CHECK_SEC 30

EpollMultiplexer::EpollMultiplexer(const char *name, int maxSockets,
                                   Logger *logger, Allocator *allocator)
    : SocketMultiplexer(name, logger),
      _epollDescriptor(INVALID_SOCKET),
      _maxSockets(maxSockets < MIN_MAX_SOCKETS ? MIN_MAX_SOCKETS : maxSockets),
      _lastIdleCheckSec(0),
      _events(0),
      _allocator(allocator ? allocator : SystemAllocator::GetInstance()),
      _currentSocketCount(),
      _currentSocketList(),
      _lock() {}

EpollMultiplexer::~EpollMultiplexer() {
  if (INVALID_SOCKET != _epollDescriptor) {
    close(_epollDescriptor);
    _epollDescriptor = INVALID_SOCKET;
  }
}

CleanupHandler *EpollMultiplexer::getCleanupHandler() { return 0; }

const char *EpollMultiplexer::getName() const { return _name; }

Error EpollMultiplexer::addMultiplexedSocket(
    MultiplexedSocket *multiplexedSocket) {
  if (0 > _epollDescriptor) {
    return ESB_INVALID_STATE;
  }

  if (0 == multiplexedSocket) {
    return ESB_NULL_POINTER;
  }

  SOCKET socketDescriptor = multiplexedSocket->getSocketDescriptor();

  if (INVALID_SOCKET == socketDescriptor) {
    return ESB_INVALID_ARGUMENT;
  }

  struct epoll_event event;

  memset(&event, 0, sizeof(event));

  event.events = EPOLLERR | EPOLLONESHOT;

  if (multiplexedSocket->wantAccept()) {
    event.events |= EPOLLIN;
  } else if (multiplexedSocket->wantConnect()) {
    event.events |= EPOLLIN | EPOLLOUT | EPOLLHUP;
  } else {
    if (multiplexedSocket->wantWrite()) {
      event.events |= EPOLLOUT | EPOLLHUP;
    }
    if (multiplexedSocket->wantRead()) {
      event.events |= EPOLLIN | EPOLLHUP;
    }
  }

  if ((EPOLLERR | EPOLLONESHOT) == event.events) {
    // means wantAccept(), wantConnect(), wantRead() and wantWrite() all
    // returned false

    return ESB_INVALID_ARGUMENT;
  }

  int currentSocketCount = _currentSocketCount.inc();

  if (currentSocketCount > _maxSockets) {
    if (_logger->isLoggable(Logger::Err)) {
      _logger->log(Logger::Err, __FILE__, __LINE__,
                   "[%s:%d] cannot add socket %d to epoll descriptor %d: at "
                   "limit of %d sockets",
                   _name, _epollDescriptor, socketDescriptor, _epollDescriptor,
                   _maxSockets);
    }

    _currentSocketCount.dec();
    return ESB_OVERFLOW;
  }

  _lock.writeAcquire();
  _currentSocketList.addLast(multiplexedSocket);
  assert(true == _currentSocketList.validate());
  _lock.writeRelease();

  event.data.ptr = multiplexedSocket;

  if (0 !=
      epoll_ctl(_epollDescriptor, EPOLL_CTL_ADD, socketDescriptor, &event)) {
    Error error = GetLastError();
    if (_logger->isLoggable(Logger::Err)) {
      char buffer[100];
      DescribeError(error, buffer, sizeof(buffer));
      _logger->log(Logger::Err, __FILE__, __LINE__,
                   "[%s:%d] cannot add socket %d to epoll descriptor %d: %d:%s",
                   _name, _epollDescriptor, socketDescriptor, _epollDescriptor,
                   error, buffer);
    }
    _lock.writeAcquire();
    _currentSocketList.remove(multiplexedSocket);
    assert(true == _currentSocketList.validate());
    _lock.writeRelease();
    _currentSocketCount.dec();
    return error;
  }

  if (_logger->isLoggable(Logger::Debug)) {
    _logger->log(Logger::Debug, __FILE__, __LINE__,
                 "[%s:%d] added socket %d to epoll descriptor %d.  Current "
                 "socket count: %d",
                 _name, _epollDescriptor, socketDescriptor, _epollDescriptor,
                 currentSocketCount);
  }

  return ESB_SUCCESS;
}

Error EpollMultiplexer::updateMultiplexedSocket(
    SharedInt *isRunning, MultiplexedSocket *multiplexedSocket) {
  if (0 > _epollDescriptor) {
    return ESB_INVALID_STATE;
  }

  if (0 == multiplexedSocket) {
    return ESB_NULL_POINTER;
  }

  SOCKET socketDescriptor = multiplexedSocket->getSocketDescriptor();

  if (INVALID_SOCKET == socketDescriptor) {
    return ESB_INVALID_ARGUMENT;
  }

  struct epoll_event event;

  memset(&event, 0, sizeof(event));

  event.events = EPOLLERR | EPOLLONESHOT;

  if (multiplexedSocket->wantAccept()) {
    event.events |= EPOLLIN;
  } else if (multiplexedSocket->wantConnect()) {
    event.events |= EPOLLIN | EPOLLOUT | EPOLLHUP;
  } else {
    if (multiplexedSocket->wantWrite()) {
      event.events |= EPOLLOUT | EPOLLHUP;
    }

    if (multiplexedSocket->wantRead()) {
      event.events |= EPOLLIN | EPOLLHUP;
    }
  }

  if ((EPOLLERR | EPOLLONESHOT) == event.events) {
    // means wantAccept(), wantConnect(), wantRead() and wantWrite() all
    // returned false

    return ESB_INVALID_ARGUMENT;
  }

  event.data.ptr = multiplexedSocket;

  if (0 !=
      epoll_ctl(_epollDescriptor, EPOLL_CTL_MOD, socketDescriptor, &event)) {
    Error error = GetLastError();
    if (_logger->isLoggable(Logger::Err)) {
      char buffer[100];
      DescribeError(error, buffer, sizeof(buffer));
      _logger->log(
          Logger::Err, __FILE__, __LINE__,
          "[%s:%d] cannot update socket %d in epoll descriptor %d: %d:%s",
          _name, _epollDescriptor, socketDescriptor, _epollDescriptor, error,
          buffer);
    }
    return error;
  }

  if (_logger->isLoggable(Logger::Debug)) {
    _logger->log(Logger::Debug, __FILE__, __LINE__,
                 "[%s:%d] updated socket %d in epoll descriptor %d", _name,
                 _epollDescriptor, socketDescriptor, _epollDescriptor);
  }

  return ESB_SUCCESS;
}

Error EpollMultiplexer::removeMultiplexedSocket(
    SharedInt *isRunning, MultiplexedSocket *multiplexedSocket,
    bool removeFromList) {
  if (0 > _epollDescriptor) {
    return ESB_INVALID_STATE;
  }

  if (0 == multiplexedSocket) {
    return ESB_NULL_POINTER;
  }

  SOCKET socketDescriptor = multiplexedSocket->getSocketDescriptor();

  if (INVALID_SOCKET == socketDescriptor) {
    return ESB_INVALID_ARGUMENT;
  }

  if (0 != epoll_ctl(_epollDescriptor, EPOLL_CTL_DEL, socketDescriptor, 0)) {
    Error error = GetLastError();
    if (_logger->isLoggable(Logger::Err)) {
      char buffer[100];
      DescribeError(error, buffer, sizeof(buffer));
      _logger->log(
          Logger::Err, __FILE__, __LINE__,
          "[%s:%d] cannot remove socket %d from epoll descriptor %d: %d:%s",
          _name, _epollDescriptor, socketDescriptor, _epollDescriptor, error,
          buffer);
    }
    return error;
  }

  int currentSocketCount = 0;

  if (removeFromList) {
    _lock.writeAcquire();
    _currentSocketList.remove(multiplexedSocket);
    assert(true == _currentSocketList.validate());
    _lock.writeRelease();

    currentSocketCount = _currentSocketCount.dec();
  } else {
    currentSocketCount = _currentSocketCount.get();
  }

  if (_logger->isLoggable(Logger::Debug)) {
    _logger->log(Logger::Debug, __FILE__, __LINE__,
                 "[%s:%d] removed socket %d from epoll descriptor %d. Current "
                 "socket count: %d",
                 _name, _epollDescriptor, socketDescriptor, _epollDescriptor,
                 currentSocketCount);
  }

  if (multiplexedSocket->handleRemoveEvent(isRunning, _logger)) {
    CleanupHandler *cleanupHandler = multiplexedSocket->getCleanupHandler();
    assert(cleanupHandler);
    if (cleanupHandler) {
      cleanupHandler->destroy(multiplexedSocket);
    }
  }

  return ESB_SUCCESS;
}

Error EpollMultiplexer::initialize() {
  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s] initializing",
                 _name);
  }

  _events = (struct epoll_event *)_allocator->allocate(
      sizeof(struct epoll_event) * _maxSockets);

  if (0 == _events) {
    if (_logger->isLoggable(Logger::Critical)) {
      _logger->log(Logger::Critical, __FILE__, __LINE__,
                   "[%s] cannot alloc epoll_event array of size %d", _name,
                   _maxSockets);
    }

    return ESB_OUT_OF_MEMORY;
  }

  _epollDescriptor = epoll_create(_maxSockets);

  if (0 > _epollDescriptor) {
    Error error = GetLastError();
    if (_logger->isLoggable(Logger::Critical)) {
      char buffer[100];
      DescribeError(error, buffer, sizeof(buffer));
      _logger->log(Logger::Critical, __FILE__, __LINE__,
                   "[%s] cannot create epoll descriptor: %d:%s", _name, error,
                   buffer);
    }
    _allocator->deallocate(_events);
    _events = 0;
    return error;
  }

  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s:%d] initialized",
                 _name, _epollDescriptor);
  }

  return ESB_SUCCESS;
}

void EpollMultiplexer::destroy() {
  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s:%d] destroying",
                 _name, _epollDescriptor);
  }

  MultiplexedSocket *head = 0;
  SharedInt isRunning(false);

  while (true) {
    _lock.writeAcquire();
    head = (MultiplexedSocket *)_currentSocketList.removeFirst();
    assert(true == _currentSocketList.validate());
    _lock.writeRelease();

    if (0 == head) {
      break;
    }

    _currentSocketCount.dec();

    removeMultiplexedSocket(&isRunning, head, false);
  }

#ifndef NDEBUG
  _lock.readAcquire();
  assert(_currentSocketCount.get() == _currentSocketList.length());
  _lock.readRelease();
#endif

  if (INVALID_SOCKET != _epollDescriptor) {
    close(_epollDescriptor);
    _epollDescriptor = INVALID_SOCKET;
  }

  if (_events) {
    _allocator->deallocate(_events);
    _events = 0;
  }

  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s] destroyed", _name);
  }
}

bool EpollMultiplexer::run(SharedInt *isRunning) {
  if (INVALID_SOCKET == _epollDescriptor) {
    return false;
  }

  if (!_events) {
    return false;
  }

  if (!isRunning) {
    return false;
  }

  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s:%d] started", _name,
                 _epollDescriptor);
  }

  int numEvents = 0;
  int i = 0;
  Error error = ESB_SUCCESS;
  MultiplexedSocket *multiplexedSocket = 0;
  int errorCount = 0;
  SOCKET socketDescriptor = INVALID_SOCKET;
  bool keepInMultiplexer = true;

  while (isRunning->get()) {
    checkIdleSockets(isRunning);

    numEvents = epoll_wait(_epollDescriptor, _events, _maxSockets,
                           EPOLL_TIMEOUT_MILLIS);

    if (0 == numEvents) {
      continue;
    }

    if (0 > numEvents) {
      error = GetLastError();

      if (ESB_INTR == error) {
        continue;
      }

      if (_logger->isLoggable(Logger::Critical)) {
        char buffer[100];

        DescribeError(error, buffer, sizeof(buffer));

        _logger->log(Logger::Critical, __FILE__, __LINE__,
                     "[%s:%d] error in epoll_wait: %s", _name, _epollDescriptor,
                     buffer);
      }

      if (errorCount >= 10) {
        if (_logger->isLoggable(Logger::Emergency)) {
          _logger->log(
              Logger::Emergency, __FILE__, __LINE__,
              "[%s:%d] too many errors in epoll_wait, multiplexer exiting",
              _name, _epollDescriptor);
        }

        _allocator->deallocate(_events);
        close(_epollDescriptor);
        _epollDescriptor = INVALID_SOCKET;

        return 0;  // caller should not cleanup this object after this method
                   // returns
      }

      ++errorCount;
      continue;
    }

    errorCount = 0;

    for (i = 0; i < numEvents; ++i) {
      keepInMultiplexer = true;

      multiplexedSocket = (MultiplexedSocket *)_events[i].data.ptr;

      socketDescriptor = multiplexedSocket->getSocketDescriptor();

      //
      // Handle Listening Socket
      //

      if (multiplexedSocket->wantAccept()) {
        //
        // UNIX non-blocking accept rules:
        //
        // (1) ! EPOLLERR && ! EPOLLIN => IN PROGRESS
        // (2) EPOLLERR => ERROR.  getsockopt() or next accept() will return
        // errno (3) EPOLLIN => ACCEPT
        //

        if (_events[i].events & EPOLLERR) {
          error = TCPSocket::GetLastSocketError(socketDescriptor);

          if (_logger->isLoggable(Logger::Err)) {
            char buffer[100];

            DescribeError(error, buffer, sizeof(buffer));

            _logger->log(Logger::Err, __FILE__, __LINE__,
                         "[%s:%d] error on listening socket %d: %s", _name,
                         _epollDescriptor, socketDescriptor, buffer);
          }

          keepInMultiplexer =
              multiplexedSocket->handleErrorEvent(error, isRunning, _logger);
        } else if (_events[i].events & EPOLLIN) {
          if (_logger->isLoggable(Logger::Debug)) {
            _logger->log(Logger::Debug, __FILE__, __LINE__,
                         "[%s:%d] accept event on listening socket %d", _name,
                         _epollDescriptor, socketDescriptor);
          }

          keepInMultiplexer =
              multiplexedSocket->handleAcceptEvent(isRunning, _logger);
        } else if (_logger->isLoggable(Logger::Err)) {
          _logger->log(
              Logger::Err, __FILE__, __LINE__,
              "[%s:%d] unexpected epoll event on listening socket %d: %d",
              _name, _epollDescriptor, socketDescriptor, _events[i].events);
        }
      }

      //
      //  Handle Connecting Socket
      //

      else if (multiplexedSocket->wantConnect()) {
        //
        // UNIX non-blocking connect rules:
        //
        // (1) ! EPOLLIN && ! EPOLLOUT && ! EPOLLERR => IN PROGRESS
        // (2) EPOLLERR => ERROR
        // (3) EPOLLHUP => EOF
        // (4) EPOLLIN && 0 == bytesReadable => EOF
        // (5) EPOLLIN && EPOLLOUT && 0 < bytesReadable => CONNECTED
        // (6) EPOLLOUT => CONNECTED
        //

        if (_events[i].events & EPOLLERR) {
          error = TCPSocket::GetLastSocketError(socketDescriptor);

          if (_logger->isLoggable(Logger::Warning)) {
            char buffer[100];

            DescribeError(error, buffer, sizeof(buffer));

            _logger->log(Logger::Warning, __FILE__, __LINE__,
                         "[%s:%d] error on conecting socket %d: %s", _name,
                         _epollDescriptor, socketDescriptor, buffer);
          }

          keepInMultiplexer =
              multiplexedSocket->handleErrorEvent(error, isRunning, _logger);
        } else if (_events[i].events & EPOLLHUP) {
          if (_logger->isLoggable(Logger::Warning)) {
            _logger->log(Logger::Warning, __FILE__, __LINE__,
                         "[%s:%d] end of file on connecting socket %d", _name,
                         _epollDescriptor, socketDescriptor);
          }

          keepInMultiplexer =
              multiplexedSocket->handleEndOfFileEvent(isRunning, _logger);
        } else if (_events[i].events & EPOLLIN) {
          int bytesReadable =
              ConnectedTCPSocket::GetBytesReadable(socketDescriptor);

          if (0 > bytesReadable) {
            error = TCPSocket::GetLastSocketError(socketDescriptor);

            if (_logger->isLoggable(Logger::Warning)) {
              char buffer[100];

              DescribeError(error, buffer, sizeof(buffer));

              _logger->log(Logger::Warning, __FILE__, __LINE__,
                           "[%s:%d] error on conecting socket %d: %s", _name,
                           _epollDescriptor, socketDescriptor, buffer);
            }

            keepInMultiplexer =
                multiplexedSocket->handleErrorEvent(error, isRunning, _logger);
          } else if (0 == bytesReadable) {
            keepInMultiplexer =
                multiplexedSocket->handleEndOfFileEvent(isRunning, _logger);
          } else {
            keepInMultiplexer =
                multiplexedSocket->handleConnectEvent(isRunning, _logger);
          }
        } else if (_events[i].events & EPOLLOUT) {
          keepInMultiplexer =
              multiplexedSocket->handleConnectEvent(isRunning, _logger);
        } else if (_logger->isLoggable(Logger::Err)) {
          _logger->log(
              Logger::Err, __FILE__, __LINE__,
              "[%s:%d] unexpected epoll event on connecting socket %d: %d",
              _name, _epollDescriptor, socketDescriptor, _events[i].events);
        }
      }

      //
      //  Handle Connected Socket
      //

      else {
        //
        // UNIX non-blocking i/o rules:
        //
        // (1) EPOLLERR => ERROR
        // (2) EPOLLHUP => EOF
        // (3) EPOLLIN && 0 == bytesReadable => EOF
        // (4) EPOLLIN && 0 < bytesReadable => READABLE
        // (5) EPOLLOUT => WRITABLE
        //
        // Not handling (3).  MultiplexedSocket implementations already have
        // to handle the possibility that read/recv will return 0 for EOF, so
        // doing another check here would just be inefficient.
        //

        if (_events[i].events & EPOLLERR) {
          error = TCPSocket::GetLastSocketError(socketDescriptor);

          if (_logger->isLoggable(Logger::Debug)) {
            char buffer[100];
            DescribeError(error, buffer, sizeof(buffer));
            _logger->log(Logger::Debug, __FILE__, __LINE__,
                         "[%s:%d] error on conected socket %d: %d:%s", _name,
                         _epollDescriptor, socketDescriptor, error, buffer);
          }

          keepInMultiplexer =
              multiplexedSocket->handleErrorEvent(error, isRunning, _logger);
        } else if (_events[i].events & EPOLLHUP) {
          if (_logger->isLoggable(Logger::Debug)) {
            _logger->log(Logger::Debug, __FILE__, __LINE__,
                         "[%s:%d] end of file on connected socket %d", _name,
                         _epollDescriptor, socketDescriptor);
          }

          keepInMultiplexer =
              multiplexedSocket->handleEndOfFileEvent(isRunning, _logger);
        } else {
          if (multiplexedSocket->wantRead() && (_events[i].events & EPOLLIN)) {
            if (_logger->isLoggable(Logger::Debug)) {
              _logger->log(Logger::Debug, __FILE__, __LINE__,
                           "[%s:%d] readable event for connected socket %d",
                           _name, _epollDescriptor, socketDescriptor);
            }

            keepInMultiplexer =
                multiplexedSocket->handleReadableEvent(isRunning, _logger);
          }

          if (keepInMultiplexer && multiplexedSocket->wantWrite() &&
              (_events[i].events & EPOLLOUT)) {
            if (_logger->isLoggable(Logger::Debug)) {
              _logger->log(Logger::Debug, __FILE__, __LINE__,
                           "[%s:%d] writable event for connected socket %d",
                           _name, _epollDescriptor, socketDescriptor);
            }

            keepInMultiplexer =
                multiplexedSocket->handleWritableEvent(isRunning, _logger);
          }
        }
      }

      if (keepInMultiplexer) {
        updateMultiplexedSocket(isRunning, multiplexedSocket);
      } else {
        removeMultiplexedSocket(isRunning, multiplexedSocket);
      }
    }
  }

  if (_logger->isLoggable(Logger::Notice)) {
    _logger->log(Logger::Notice, __FILE__, __LINE__, "[%s:%d] stopped", _name,
                 _epollDescriptor);
  }

  return false;  // caller should not cleanup this object after this method
                 // returns
}

int EpollMultiplexer::getCurrentSockets() { return _currentSocketCount.get(); }

int EpollMultiplexer::getMaximumSockets() { return _maxSockets; }

Error EpollMultiplexer::checkIdleSockets(SharedInt *isRunning) {
  if (_lastIdleCheckSec + IDLE_CHECK_SEC < time(0)) {
    return ESB_SUCCESS;
  }

  if (_logger->isLoggable(Logger::Debug)) {
    _logger->log(Logger::Debug, __FILE__, __LINE__,
                 "[%s:%d] starting idle socket check", _name, _epollDescriptor);
  }

  WriteScopeLock scopeLock(_lock);

  MultiplexedSocket *current =
      (MultiplexedSocket *)_currentSocketList.getFirst();
  MultiplexedSocket *next = 0;

  while (current && isRunning->get()) {
    next = (MultiplexedSocket *)current->getNext();

    if (current->isIdle()) {
      if (_logger->isLoggable(Logger::Debug)) {
        _logger->log(Logger::Debug, __FILE__, __LINE__,
                     "[%s:%d] found idle socket %d", _name, _epollDescriptor,
                     current->getSocketDescriptor());
      }

      if (current->handleIdleEvent(isRunning, _logger)) {
        updateMultiplexedSocket(isRunning, current);
      } else {
        removeMultiplexedSocket(isRunning, current, false);
        _currentSocketList.remove(current);
        assert(true == _currentSocketList.validate());
        _currentSocketCount.dec();
      }
    }

    current = next;
  }

  if (_logger->isLoggable(Logger::Debug)) {
    _logger->log(Logger::Debug, __FILE__, __LINE__,
                 "[%s:%d] finished idle socket check", _name, _epollDescriptor);
  }

  return ESB_SUCCESS;
}

}  // namespace ESB
