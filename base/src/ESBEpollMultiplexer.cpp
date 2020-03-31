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

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

namespace ESB {

#define EPOLL_TIMEOUT_MILLIS 1000
#define MIN_MAX_SOCKETS 1
#define IDLE_CHECK_SEC 30

EpollMultiplexer::EpollMultiplexer(UInt32 maxSockets, Allocator &allocator,
                                   Lockable &lock)
    : SocketMultiplexer(),
      _epollDescriptor(INVALID_SOCKET),
      _maxSockets(maxSockets < MIN_MAX_SOCKETS ? MIN_MAX_SOCKETS : maxSockets),
      _lastIdleCheckSec(0),
      _events(NULL),
      _allocator(allocator),
      _lock(lock),
      _currentSocketCount(),
      _currentSocketList() {}

EpollMultiplexer::~EpollMultiplexer() {
  if (INVALID_SOCKET != _epollDescriptor) {
    close(_epollDescriptor);
    _epollDescriptor = INVALID_SOCKET;
  }

  if (_events) {
    _allocator.deallocate(_events);
    _events = NULL;
  }
}

CleanupHandler *EpollMultiplexer::cleanupHandler() { return 0; }

Error EpollMultiplexer::addMultiplexedSocket(MultiplexedSocket *socket) {
  if (0 > _epollDescriptor) {
    return ESB_INVALID_STATE;
  }

  if (!socket) {
    return ESB_NULL_POINTER;
  }

  SOCKET fd = socket->socketDescriptor();

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
    ESB_LOG_ERROR("Cannot add socket %d, socket wants nothing", fd);
    return ESB_INVALID_ARGUMENT;
  }

  int currentSocketCount = _currentSocketCount.inc();

  if (currentSocketCount > _maxSockets) {
    ESB_LOG_ERROR("Cannot add socket %d: at limit of %d sockets", fd,
                  _maxSockets);
    _currentSocketCount.dec();
    return ESB_OVERFLOW;
  }

  _lock.writeAcquire();
  _currentSocketList.addLast(socket);
  assert(_currentSocketList.validate());
  _lock.writeRelease();

  event.data.ptr = socket;

  if (0 != epoll_ctl(_epollDescriptor, EPOLL_CTL_ADD, fd, &event)) {
    Error error = LastError();
    ESB_LOG_ERROR_ERRNO(error, "Cannot add socket %d", fd);
    _lock.writeAcquire();
    _currentSocketList.remove(socket);
    assert(_currentSocketList.validate());
    _lock.writeRelease();
    _currentSocketCount.dec();
    return error;
  }

  ESB_LOG_DEBUG("Added socket %d", fd);
  return ESB_SUCCESS;
}

Error EpollMultiplexer::updateMultiplexedSocket(MultiplexedSocket *socket) {
  if (0 > _epollDescriptor) {
    return ESB_INVALID_STATE;
  }

  if (!socket) {
    return ESB_NULL_POINTER;
  }

  SOCKET fd = socket->socketDescriptor();

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
    ESB_LOG_ERROR("Cannot update socket %d, socket wants nothing", fd);
    return ESB_INVALID_ARGUMENT;
  }

  event.data.ptr = socket;

  if (0 != epoll_ctl(_epollDescriptor, EPOLL_CTL_MOD, fd, &event)) {
    Error error = LastError();
    ESB_LOG_ERROR_ERRNO(error, "Cannot update socket %d", fd);
    return error;
  }

  ESB_LOG_DEBUG("Updated socket %d", fd);
  return ESB_SUCCESS;
}

Error EpollMultiplexer::removeMultiplexedSocket(MultiplexedSocket *socket,
                                                bool removeFromList) {
  if (0 > _epollDescriptor) {
    return ESB_INVALID_STATE;
  }

  if (!socket) {
    return ESB_NULL_POINTER;
  }

  SOCKET fd = socket->socketDescriptor();

  assert(INVALID_SOCKET != fd);
  if (INVALID_SOCKET == fd) {
    return ESB_INVALID_ARGUMENT;
  }

  if (0 != epoll_ctl(_epollDescriptor, EPOLL_CTL_DEL, fd, 0)) {
    Error error = LastError();
    ESB_LOG_ERROR_ERRNO(error, "Cannot remove socket %d", fd);
    return error;
  }

  int currentSocketCount = 0;

  if (removeFromList) {
    _lock.writeAcquire();
    _currentSocketList.remove(socket);
    assert(_currentSocketList.validate());
    _lock.writeRelease();

    currentSocketCount = _currentSocketCount.dec();
  } else {
    currentSocketCount = _currentSocketCount.get();
  }

  ESB_LOG_DEBUG("Removed socket %d", fd);

  if (socket->handleRemove(*this)) {
    CleanupHandler *cleanupHandler = socket->cleanupHandler();
    if (cleanupHandler) {
      cleanupHandler->destroy(socket);
    }
  }

  return ESB_SUCCESS;
}

Error EpollMultiplexer::initialize() {
  ESB_LOG_NOTICE("Initializing multiplexer");

  _events = (struct epoll_event *)_allocator.allocate(
      sizeof(struct epoll_event) * _maxSockets);

  if (!_events) {
    ESB_LOG_CRITICAL("Cannot allocate %d epoll events", _maxSockets);
    return ESB_OUT_OF_MEMORY;
  }

  _epollDescriptor = epoll_create(_maxSockets);

  if (0 > _epollDescriptor) {
    Error error = LastError();
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot acquire epoll descriptor");
    _allocator.deallocate(_events);
    _events = NULL;
    return error;
  }

  ESB_LOG_NOTICE("Multiplexer initialized");
  return ESB_SUCCESS;
}

void EpollMultiplexer::destroy() {
  ESB_LOG_NOTICE("Destroying multiplexer");

  MultiplexedSocket *head = 0;

  while (true) {
    _lock.writeAcquire();
    head = (MultiplexedSocket *)_currentSocketList.removeFirst();
    assert(_currentSocketList.validate());
    _lock.writeRelease();

    if (!head) {
      break;
    }

    _currentSocketCount.dec();

    removeMultiplexedSocket(head, false);
  }

#ifndef NDEBUG
  _lock.readAcquire();
  assert(_currentSocketCount.get() == _currentSocketList.size());
  _lock.readRelease();
#endif

  if (INVALID_SOCKET != _epollDescriptor) {
    close(_epollDescriptor);
    _epollDescriptor = INVALID_SOCKET;
  }

  if (_events) {
    _allocator.deallocate(_events);
    _events = NULL;
  }

  ESB_LOG_NOTICE("Multiplexer destroyed");
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

  ESB_LOG_NOTICE("Multiplexer started");

  int numEvents = 0;
  int i = 0;
  Error error = ESB_SUCCESS;
  MultiplexedSocket *socket = 0;
  int errorCount = 0;
  SOCKET fd = INVALID_SOCKET;
  bool keepInMultiplexer = true;
  _isRunning = isRunning;

  while (_isRunning->get()) {
    checkIdleSockets(isRunning);
    numEvents = epoll_wait(_epollDescriptor, _events, _maxSockets,
                           EPOLL_TIMEOUT_MILLIS);

    if (0 == numEvents) {
      // Timeout
      continue;
    }

    if (0 > numEvents) {
      error = LastError();
      if (ESB_INTR == error) {
        continue;
      }

      ESB_LOG_ERROR_ERRNO(error, "Error in epoll_wait");

      if (errorCount >= 10) {
        ESB_LOG_CRITICAL("Too many errors in epoll_wait, exiting");
        _allocator.deallocate(_events);
        _events = NULL;
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
      fd = socket->socketDescriptor();

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
          error = TCPSocket::LastSocketError(fd);
          ESB_LOG_ERROR_ERRNO(error, "Error on listening socket %d", fd);
          keepInMultiplexer = socket->handleError(error, *this);
        } else if (_events[i].events & EPOLLIN) {
          ESB_LOG_DEBUG("Accept event on listening socket %d", fd);
          keepInMultiplexer = socket->handleAccept(*this);
        } else {
          ESB_LOG_WARNING("Unknown event on listening socket %d: %d", fd,
                          _events[i].events);
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
          error = TCPSocket::LastSocketError(fd);
          ESB_LOG_INFO_ERRNO(error, "Error on connecting socket %d", fd);
          keepInMultiplexer = socket->handleError(error, *this);
        } else if (_events[i].events & EPOLLHUP) {
          ESB_LOG_INFO("End of file on connecting socket %d", fd);
          keepInMultiplexer = socket->handleRemoteClose(*this);
        } else if (_events[i].events & EPOLLIN) {
          int bytesReadable = ConnectedTCPSocket::BytesReadable(fd);

          if (0 > bytesReadable) {
            error = TCPSocket::LastSocketError(fd);
            ESB_LOG_INFO_ERRNO(error, "Error on connecting socket %d", fd);
            keepInMultiplexer = socket->handleError(error, *this);
          } else if (0 == bytesReadable) {
            ESB_LOG_INFO("Immediate close on connecting socket %d", fd);
            keepInMultiplexer = socket->handleRemoteClose(*this);
          } else {
            ESB_LOG_INFO("Connecting socket %d connected", fd);
            keepInMultiplexer = socket->handleConnect(*this);
          }
        } else if (_events[i].events & EPOLLOUT) {
          ESB_LOG_INFO("Connecting socket %d connected", fd);
          keepInMultiplexer = socket->handleConnect(*this);
        } else {
          ESB_LOG_WARNING("Unknown event on connecting socket %d: %d", fd,
                          _events[i].events);
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
          error = TCPSocket::LastSocketError(fd);
          ESB_LOG_INFO_ERRNO(error, "Error on connected socket %d", fd);
          keepInMultiplexer = socket->handleError(error, *this);
        } else if (_events[i].events & EPOLLHUP) {
          ESB_LOG_INFO("End of file on connected socket %d", fd);
          keepInMultiplexer = socket->handleRemoteClose(*this);
        } else {
          if (socket->wantRead() && (_events[i].events & EPOLLIN)) {
            ESB_LOG_DEBUG("Readable event for connected socket %d", fd);
            keepInMultiplexer = socket->handleReadable(*this);
          }

          if (keepInMultiplexer && socket->wantWrite() &&
              (_events[i].events & EPOLLOUT)) {
            ESB_LOG_DEBUG("Writable event for connected socket %d", fd);
            keepInMultiplexer = socket->handleWritable(*this);
          }
        }
      }

      if (keepInMultiplexer) {
        updateMultiplexedSocket(socket);
      } else {
        removeMultiplexedSocket(socket);
      }
    }
  }

  ESB_LOG_NOTICE("Multiplexer stopped");
  return false;
}

int EpollMultiplexer::currentSockets() const {
  return _currentSocketCount.get();
}

int EpollMultiplexer::maximumSockets() const { return _maxSockets; }

bool EpollMultiplexer::isRunning() const {
  return _isRunning && _isRunning->get();
}

Error EpollMultiplexer::checkIdleSockets(SharedInt *isRunning) {
  if (_lastIdleCheckSec + IDLE_CHECK_SEC < time(0)) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("Starting idle socket check");

  WriteScopeLock scopeLock(_lock);

  MultiplexedSocket *current = (MultiplexedSocket *)_currentSocketList.first();
  MultiplexedSocket *next = 0;

  while (current && isRunning->get()) {
    next = (MultiplexedSocket *)current->next();

    if (current->isIdle()) {
      ESB_LOG_DEBUG("Found idle socket %d", current->socketDescriptor());

      if (current->handleIdle(*this)) {
        updateMultiplexedSocket(current);
      } else {
        removeMultiplexedSocket(current, false);
        _currentSocketList.remove(current);
        assert(_currentSocketList.validate());
        _currentSocketCount.dec();
      }
    }

    current = next;
  }

  ESB_LOG_DEBUG("Finished idle socket check");
  return ESB_SUCCESS;
}

}  // namespace ESB
