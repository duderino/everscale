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

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
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

EpollMultiplexer::EpollMultiplexer(const char *name, int maxSockets, Allocator *allocator)
    : SocketMultiplexer(name),
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

Error EpollMultiplexer::addMultiplexedSocket(MultiplexedSocket *socket) {
  if (0 > _epollDescriptor) {
    return ESB_INVALID_STATE;
  }

  if (0 == socket) {
    return ESB_NULL_POINTER;
  }

  SOCKET fd = socket->getSocketDescriptor();

  assert(INVALID_SOCKET != fd);
  if (INVALID_SOCKET == fd) {
    return ESB_INVALID_ARGUMENT;
  }

  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = EPOLLERR | EPOLLONESHOT;

  if (socket->wantAccept()) {
    event.events |= EPOLLIN;
  } else if (socket->wantConnect()) {
    event.events |= EPOLLIN | EPOLLOUT | EPOLLHUP;
  } else {
    if (socket->wantWrite()) {
      event.events |= EPOLLOUT | EPOLLHUP;
    }
    if (socket->wantRead()) {
      event.events |= EPOLLIN | EPOLLHUP;
    }
  }

  if ((EPOLLERR | EPOLLONESHOT) == event.events) {
    // means wantAccept(), wantConnect(), wantRead() and wantWrite() all
    // returned false
    ESB_LOG_ERROR("Cannot add socket %d to %s:%d, socket wants nothing",
        fd, _name, _epollDescriptor);
    return ESB_INVALID_ARGUMENT;
  }

  int currentSocketCount = _currentSocketCount.inc();

  if (currentSocketCount > _maxSockets) {
    ESB_LOG_ERROR("Cannot add socket %d to %s:%d: at limit of %d sockets",
                  fd,_name, _epollDescriptor, _maxSockets);
    _currentSocketCount.dec();
    return ESB_OVERFLOW;
  }

  _lock.writeAcquire();
  _currentSocketList.addLast(socket);
  assert(true == _currentSocketList.validate());
  _lock.writeRelease();

  event.data.ptr = socket;

  if (0 != epoll_ctl(_epollDescriptor, EPOLL_CTL_ADD, fd, &event)) {
    Error error = GetLastError();
    ESB_LOG_ERRNO_ERROR(error, "Cannot add socket %d to %s:%d",fd, _name, _epollDescriptor);

    _lock.writeAcquire();
    _currentSocketList.remove(socket);
    assert(true == _currentSocketList.validate());
    _lock.writeRelease();
    _currentSocketCount.dec();
    return error;
  }

  ESB_LOG_DEBUG("Added socket %d to %s:%d",fd, _name, _epollDescriptor);
  return ESB_SUCCESS;
}

Error EpollMultiplexer::updateMultiplexedSocket(SharedInt *isRunning, MultiplexedSocket *socket) {
  if (0 > _epollDescriptor) {
    return ESB_INVALID_STATE;
  }

  if (0 == socket) {
    return ESB_NULL_POINTER;
  }

  SOCKET fd = socket->getSocketDescriptor();

  assert(INVALID_SOCKET != fd);
  if (INVALID_SOCKET == fd) {
    return ESB_INVALID_ARGUMENT;
  }

  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = EPOLLERR | EPOLLONESHOT;

  if (socket->wantAccept()) {
    event.events |= EPOLLIN;
  } else if (socket->wantConnect()) {
    event.events |= EPOLLIN | EPOLLOUT | EPOLLHUP;
  } else {
    if (socket->wantWrite()) {
      event.events |= EPOLLOUT | EPOLLHUP;
    }

    if (socket->wantRead()) {
      event.events |= EPOLLIN | EPOLLHUP;
    }
  }

  if ((EPOLLERR | EPOLLONESHOT) == event.events) {
    // means wantAccept(), wantConnect(), wantRead() and wantWrite() all
    // returned false
    ESB_LOG_ERROR("Cannot update socket %d in %s:%d, socket wants nothing",
                  fd, _name, _epollDescriptor);
    return ESB_INVALID_ARGUMENT;
  }

  event.data.ptr = socket;

  if (0 != epoll_ctl(_epollDescriptor, EPOLL_CTL_MOD, fd, &event)) {
    Error error = GetLastError();
    ESB_LOG_ERRNO_ERROR(error, "Cannot update socket %d in %s:%d",
                  fd, _name, _epollDescriptor);
    return error;
  }

  ESB_LOG_DEBUG("Updated socket %d in %s:%d", fd, _name, _epollDescriptor);
  return ESB_SUCCESS;
}

Error EpollMultiplexer::removeMultiplexedSocket(SharedInt *isRunning, MultiplexedSocket *socket, bool removeFromList) {
  if (0 > _epollDescriptor) {
    return ESB_INVALID_STATE;
  }

  if (!socket) {
    return ESB_NULL_POINTER;
  }

  SOCKET fd = socket->getSocketDescriptor();

  assert(INVALID_SOCKET != fd);
  if (INVALID_SOCKET == fd) {
    return ESB_INVALID_ARGUMENT;
  }

  if (0 != epoll_ctl(_epollDescriptor, EPOLL_CTL_DEL, fd, 0)) {
    Error error = GetLastError();
    ESB_LOG_ERRNO_ERROR(error, "Cannot remove socket %d from %s:%d",
          fd, _name, _epollDescriptor);
    return error;
  }

  int currentSocketCount = 0;

  if (removeFromList) {
    _lock.writeAcquire();
    _currentSocketList.remove(socket);
    assert(true == _currentSocketList.validate());
    _lock.writeRelease();

    currentSocketCount = _currentSocketCount.dec();
  } else {
    currentSocketCount = _currentSocketCount.get();
  }

  ESB_LOG_DEBUG("Removed socket %d from %s:%d", fd, _name, _epollDescriptor);

  if (socket->handleRemoveEvent(isRunning)) {
    CleanupHandler *cleanupHandler = socket->getCleanupHandler();
    assert(cleanupHandler);
    if (cleanupHandler) {
      cleanupHandler->destroy(socket);
    }
  }

  return ESB_SUCCESS;
}

Error EpollMultiplexer::initialize() {
  ESB_LOG_NOTICE("Initializing multiplexer '%s'", _name);

  _events = (struct epoll_event *)_allocator->allocate(
      sizeof(struct epoll_event) * _maxSockets);

  if (!_events) {
    ESB_LOG_CRITICAL("Cannot allocate %d epoll events for '%s'", _maxSockets, _name);
    return ESB_OUT_OF_MEMORY;
  }

  _epollDescriptor = epoll_create(_maxSockets);

  if (0 > _epollDescriptor) {
    Error error = GetLastError();
    ESB_LOG_ERRNO_CRITICAL(error, "Cannot create epoll descriptor for '%s'", _name);
    _allocator->deallocate(_events);
    _events = 0;
    return error;
  }

  ESB_LOG_NOTICE("Multiplexer '%s' initialized", _name);
  return ESB_SUCCESS;
}

void EpollMultiplexer::destroy() {
  ESB_LOG_NOTICE("Destroying multiplexer '%s'", _name);

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

  ESB_LOG_NOTICE("Multiplexer '%s' destroyed", _name);
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

  ESB_LOG_NOTICE("Multiplexer '%s' started", _name);

  int numEvents = 0;
  int i = 0;
  Error error = ESB_SUCCESS;
  MultiplexedSocket *socket = 0;
  int errorCount = 0;
  SOCKET fd = INVALID_SOCKET;
  bool keepInMultiplexer = true;

  while (isRunning->get()) {
    checkIdleSockets(isRunning);
    numEvents = epoll_wait(_epollDescriptor, _events, _maxSockets,
                           EPOLL_TIMEOUT_MILLIS);

    if (0 == numEvents) {
      // Timeout
      continue;
    }

    if (0 > numEvents) {
      error = GetLastError();
      if (ESB_INTR == error) {
        continue;
      }

      ESB_LOG_ERRNO_ERROR(error, "%s:%d error in epoll_wait", _name, _epollDescriptor);

      if (errorCount >= 10) {
        ESB_LOG_CRITICAL("%s:%d too many errors in epoll_wait, exiting",
              _name, _epollDescriptor);

        _allocator->deallocate(_events);
        close(_epollDescriptor);
        _epollDescriptor = INVALID_SOCKET;

        return false;
      }

      ++errorCount;
      continue;
    }

    errorCount = 0;

    for (i = 0; i < numEvents; ++i) {
      keepInMultiplexer = true;
      socket = (MultiplexedSocket *)_events[i].data.ptr;
      fd = socket->getSocketDescriptor();

      //
      // Handle Listening Socket
      //

      if (socket->wantAccept()) {
        //
        // UNIX non-blocking accept rules:
        //
        // (1) ! EPOLLERR && ! EPOLLIN => IN PROGRESS
        // (2) EPOLLERR => ERROR.  getsockopt() or next accept() will return
        // errno (3) EPOLLIN => ACCEPT
        //

        if (_events[i].events & EPOLLERR) {
          error = TCPSocket::GetLastSocketError(fd);
          ESB_LOG_ERRNO_ERROR(error, "%s:%d error on listening socket %d",
              _name, _epollDescriptor, fd);
          keepInMultiplexer = socket->handleErrorEvent(error, isRunning);
        } else if (_events[i].events & EPOLLIN) {
          ESB_LOG_INFO("%s:%d accept event on listening socket %d", _name,
                         _epollDescriptor, fd);
          keepInMultiplexer = socket->handleAcceptEvent(isRunning);
        } else {
          ESB_LOG_WARNING("%s:%d unknown event on listening socket %d: %d",
              _name, _epollDescriptor, fd, _events[i].events);
        }
      }

      //
      //  Handle Connecting Socket
      //

      else if (socket->wantConnect()) {
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
          error = TCPSocket::GetLastSocketError(fd);
          ESB_LOG_ERRNO_INFO(error, "%s:%d error on connecting socket %d",
              _name, _epollDescriptor, fd);
          keepInMultiplexer = socket->handleErrorEvent(error, isRunning);
        } else if (_events[i].events & EPOLLHUP) {
          ESB_LOG_INFO("%s:%d end of file on connecting socket %d", _name,
                         _epollDescriptor, fd);
          keepInMultiplexer = socket->handleEndOfFileEvent(isRunning);
        } else if (_events[i].events & EPOLLIN) {
          int bytesReadable = ConnectedTCPSocket::GetBytesReadable(fd);

          if (0 > bytesReadable) {
            error = TCPSocket::GetLastSocketError(fd);
            ESB_LOG_ERRNO_INFO(error, "%s:%d error on connecting socket %d",
                _name, _epollDescriptor, fd);
            keepInMultiplexer = socket->handleErrorEvent(error, isRunning);
          } else if (0 == bytesReadable) {
            ESB_LOG_INFO("%s:%d immediate close on connecting socket %d",
                          _name, _epollDescriptor, fd);
            keepInMultiplexer = socket->handleEndOfFileEvent(isRunning);
          } else {
            ESB_LOG_INFO("%s:%d connecting socket %d connected",
                         _name, _epollDescriptor, fd);
            keepInMultiplexer = socket->handleConnectEvent(isRunning);
          }
        } else if (_events[i].events & EPOLLOUT) {
          ESB_LOG_INFO("%s:%d connecting socket %d connected",
                       _name, _epollDescriptor, fd);
          keepInMultiplexer = socket->handleConnectEvent(isRunning);
        } else {
          ESB_LOG_WARNING("%s:%d unknown event on connecting socket %d: %d",
                          _name, _epollDescriptor, fd, _events[i].events);
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
          error = TCPSocket::GetLastSocketError(fd);
          ESB_LOG_ERRNO_INFO(error, "%s:%d error on connected socket %d", 
              _name, _epollDescriptor, fd);
          keepInMultiplexer = socket->handleErrorEvent(error, isRunning);
        } else if (_events[i].events & EPOLLHUP) {
          ESB_LOG_INFO("%s:%d end of file on connected socket %d", _name,
                         _epollDescriptor, fd);
          keepInMultiplexer = socket->handleEndOfFileEvent(isRunning);
        } else {
          if (socket->wantRead() && (_events[i].events & EPOLLIN)) {
            ESB_LOG_DEBUG("%s:%d readable event for connected socket %d",
                           _name, _epollDescriptor, fd);
            keepInMultiplexer = socket->handleReadableEvent(isRunning);
          }

          if (keepInMultiplexer && socket->wantWrite() &&
              (_events[i].events & EPOLLOUT)) {
            ESB_LOG_DEBUG("%s:%d writable event for connected socket %d",
                           _name, _epollDescriptor, fd);
            keepInMultiplexer = socket->handleWritableEvent(isRunning);
          }
        }
      }

      if (keepInMultiplexer) {
        updateMultiplexedSocket(isRunning, socket);
      } else {
        removeMultiplexedSocket(isRunning, socket);
      }
    }
  }

  ESB_LOG_NOTICE("Multiplexer '%s' stopped", _name);
  return false;
}

int EpollMultiplexer::getCurrentSockets() { return _currentSocketCount.get(); }

int EpollMultiplexer::getMaximumSockets() { return _maxSockets; }

Error EpollMultiplexer::checkIdleSockets(SharedInt *isRunning) {
  if (_lastIdleCheckSec + IDLE_CHECK_SEC < time(0)) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("%s:%d starting idle socket check", _name, _epollDescriptor);

  WriteScopeLock scopeLock(_lock);

  MultiplexedSocket *current =
      (MultiplexedSocket *)_currentSocketList.getFirst();
  MultiplexedSocket *next = 0;

  while (current && isRunning->get()) {
    next = (MultiplexedSocket *)current->getNext();

    if (current->isIdle()) {
      ESB_LOG_DEBUG("%s:%d found idle socket %d", _name, _epollDescriptor,
                     current->getSocketDescriptor());

      if (current->handleIdleEvent(isRunning)) {
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

  ESB_LOG_DEBUG("%s:%d finished idle socket check", _name, _epollDescriptor);
  return ESB_SUCCESS;
}

}  // namespace ESB
