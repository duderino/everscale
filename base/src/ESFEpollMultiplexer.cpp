/** @file ESFEpollMultiplexer.cpp
 *  @brief A linux epoll-based implementation of ESFSocketMultiplexer
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_EPOLL_MULTIPLEXER_H
#include <ESFEpollMultiplexer.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_WRITE_SCOPE_LOCK_H
#include <ESFWriteScopeLock.h>
#endif

#ifndef ESF_READ_SCOPE_LOCK_H
#include <ESFReadScopeLock.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifndef ESF_CONNECTED_TCP_SOCKET_H
#include <ESFConnectedTCPSocket.h>
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

#define EPOLL_TIMEOUT_MILLIS 5000
#define MIN_MAX_SOCKETS 1
#define IDLE_CHECK_SEC 30

ESFEpollMultiplexer::ESFEpollMultiplexer(const char *name, int maxSockets,
                                         ESFLogger *logger,
                                         ESFAllocator *allocator)
    : ESFSocketMultiplexer(name, logger),
      _epollDescriptor(INVALID_SOCKET),
      _maxSockets(maxSockets < MIN_MAX_SOCKETS ? MIN_MAX_SOCKETS : maxSockets),
      _lastIdleCheckSec(0),
      _events(0),
      _allocator(allocator ? allocator : ESFSystemAllocator::GetInstance()),
      _currentSocketCount(),
      _currentSocketList(),
      _lock() {}

ESFEpollMultiplexer::~ESFEpollMultiplexer() {
  if (INVALID_SOCKET != _epollDescriptor) {
    close(_epollDescriptor);
    _epollDescriptor = INVALID_SOCKET;
  }
}

ESFCleanupHandler *ESFEpollMultiplexer::getCleanupHandler() { return 0; }

const char *ESFEpollMultiplexer::getName() const { return _name; }

ESFError ESFEpollMultiplexer::addMultiplexedSocket(
    ESFMultiplexedSocket *multiplexedSocket) {
  if (0 > _epollDescriptor) {
    return ESF_INVALID_STATE;
  }

  if (0 == multiplexedSocket) {
    return ESF_NULL_POINTER;
  }

  SOCKET socketDescriptor = multiplexedSocket->getSocketDescriptor();

  if (INVALID_SOCKET == socketDescriptor) {
    return ESF_INVALID_ARGUMENT;
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

    return ESF_INVALID_ARGUMENT;
  }

  int currentSocketCount = _currentSocketCount.inc();

  if (currentSocketCount > _maxSockets) {
    if (_logger->isLoggable(ESFLogger::Error)) {
      _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                   "[%s:%d] cannot add socket %d to epoll descriptor %d: at "
                   "limit of %d sockets",
                   _name, _epollDescriptor, socketDescriptor, _epollDescriptor,
                   _maxSockets);
    }

    _currentSocketCount.dec();

    return ESF_OVERFLOW;
  }

  _lock.writeAcquire();

  _currentSocketList.addLast(multiplexedSocket);
  ESF_ASSERT(true == _currentSocketList.validate());

  _lock.writeRelease();

  event.data.ptr = multiplexedSocket;

  ESFError error = ESFConvertError(
      epoll_ctl(_epollDescriptor, EPOLL_CTL_ADD, socketDescriptor, &event));

  if (ESF_SUCCESS != error) {
    if (_logger->isLoggable(ESFLogger::Error)) {
      char buffer[100];

      ESFDescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                   "[%s:%d] cannot add socket %d to epoll descriptor %d: %s",
                   _name, _epollDescriptor, socketDescriptor, _epollDescriptor,
                   buffer);
    }

    _lock.writeAcquire();

    _currentSocketList.remove(multiplexedSocket);
    ESF_ASSERT(true == _currentSocketList.validate());

    _lock.writeRelease();

    _currentSocketCount.dec();

    return error;
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[%s:%d] added socket %d to epoll descriptor %d.  Current "
                 "socket count: %d",
                 _name, _epollDescriptor, socketDescriptor, _epollDescriptor,
                 currentSocketCount);
  }

  return ESF_SUCCESS;
}

ESFError ESFEpollMultiplexer::updateMultiplexedSocket(
    ESFFlag *isRunning, ESFMultiplexedSocket *multiplexedSocket) {
  if (0 > _epollDescriptor) {
    return ESF_INVALID_STATE;
  }

  if (0 == multiplexedSocket) {
    return ESF_NULL_POINTER;
  }

  SOCKET socketDescriptor = multiplexedSocket->getSocketDescriptor();

  if (INVALID_SOCKET == socketDescriptor) {
    return ESF_INVALID_ARGUMENT;
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

    return ESF_INVALID_ARGUMENT;
  }

  event.data.ptr = multiplexedSocket;

  ESFError error = ESFConvertError(
      epoll_ctl(_epollDescriptor, EPOLL_CTL_MOD, socketDescriptor, &event));

  if (ESF_SUCCESS != error) {
    if (_logger->isLoggable(ESFLogger::Error)) {
      char buffer[100];

      ESFDescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                   "[%s:%d] cannot update socket %d in epoll descriptor %d: %s",
                   _name, _epollDescriptor, socketDescriptor, _epollDescriptor,
                   buffer);
    }

    return error;
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[%s:%d] updated socket %d in epoll descriptor %d", _name,
                 _epollDescriptor, socketDescriptor, _epollDescriptor);
  }

  return ESF_SUCCESS;
}

ESFError ESFEpollMultiplexer::removeMultiplexedSocket(
    ESFFlag *isRunning, ESFMultiplexedSocket *multiplexedSocket,
    bool removeFromList) {
  if (0 > _epollDescriptor) {
    return ESF_INVALID_STATE;
  }

  if (0 == multiplexedSocket) {
    return ESF_NULL_POINTER;
  }

  SOCKET socketDescriptor = multiplexedSocket->getSocketDescriptor();

  if (INVALID_SOCKET == socketDescriptor) {
    return ESF_INVALID_ARGUMENT;
  }

  ESFError error = ESFConvertError(
      epoll_ctl(_epollDescriptor, EPOLL_CTL_DEL, socketDescriptor, 0));

  if (ESF_SUCCESS != error) {
    if (_logger->isLoggable(ESFLogger::Error)) {
      char buffer[100];

      ESFDescribeError(error, buffer, sizeof(buffer));

      _logger->log(
          ESFLogger::Error, __FILE__, __LINE__,
          "[%s:%d] cannot remove socket %d from epoll descriptor %d: %s", _name,
          _epollDescriptor, socketDescriptor, _epollDescriptor, buffer);
    }

    return error;
  }

  int currentSocketCount = 0;

  if (removeFromList) {
    _lock.writeAcquire();

    _currentSocketList.remove(multiplexedSocket);
    ESF_ASSERT(true == _currentSocketList.validate());

    _lock.writeRelease();

    currentSocketCount = _currentSocketCount.dec();
  } else {
    currentSocketCount = _currentSocketCount.get();
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[%s:%d] removed socket %d from epoll descriptor %d. Current "
                 "socket count: %d",
                 _name, _epollDescriptor, socketDescriptor, _epollDescriptor,
                 currentSocketCount);
  }

  if (multiplexedSocket->handleRemoveEvent(isRunning, _logger)) {
    ESFCleanupHandler *cleanupHandler = multiplexedSocket->getCleanupHandler();

    if (cleanupHandler) {
      cleanupHandler->destroy(multiplexedSocket);
    }
  }

  return ESF_SUCCESS;
}

ESFError ESFEpollMultiplexer::initialize() {
  if (_logger->isLoggable(ESFLogger::Notice)) {
    _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s] initializing",
                 _name);
  }

  _events = (struct epoll_event *)_allocator->allocate(
      sizeof(struct epoll_event) * _maxSockets);

  if (0 == _events) {
    if (_logger->isLoggable(ESFLogger::Critical)) {
      _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                   "[%s] cannot alloc epoll_event array of size %d", _name,
                   _maxSockets);
    }

    return ESF_OUT_OF_MEMORY;
  }

  _epollDescriptor = epoll_create(_maxSockets);

  if (0 > _epollDescriptor) {
    ESFError error = ESFGetLastError();

    if (_logger->isLoggable(ESFLogger::Critical)) {
      char buffer[100];

      ESFDescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                   "[%s] cannot create epoll descriptor: %s", _name, buffer);
    }

    _allocator->deallocate(_events);
    _events = 0;

    return error;
  }

  if (_logger->isLoggable(ESFLogger::Notice)) {
    _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s:%d] initialized",
                 _name, _epollDescriptor);
  }

  return ESF_SUCCESS;
}

void ESFEpollMultiplexer::destroy() {
  if (_logger->isLoggable(ESFLogger::Notice)) {
    _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s:%d] destroying",
                 _name, _epollDescriptor);
  }

  ESFMultiplexedSocket *head = 0;
  ESFFlag isRunning(false);

  while (true) {
    _lock.writeAcquire();

    head = (ESFMultiplexedSocket *)_currentSocketList.removeFirst();
    ESF_ASSERT(true == _currentSocketList.validate());

    _lock.writeRelease();

    if (0 == head) {
      break;
    }

    _currentSocketCount.dec();

    removeMultiplexedSocket(&isRunning, head, false);
  }

  ESF_ASSERT(_currentSocketCount.get() == _currentSocketList.length());

  if (INVALID_SOCKET != _epollDescriptor) {
    close(_epollDescriptor);
    _epollDescriptor = INVALID_SOCKET;
  }

  if (_events) {
    _allocator->deallocate(_events);
    _events = 0;
  }

  if (_logger->isLoggable(ESFLogger::Notice)) {
    _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s] destroyed",
                 _name);
  }
}

bool ESFEpollMultiplexer::run(ESFFlag *isRunning) {
  if (INVALID_SOCKET == _epollDescriptor) {
    return false;
  }

  if (!_events) {
    return false;
  }

  if (!isRunning) {
    return false;
  }

  if (_logger->isLoggable(ESFLogger::Notice)) {
    _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s:%d] started",
                 _name, _epollDescriptor);
  }

  int numEvents = 0;
  int i = 0;
  ESFError error = ESF_SUCCESS;
  ESFMultiplexedSocket *multiplexedSocket = 0;
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
      error = ESFGetLastError();

      if (ESF_INTR == error) {
        continue;
      }

      if (_logger->isLoggable(ESFLogger::Critical)) {
        char buffer[100];

        ESFDescribeError(error, buffer, sizeof(buffer));

        _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                     "[%s:%d] error in epoll_wait: %s", _name, _epollDescriptor,
                     buffer);
      }

      if (errorCount >= 10) {
        if (_logger->isLoggable(ESFLogger::Emergency)) {
          _logger->log(
              ESFLogger::Emergency, __FILE__, __LINE__,
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

      multiplexedSocket = (ESFMultiplexedSocket *)_events[i].data.ptr;

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
          error = ESFTCPSocket::GetLastError(socketDescriptor);

          if (_logger->isLoggable(ESFLogger::Error)) {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                         "[%s:%d] error on listening socket %d: %s", _name,
                         _epollDescriptor, socketDescriptor, buffer);
          }

          keepInMultiplexer =
              multiplexedSocket->handleErrorEvent(error, isRunning, _logger);
        } else if (_events[i].events & EPOLLIN) {
          if (_logger->isLoggable(ESFLogger::Debug)) {
            _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                         "[%s:%d] accept event on listening socket %d", _name,
                         _epollDescriptor, socketDescriptor);
          }

          keepInMultiplexer =
              multiplexedSocket->handleAcceptEvent(isRunning, _logger);
        } else if (_logger->isLoggable(ESFLogger::Error)) {
          _logger->log(
              ESFLogger::Error, __FILE__, __LINE__,
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
          error = ESFTCPSocket::GetLastError(socketDescriptor);

          if (_logger->isLoggable(ESFLogger::Warning)) {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                         "[%s:%d] error on conecting socket %d: %s", _name,
                         _epollDescriptor, socketDescriptor, buffer);
          }

          keepInMultiplexer =
              multiplexedSocket->handleErrorEvent(error, isRunning, _logger);
        } else if (_events[i].events & EPOLLHUP) {
          if (_logger->isLoggable(ESFLogger::Warning)) {
            _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                         "[%s:%d] end of file on connecting socket %d", _name,
                         _epollDescriptor, socketDescriptor);
          }

          keepInMultiplexer =
              multiplexedSocket->handleEndOfFileEvent(isRunning, _logger);
        } else if (_events[i].events & EPOLLIN) {
          int bytesReadable =
              ESFConnectedTCPSocket::GetBytesReadable(socketDescriptor);

          if (0 > bytesReadable) {
            error = ESFTCPSocket::GetLastError(socketDescriptor);

            if (_logger->isLoggable(ESFLogger::Warning)) {
              char buffer[100];

              ESFDescribeError(error, buffer, sizeof(buffer));

              _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
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
        } else if (_logger->isLoggable(ESFLogger::Error)) {
          _logger->log(
              ESFLogger::Error, __FILE__, __LINE__,
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
        // Not handling (3).  ESFMultiplexedSocket implementations already have
        // to handle the possibility that read/recv will return 0 for EOF, so
        // doing another check here would just be inefficient.
        //

        if (_events[i].events & EPOLLERR) {
          error = ESFTCPSocket::GetLastError(socketDescriptor);

          if (_logger->isLoggable(ESFLogger::Notice)) {
            char buffer[100];

            ESFDescribeError(error, buffer, sizeof(buffer));

            _logger->log(ESFLogger::Notice, __FILE__, __LINE__,
                         "[%s:%d] error on conected socket %d: %d:%s", _name,
                         _epollDescriptor, socketDescriptor, error, buffer);
          }

          keepInMultiplexer =
              multiplexedSocket->handleErrorEvent(error, isRunning, _logger);
        } else if (_events[i].events & EPOLLHUP) {
          if (_logger->isLoggable(ESFLogger::Debug)) {
            _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                         "[%s:%d] end of file on connected socket %d", _name,
                         _epollDescriptor, socketDescriptor);
          }

          keepInMultiplexer =
              multiplexedSocket->handleEndOfFileEvent(isRunning, _logger);
        } else {
          if (multiplexedSocket->wantRead() && (_events[i].events & EPOLLIN)) {
            if (_logger->isLoggable(ESFLogger::Debug)) {
              _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                           "[%s:%d] readable event for connected socket %d",
                           _name, _epollDescriptor, socketDescriptor);
            }

            keepInMultiplexer =
                multiplexedSocket->handleReadableEvent(isRunning, _logger);
          }

          if (keepInMultiplexer && multiplexedSocket->wantWrite() &&
              (_events[i].events & EPOLLOUT)) {
            if (_logger->isLoggable(ESFLogger::Debug)) {
              _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
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

  if (_logger->isLoggable(ESFLogger::Notice)) {
    _logger->log(ESFLogger::Notice, __FILE__, __LINE__, "[%s:%d] stopped",
                 _name, _epollDescriptor);
  }

  return false;  // caller should not cleanup this object after this method
                 // returns
}

int ESFEpollMultiplexer::getCurrentSockets() {
  return _currentSocketCount.get();
}

int ESFEpollMultiplexer::getMaximumSockets() { return _maxSockets; }

ESFError ESFEpollMultiplexer::checkIdleSockets(ESFFlag *isRunning) {
  if (_lastIdleCheckSec + IDLE_CHECK_SEC < time(0)) {
    return ESF_SUCCESS;
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[%s:%d] starting idle socket check", _name, _epollDescriptor);
  }

  ESFWriteScopeLock scopeLock(_lock);

  ESFMultiplexedSocket *current =
      (ESFMultiplexedSocket *)_currentSocketList.getFirst();
  ESFMultiplexedSocket *next = 0;

  while (current && isRunning->get()) {
    next = (ESFMultiplexedSocket *)current->getNext();

    if (current->isIdle()) {
      if (_logger->isLoggable(ESFLogger::Debug)) {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[%s:%d] found idle socket %d", _name, _epollDescriptor,
                     current->getSocketDescriptor());
      }

      if (current->handleIdleEvent(isRunning, _logger)) {
        updateMultiplexedSocket(isRunning, current);
      } else {
        //
        // martin fowler says code comments are 'code smells'....
        //
        // in theory, if the idle timeout was insanely low, we could have a race
        // condition here.  the addMultiplexedSocket code first adds the socket
        // to the _connectedSocketsList, then it adds to the _epollDescriptor.
        // This code could potentially try to remove a socket in the
        // _connectedSocketList before it has been added to the
        // _epollDescriptor. But that would mean that the socket goes idle
        // before the adding thread runs again.  Unlikely to impossible.
        //

        _currentSocketList.remove(current);
        ESF_ASSERT(true == _currentSocketList.validate());

        _currentSocketCount.dec();

        removeMultiplexedSocket(isRunning, current, false);
      }
    }

    current = next;
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[%s:%d] finished idle socket check", _name, _epollDescriptor);
  }

  return ESF_SUCCESS;
}
