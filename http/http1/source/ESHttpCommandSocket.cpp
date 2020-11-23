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

HttpCommandSocket::HttpCommandSocket(const char *prefix) : _eventSocket(), _lock(), _queue(), _removed(false) {
  snprintf(_name, sizeof(_name), "%s%s", prefix, ESB_COMMAND_SUFFIX);
  _name[sizeof(_name) - 1] = 0;
}

HttpCommandSocket::~HttpCommandSocket() {}

bool HttpCommandSocket::wantAccept() { return false; }

bool HttpCommandSocket::wantConnect() { return false; }

bool HttpCommandSocket::wantRead() { return true; }

bool HttpCommandSocket::wantWrite() { return false; }

bool HttpCommandSocket::isIdle() { return false; }

ESB::Error HttpCommandSocket::handleAccept() {
  ESB_LOG_ERROR("[%s] command sockets cannot handle accept", _name);
  return ESB_INVALID_STATE;  // remove from multiplexer
}

ESB::Error HttpCommandSocket::handleConnect() {
  ESB_LOG_ERROR("[%s] command sockets cannot handle connect", _name);
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
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot update command socket", _name);
    return error;
  }

  return ESB_SUCCESS;
}

// This code runs in the multiplexer's thread
ESB::Error HttpCommandSocket::handleReadable() {
  ESB::UInt64 value = 0;
  ESB::Error error = _eventSocket.read(&value);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot read command socket", _name);
    return ESB_AGAIN;  // keep in multiplexer, try again
  }

  for (ESB::UInt64 i = 0; i < value; ++i) {
    ESB::EmbeddedListElement *command = NULL;
    {
      ESB::WriteScopeLock lock(_lock);
      command = _queue.removeFirst();
    }

    if (!command) {
      ESB_LOG_WARNING("[%s] command socket and queue are out of sync", _name);
      return ESB_AGAIN;  // keep in multiplexer, try again
    }

    // TODO track latency
    runCommand(command);

    if (command->cleanupHandler()) {
      command->cleanupHandler()->destroy(command);
    }
  }

  return ESB_AGAIN;  // keep in multiplexer
}

ESB::Error HttpCommandSocket::handleWritable() {
  ESB_LOG_ERROR("[%s] command sockets cannot handle writable", _name);
  return ESB_INVALID_STATE;  // remove from multiplexer
}

void HttpCommandSocket::handleError(ESB::Error errorCode) {
  ESB_LOG_ERROR_ERRNO(errorCode, "[%s] command socket had error: %d", _name, errorCode);
}

void HttpCommandSocket::handleRemoteClose() { ESB_LOG_ERROR("[%s] command sockets cannot handle remote close", _name); }

void HttpCommandSocket::handleIdle() { ESB_LOG_ERROR("[%s] command sockets cannot handle idle event", _name); }

bool HttpCommandSocket::handleRemove() {
  ESB_LOG_NOTICE("[%s] command socket removed from multiplexer", _name);

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

const char *HttpCommandSocket::name() const { return _name; }

const void *HttpCommandSocket::key() const { return _name; }

}  // namespace ES
