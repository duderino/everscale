#ifndef ES_HTTP_CLIENT_COMMAND_SOCKET_H
#include <ESHttpClientCommandSocket.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

namespace ES {

HttpClientCommandSocket::HttpClientCommandSocket(HttpClientStack &stack)
    : _eventSocket(), _lock(), _queue(), _stack(stack), _removed(false) {}

HttpClientCommandSocket::~HttpClientCommandSocket() {}

bool HttpClientCommandSocket::wantAccept() { return false; }

bool HttpClientCommandSocket::wantConnect() { return false; }

bool HttpClientCommandSocket::wantRead() { return true; }

bool HttpClientCommandSocket::wantWrite() { return false; }

bool HttpClientCommandSocket::isIdle() { return false; }

bool HttpClientCommandSocket::handleAccept(
    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle accept");
  return true;  // keep in multiplexer
}

bool HttpClientCommandSocket::handleConnect(
    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle connect");
  return true;  // keep in multiplexer
}

// This code runs in any thread
ESB::Error HttpClientCommandSocket::push(HttpClientCommand *command) {
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
bool HttpClientCommandSocket::handleReadable(
    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_DEBUG("Event socket had event");

  ESB::UInt64 value = 0;
  ESB::Error error = _eventSocket.read(&value);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot read value of event socket");
    return true;  // keep in multiplexer, try again
  }

  for (ESB::UInt64 i = 0; i < value; ++i) {
    HttpClientCommand *command = NULL;
    {
      ESB::WriteScopeLock lock(_lock);
      command = (HttpClientCommand *)_queue.removeFirst();
    }

    if (!command) {
      ESB_LOG_WARNING("Event socket and queue size are out of sync");
      return true;  // keep in multiplexer, try again
    }

    // TODO track latency
    ESB_LOG_DEBUG("Executing command '%s' in multiplexer",
                  ESB_SAFE_STR(command->name()));
    error = command->run(_stack);

    if (ESB_SUCCESS != error) {
      ESB_LOG_WARNING_ERRNO(error,
                            "Failed executing command '%s in multiplexer",
                            ESB_SAFE_STR(command->name()));
    }

    if (command->cleanupHandler()) {
      command->cleanupHandler()->destroy(command);
    }
  }

  return true;  // keep in multiplexer
}

bool HttpClientCommandSocket::handleWritable(
    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle writable");
  return true;  // keep in multiplexer
}

bool HttpClientCommandSocket::handleError(ESB::Error errorCode,
                                          ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR_ERRNO(errorCode, "Event socket had error");
  return false;  // remove from multiplexer
}

bool HttpClientCommandSocket::handleRemoteClose(
    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle remote close");
  return true;  // keep in multiplexer
}

bool HttpClientCommandSocket::handleIdle(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("Event sockets cannot handle idle");
  return true;  // keep in multiplexer
}

bool HttpClientCommandSocket::handleRemove(
    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_NOTICE("Event socket removed from multiplexer");

  ESB::WriteScopeLock lock(_lock);
  _removed = true;

  while (true) {
    HttpClientCommand *command = (HttpClientCommand *)_queue.removeFirst();
    if (!command) {
      break;
    }
    if (command->cleanupHandler()) {
      command->cleanupHandler()->destroy(command);
    }
  }

  return false;  // do not call cleanup handler
}

SOCKET HttpClientCommandSocket::socketDescriptor() const {
  return _eventSocket.socketDescriptor();
}

ESB::CleanupHandler *HttpClientCommandSocket::cleanupHandler() { return NULL; }

const char *HttpClientCommandSocket::getName() const {
  return "ClientCommandSocket";
}

}  // namespace ES
