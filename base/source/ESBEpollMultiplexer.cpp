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

EpollMultiplexer::EpollMultiplexer(const char *namePrefix, UInt32 maxSockets, Allocator &allocator, Lockable &lock)
    : SocketMultiplexer(),
      _epollDescriptor(INVALID_SOCKET),
      _maxSockets(maxSockets < MIN_MAX_SOCKETS ? MIN_MAX_SOCKETS : maxSockets),
      _lastIdleCheckSec(0),
      _events(NULL),
      _eventCache(NULL),
      _allocator(allocator),
      _lock(lock),
      _currentSocketCount(),
      _currentSocketList() {
  strncpy(_namePrefix, namePrefix, sizeof(_namePrefix));
  _namePrefix[sizeof(_namePrefix) - 1] = 0;

  _epollDescriptor = epoll_create(_maxSockets);
  if (0 > _epollDescriptor) {
    _epollDescriptor = INVALID_SOCKET;
    ESB_LOG_CRITICAL_ERRNO(LastError(), "[%s] cannot create epoll descriptor", name());
    return;
  }

  _events = (struct epoll_event *)_allocator.allocate(sizeof(struct epoll_event) * _maxSockets);

  if (!_events) {
    close(_epollDescriptor);
    _epollDescriptor = INVALID_SOCKET;
    ESB_LOG_CRITICAL_ERRNO(ESB_OUT_OF_MEMORY, "[%s] cannot create %d epoll_events", name(), _maxSockets);
    return;
  }

  _eventCache = (struct DescriptorState *)_allocator.allocate(sizeof(struct DescriptorState) * _maxSockets);

  if (!_eventCache) {
    close(_epollDescriptor);
    _epollDescriptor = INVALID_SOCKET;
    _allocator.deallocate(_events);
    _events = NULL;
    ESB_LOG_CRITICAL_ERRNO(ESB_OUT_OF_MEMORY, "[%s] cannot create %d DescriptorStates", name(), _maxSockets);
    return;
  }

  memset(_eventCache, 0, sizeof(struct DescriptorState) * _maxSockets);
}

EpollMultiplexer::~EpollMultiplexer() {
  if (INVALID_SOCKET != _epollDescriptor) {
    close(_epollDescriptor);
    _epollDescriptor = INVALID_SOCKET;
  }

  if (_events) {
    _allocator.deallocate(_events);
    _events = NULL;
  }

  if (_eventCache) {
    _allocator.deallocate(_eventCache);
    _eventCache = NULL;
  }
}

CleanupHandler *EpollMultiplexer::cleanupHandler() { return 0; }

Error EpollMultiplexer::addMultiplexedSocket(MultiplexedSocket *socket) {
  if (INVALID_SOCKET == _epollDescriptor) {
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
  event.events = EPOLLERR | EPOLLET;

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

  int currentSocketCount = _currentSocketCount.inc();

  if (currentSocketCount > _maxSockets) {
    ESB_LOG_ERROR("[%s] Cannot add socket: at limit of %d sockets", socket->name(), _maxSockets);
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
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot add socket", socket->name());
    _lock.writeAcquire();
    _currentSocketList.remove(socket);
    assert(_currentSocketList.validate());
    _lock.writeRelease();
    _currentSocketCount.dec();
    return error;
  }

  _eventCache[fd]._interests = event.events;

  ESB_LOG_DEBUG("[%s] added socket", socket->name());
  return ESB_SUCCESS;
}

Error EpollMultiplexer::updateMultiplexedSocket(MultiplexedSocket *socket) {
  if (INVALID_SOCKET == _epollDescriptor) {
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
  event.events = EPOLLERR;

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

  event.data.ptr = socket;

  if (_eventCache[fd]._interests != event.events) {
    if (0 != epoll_ctl(_epollDescriptor, EPOLL_CTL_MOD, fd, &event)) {
      Error error = LastError();
      ESB_LOG_ERROR_ERRNO(error, "[%s] cannot update socket", socket->name());
      return error;
    }
    _eventCache[fd]._interests = event.events;
    ESB_LOG_DEBUG("[%s] updated socket. wantRead=%s, wantWrite=%s", socket->name(),
                  socket->wantRead() ? "true" : "false", socket->wantWrite() ? "true" : "false");
  }

  return ESB_SUCCESS;
}

Error EpollMultiplexer::removeMultiplexedSocket(MultiplexedSocket *socket, bool removeFromList) {
  if (INVALID_SOCKET == _epollDescriptor) {
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
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot remove socket", socket->name());
    return error;
  }

  memset(&_eventCache[fd], 0, sizeof(struct DescriptorState));

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

  ESB_LOG_DEBUG("[%s] removed socket", socket->name());

  bool cleanup = socket->handleRemove();

  if (cleanup) {
    CleanupHandler *cleanupHandler = socket->cleanupHandler();
    if (cleanupHandler) {
      cleanupHandler->destroy(socket);
    }
  }

  return ESB_SUCCESS;
}

void EpollMultiplexer::destroy() {
  ESB_LOG_DEBUG("[%s] destroying", name());

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

  if (_eventCache) {
    _allocator.deallocate(_eventCache);
    _eventCache = NULL;
  }

  ESB_LOG_DEBUG("[%s] destroyed", name());
}

bool EpollMultiplexer::run(SharedInt *isRunning) {
  if (INVALID_SOCKET == _epollDescriptor) {
    return false;
  }

  if (!isRunning) {
    return false;
  }

  ESB_LOG_NOTICE("[%s] multiplexer thread started", name());

  int numEvents = 0;
  int i = 0;
  MultiplexedSocket *socket = 0;
  int errorCount = 0;
  SOCKET fd = INVALID_SOCKET;
  bool keepInMultiplexer = true;
  _isRunning = isRunning;

  while (_isRunning->get()) {
    checkIdleSockets(isRunning);
    numEvents = epoll_wait(_epollDescriptor, _events, _maxSockets, EPOLL_TIMEOUT_MILLIS);

    if (0 == numEvents) {
      // Timeout
      continue;
    }

    if (0 > numEvents) {
      ESB::Error error = LastError();
      if (ESB_INTR == error) {
        continue;
      }

      ESB_LOG_ERROR_ERRNO(error, "[%s] error in epoll_wait", name());

      if (errorCount >= 10) {
        ESB_LOG_CRITICAL("[%s] too many errors in epoll_wait, exiting", name());
        _allocator.deallocate(_events);
        _events = NULL;
        close(_epollDescriptor);
        _epollDescriptor = INVALID_SOCKET;
        return false;
      }

      ++errorCount;
      continue;
    }

    // Dump a description of all events that will be processed in this loop before taking any action.

    if (ESB_DEBUG_LOGGABLE) {
      ESB_LOG_DEBUG("[%s] processing %d events", name(), numEvents);

      for (i = 0; i < numEvents; ++i) {
        socket = (MultiplexedSocket *)_events[i].data.ptr;
        fd = socket->socketDescriptor();

        if (socket->wantAccept()) {
          if (_events[i].events & EPOLLERR) {
            ESB_LOG_DEBUG("[%s] listening socket error event", socket->name());
          } else if (_events[i].events & EPOLLIN) {
            ESB_LOG_DEBUG("[%s] listening socket accept event", socket->name());
          }
        } else if (socket->wantConnect()) {
          if (_events[i].events & EPOLLERR) {
            ESB_LOG_DEBUG("[%s] connecting socket error", socket->name());
          } else if (_events[i].events & EPOLLHUP) {
            ESB_LOG_DEBUG("[%s] connecting socket remote close", socket->name());
          } else if (_events[i].events & EPOLLIN) {
            ESB_LOG_DEBUG("[%s] connecting socket connected", socket->name());
          }
        } else {
          if (_events[i].events & EPOLLERR) {
            ESB_LOG_DEBUG("[%s] socket error", socket->name());
          } else if (_events[i].events & EPOLLHUP) {
            ESB_LOG_DEBUG("[%s] socket remote close", socket->name());
          } else if (_events[i].events & EPOLLIN) {
            ESB_LOG_DEBUG("[%s] socket read event", socket->name());
          } else if (_events[i].events & EPOLLOUT) {
            ESB_LOG_DEBUG("[%s] socket write event", socket->name());
          }
        }
      }
    }

    // Now take action

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
          ESB::Error error = TCPSocket::LastSocketError(fd);
          ESB_LOG_ERROR_ERRNO(error, "[%s] listening socket error", socket->name());
          socket->handleError(error);
          keepInMultiplexer = false;
        } else if (_events[i].events & EPOLLIN) {
          while (keepInMultiplexer) {
            ESB_LOG_DEBUG("[%s] listening socket accept event", socket->name());
            ESB::Error error = socket->handleAccept();
            if (ESB_AGAIN == error) {
              continue;
            } else if (ESB_SUCCESS == error) {
              break;
            } else {
              keepInMultiplexer = false;
              break;
            }
          }
        } else {
          ESB_LOG_WARNING("[%s] listening socket unknown event %d", socket->name(), _events[i].events);
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
          ESB::Error error = TCPSocket::LastSocketError(fd);
          ESB_LOG_INFO_ERRNO(error, "[%s] connecting socket error", socket->name());
          keepInMultiplexer = false;
          socket->handleError(error);
        } else if (_events[i].events & EPOLLHUP) {
          ESB_LOG_INFO("[%s] connected socket remote close", socket->name());
          keepInMultiplexer = false;
          socket->handleRemoteClose();
        } else if (_events[i].events & EPOLLIN) {
          int bytesReadable = ConnectedTCPSocket::BytesReadable(fd);

          if (0 > bytesReadable) {
            ESB::Error error = TCPSocket::LastSocketError(fd);
            ESB_LOG_INFO_ERRNO(error, "[%s] connecting socket error", socket->name());
            keepInMultiplexer = false;
            socket->handleError(error);
          } else if (0 == bytesReadable) {
            ESB_LOG_INFO("[%s] connecting socket remote close", socket->name());
            keepInMultiplexer = false;
            socket->handleRemoteClose();
          } else {
            ESB_LOG_INFO("[%s] socket connected", socket->name());
            ESB::Error error = socket->handleConnect();
            keepInMultiplexer = ESB_AGAIN == error;
          }
        } else if (_events[i].events & EPOLLOUT) {
          ESB_LOG_INFO("[%s] socket connected", socket->name());
          ESB::Error error = socket->handleConnect();
          keepInMultiplexer = ESB_AGAIN == error || ESB_PAUSE == error;
        } else {
          ESB_LOG_WARNING("[%s] connecting socket unknown event %d", socket->name(), _events[i].events);
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
          ESB::Error error = TCPSocket::LastSocketError(fd);
          ESB_LOG_INFO_ERRNO(error, "[%s] socket error", socket->name());
          keepInMultiplexer = false;
          socket->handleError(error);
        } else if (_events[i].events & EPOLLHUP) {
          ESB_LOG_INFO("[%s] socket remote close", socket->name());
          keepInMultiplexer = false;
          socket->handleRemoteClose();
        } else {
          // TODO consider swapping these - drain first, then fill.
          if (socket->wantRead() && (_events[i].events & EPOLLIN)) {
            ESB_LOG_DEBUG("[%s] socket read event", socket->name());
            ESB::Error error = socket->handleReadable();
            keepInMultiplexer = ESB_AGAIN == error || ESB_PAUSE == error;
          }

          if (keepInMultiplexer && socket->wantWrite() && (_events[i].events & EPOLLOUT)) {
            ESB_LOG_DEBUG("[%s] socket write event", socket->name());
            ESB::Error error = socket->handleWritable();
            keepInMultiplexer = ESB_AGAIN == error || ESB_PAUSE == error;
          }
        }
      }

      if (likely(keepInMultiplexer)) {
        updateMultiplexedSocket(socket);
      } else {
        removeMultiplexedSocket(socket);
      }
    }
  }

  ESB_LOG_NOTICE("[%s] stopped", name());
  destroy();
  return false;
}

int EpollMultiplexer::currentSockets() const { return _currentSocketCount.get(); }

int EpollMultiplexer::maximumSockets() const { return _maxSockets; }

bool EpollMultiplexer::isRunning() const { return _isRunning && _isRunning->get(); }

Error EpollMultiplexer::checkIdleSockets(SharedInt *isRunning) {
  if (_lastIdleCheckSec + IDLE_CHECK_SEC < time(0)) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] starting idle socket check", name());

  WriteScopeLock scopeLock(_lock);

  MultiplexedSocket *current = (MultiplexedSocket *)_currentSocketList.first();
  MultiplexedSocket *next = 0;

  while (current && isRunning->get()) {
    next = (MultiplexedSocket *)current->next();

    if (current->isIdle()) {
      ESB_LOG_DEBUG("[%s] socket is idle", current->name());
      removeMultiplexedSocket(current, false);
      _currentSocketList.remove(current);
      assert(_currentSocketList.validate());
      _currentSocketCount.dec();
    }

    current = next;
  }

  ESB_LOG_DEBUG("[%s] finished idle socket check", name());
  return ESB_SUCCESS;
}

const char *EpollMultiplexer::name() const { return _namePrefix; }

}  // namespace ESB
