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

HttpCommandSocket::HttpCommandSocket() : _eventSocket(), _lock(), _queue(), _removed(false) {}

HttpCommandSocket::~HttpCommandSocket() {}

bool HttpCommandSocket::wantAccept() { return false; }

bool HttpCommandSocket::wantConnect() { return false; }

bool HttpCommandSocket::wantRead() { return true; }

bool HttpCommandSocket::wantWrite() { return false; }

bool HttpCommandSocket::isIdle() { return false; }

ESB::Error HttpCommandSocket::handleAccept() {
  ESB_LOG_ERROR("Event sockets cannot handle accept");
  return ESB_INVALID_STATE;  // remove from multiplexer
}

ESB::Error HttpCommandSocket::handleConnect() {
  ESB_LOG_ERROR("Event sockets cannot handle connect");
  return ESB_INVALID_STATE;  // remove from multiplexer
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
    ESB_LOG_ERROR_ERRNO(error, "[%d] cannot update command socket", _eventSocket.socketDescriptor());
    return error;
  }

  return ESB_SUCCESS;
}

// This code runs in the multiplexer's thread
ESB::Error HttpCommandSocket::handleReadable() {
  ESB_LOG_DEBUG("[%d] command socket event", _eventSocket.socketDescriptor());

  ESB::UInt64 value = 0;
  ESB::Error error = _eventSocket.read(&value);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[%d] cannot read command socket", _eventSocket.socketDescriptor());
    return ESB_SUCCESS;  // keep in multiplexer, try again
  }

  for (ESB::UInt64 i = 0; i < value; ++i) {
    ESB::EmbeddedListElement *command = NULL;
    {
      ESB::WriteScopeLock lock(_lock);
      command = _queue.removeFirst();
    }

    if (!command) {
      ESB_LOG_WARNING("[%d] command socket and queue are out of sync", _eventSocket.socketDescriptor());
      return ESB_SUCCESS;  // keep in multiplexer, try again
    }

    // TODO track latency
    runCommand(command);

    if (command->cleanupHandler()) {
      command->cleanupHandler()->destroy(command);
    }
  }

  return ESB_SUCCESS;  // keep in multiplexer
}

ESB::Error HttpCommandSocket::handleWritable() {
  ESB_LOG_ERROR("[%d] command sockets cannot handle writable", _eventSocket.socketDescriptor());
  return ESB_INVALID_STATE;  // remove from multiplexer
}

bool HttpCommandSocket::handleError(ESB::Error errorCode) {
  ESB_LOG_ERROR_ERRNO(errorCode, "[%d] command socket had error", _eventSocket.socketDescriptor());
  return false;  // remove from multiplexer
}

bool HttpCommandSocket::handleRemoteClose() {
  ESB_LOG_ERROR("[%d] command sockets cannot handle remote close", _eventSocket.socketDescriptor());
  return true;  // keep in multiplexer
}

bool HttpCommandSocket::handleIdle() {
  ESB_LOG_ERROR("[%d] command sockets cannot handle idle", _eventSocket.socketDescriptor());
  return true;  // keep in multiplexer
}

bool HttpCommandSocket::handleRemove() {
  ESB_LOG_NOTICE("[%d] command socket removed from multiplexer", _eventSocket.socketDescriptor());

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

SOCKET HttpCommandSocket::socketDescriptor() const { return _eventSocket.socketDescriptor(); }

ESB::CleanupHandler *HttpCommandSocket::cleanupHandler() { return NULL; }

}  // namespace ES
