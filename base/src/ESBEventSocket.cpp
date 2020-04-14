#ifndef ESB_EVENT_SOCKET_H
#include <ESBEventSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

#ifdef HAVE_SYS_EVENTFD_H
#include <sys/eventfd.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace ESB {
EventSocket::EventSocket(SharedInt &isRunning)
    : _lock(), _queue(), _isRunning(isRunning) {
#ifdef HAVE_EVENTFD
  _eventFd = eventfd(0, EFD_NONBLOCK);
#else
  // to implement on plaforms without event fd, use a socket or pipe
#error "eventfd() or equivalent is required"
#endif

  if (0 > _eventFd) {
    ESB_LOG_ERROR_ERRNO(ConvertError(_eventFd), "Cannot create event fd");
  }
}

EventSocket::~EventSocket() {
#ifdef HAVE_CLOSE
  close(_eventFd);
  _eventFd = INVALID_SOCKET;
#else
#error "close() or equivalent is required"
#endif

  WriteScopeLock lock(_lock);
  for (EmbeddedListElement *e = _queue.first(); e; e = e->next()) {
    if (e->cleanupHandler()) {
      e->cleanupHandler()->destroy(e);
    }
  }
}

// This runs in any thread
Error EventSocket::push(Command *command) {
  _lock.writeAcquire();
  _queue.addLast(command);
  _lock.writeRelease();

#ifdef HAVE_WRITE
  ESB::UInt64 value = 1;
  ESB::Error error = ConvertError(write(_eventFd, &value, sizeof(value)));
#else
#error "write() or equivalent is required"
#endif

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot write to event fd");
  }

  return error;
}

bool EventSocket::wantAccept() { return false; }

bool EventSocket::wantConnect() { return false; }

bool EventSocket::wantRead() { return true; }

bool EventSocket::wantWrite() { return false; }

bool EventSocket::isIdle() { return false; }

bool EventSocket::handleAccept(SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle accept");
  return true;  // keep in multiplexer
}

bool EventSocket::handleConnect(SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle connect");
  return true;  // keep in multiplexer
}

// This runs in the multiplexer thread
bool EventSocket::handleReadable(SocketMultiplexer &multiplexer) {
  ESB_LOG_NOTICE("Event socket had event");

  ESB::UInt64 value = 0;

#ifdef HAVE_READ
  Error error = ConvertError(read(_eventFd, &value, sizeof(value)));
#else
#error "read() or equivalent is required"
#endif

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("Event fd had spurious wakeup");
    return true;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot write to event fd");
    return true;  // keep in multiplexer
  }

  for (ESB::UInt64 i = 0; i < value; ++i) {
    _lock.writeAcquire();
    Command *command = (Command *)_queue.removeFirst();
    _lock.writeRelease();

    if (!command) {
      ESB_LOG_WARNING("Event fd counter and queue size are out of sync");
      return true;  // keep in multiplexer
    }

    ESB_LOG_DEBUG("Executing command '%s' in multiplexer", command->name());
    // TODO track latency
    if (command->run(&_isRunning) && command->cleanupHandler()) {
      command->cleanupHandler()->destroy(command);
    }
  }

  return true;  // keep in multiplexer
}

bool EventSocket::handleWritable(SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle writable");
  return true;  // keep in multiplexer
}

bool EventSocket::handleError(Error errorCode, SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR_ERRNO(errorCode, "Event socket had error");
  return false;  // remove from multiplexer
}

bool EventSocket::handleRemoteClose(SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle remote close");
  return true;  // keep in multiplexer
}

bool EventSocket::handleIdle(SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle idle");
  return true;  // keep in multiplexer
}

bool EventSocket::handleRemove(SocketMultiplexer &multiplexer) {
  ESB_LOG_NOTICE("Event socket removed from multiplexer");
  return false;  // do not call cleanup handler
}

SOCKET EventSocket::socketDescriptor() const { return _eventFd; }

CleanupHandler *EventSocket::cleanupHandler() { return NULL; }

}  // namespace ESB
