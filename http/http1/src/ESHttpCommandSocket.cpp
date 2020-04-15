#ifndef ES_HTTP_COMMAND_SOCKET_H
#include <ESHttpCommandSocket.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ES {

HttpCommandSocket::HttpCommandSocket()
    : _eventSocket(), _lock(), _queue(), _removed(false) {}

HttpCommandSocket::~HttpCommandSocket() {}

bool HttpCommandSocket::wantAccept() { return false; }

bool HttpCommandSocket::wantConnect() { return false; }

bool HttpCommandSocket::wantRead() { return true; }

bool HttpCommandSocket::wantWrite() { return false; }

bool HttpCommandSocket::isIdle() { return false; }

bool HttpCommandSocket::handleAccept(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle accept");
  return true;  // keep in multiplexer
}

bool HttpCommandSocket::handleConnect(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle connect");
  return true;  // keep in multiplexer
}

// This code runs in any thread
ESB::Error HttpCommandSocket::pushInternal(ESB::EmbeddedListElement *command) {
  if (!command) {
    return ESB_NULL_POINTER;
  }

  {
    ESB::WriteScopeLock lock(_lock);
    if (_removed) {
      return ESB_SHUTDOWN;
    }
    _queue.addLast(command);
  }

  ESB::Error error = _eventSocket.write(1);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot update value of event socket");
    return error;
  }

  return ESB_SUCCESS;
}

// This code runs in the multiplexer's thread
bool HttpCommandSocket::handleReadable(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_DEBUG("Event socket had event");

  ESB::UInt64 value = 0;
  ESB::Error error = _eventSocket.read(&value);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot read value of event socket");
    return true;  // keep in multiplexer, try again
  }

  for (ESB::UInt64 i = 0; i < value; ++i) {
    ESB::EmbeddedListElement *command = NULL;
    {
      ESB::WriteScopeLock lock(_lock);
      command = _queue.removeFirst();
    }

    if (!command) {
      ESB_LOG_WARNING("Event socket and queue size are out of sync");
      return true;  // keep in multiplexer, try again
    }

    // TODO track latency
    runCommand(command);

    if (command->cleanupHandler()) {
      command->cleanupHandler()->destroy(command);
    }
  }

  return true;  // keep in multiplexer
}

bool HttpCommandSocket::handleWritable(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle writable");
  return true;  // keep in multiplexer
}

bool HttpCommandSocket::handleError(ESB::Error errorCode,
                                    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR_ERRNO(errorCode, "Event socket had error");
  return false;  // remove from multiplexer
}

bool HttpCommandSocket::handleRemoteClose(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle remote close");
  return true;  // keep in multiplexer
}

bool HttpCommandSocket::handleIdle(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle idle");
  return true;  // keep in multiplexer
}

bool HttpCommandSocket::handleRemove(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_NOTICE("Event socket removed from multiplexer");

  ESB::WriteScopeLock lock(_lock);
  _removed = true;

  while (true) {
    ESB::EmbeddedListElement *command = _queue.removeFirst();
    if (!command) {
      break;
    }
    if (command->cleanupHandler()) {
      command->cleanupHandler()->destroy(command);
    }
  }

  return false;  // do not call cleanup handler
}

SOCKET HttpCommandSocket::socketDescriptor() const {
  return _eventSocket.socketDescriptor();
}

ESB::CleanupHandler *HttpCommandSocket::cleanupHandler() { return NULL; }

const char *HttpCommandSocket::getName() const { return "CommandSocket"; }

}  // namespace ES