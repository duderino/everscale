#ifndef ES_HTTP_CLIENT_SOCKET_H
#include <ESHttpClientSocket.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ES_HTTP_ERROR_H
#include <ESHttpError.h>
#endif

#ifndef ES_HTTP_ALLOCATOR_SIZE
#define ES_HTTP_ALLOCATOR_SIZE 3072
#endif

// TODO add performance counters

#define HAS_BEEN_REMOVED (1 << 0)
#define CONNECTING (1 << 1)
#define TRANSACTION_BEGIN (1 << 2)
#define FORMATTING_HEADERS (1 << 3)
#define FLUSHING_HEADERS (1 << 4)
#define FORMATTING_BODY (1 << 5)
#define FLUSHING_BODY (1 << 6)
#define PARSING_HEADERS (1 << 7)
#define PARSING_BODY (1 << 8)
#define TRANSACTION_END (1 << 9)
#define RETRY_STALE_CONNECTION (1 << 10)
#define FIRST_USE_AFTER_REUSE (1 << 11)

// TODO - max requests per connection option (1 disables keepalives)
// TODO - max header size option
// TODO - max body size option

namespace ES {

static bool YieldAfterSendingRequest = false;
static bool YieldAfterFormattingHeaders = false;
static bool YieldAfterFormattingChunk = false;
// static bool YieldAfterParsingChunk = false;
static bool FlushRequestHeaders = false;

bool HttpClientSocket::_ReuseConnections = true;

HttpClientSocket::HttpClientSocket(HttpConnectionPool *pool,
                                   HttpClientTransaction *transaction,
                                   HttpClientCounters *counters,
                                   ESB::CleanupHandler *cleanupHandler,
                                   ESB::Logger *logger)
    : _state(CONNECTING),
      _bodyBytesWritten(0),
      _pool(pool),
      _transaction(transaction),
      _counters(counters),
      _logger(logger ? logger : ESB::NullLogger::GetInstance()),
      _cleanupHandler(cleanupHandler),
      _socket(*transaction->getPeerAddress(), false) {}

HttpClientSocket::~HttpClientSocket() {}

ESB::Error HttpClientSocket::reset(bool reused, HttpConnectionPool *pool,
                                   HttpClientTransaction *transaction) {
  if (!transaction) {
    return ESB_NULL_POINTER;
  }

  _state = TRANSACTION_BEGIN;

  if (reused) {
    assert(true == isConnected());

    _state |= FIRST_USE_AFTER_REUSE;
  } else {
    assert(false == isConnected());
  }

  _bodyBytesWritten = 0;
  _pool = pool;
  _transaction = transaction;

  return ESB_SUCCESS;
}

bool HttpClientSocket::wantAccept() { return false; }

bool HttpClientSocket::wantConnect() { return _state & CONNECTING; }

bool HttpClientSocket::wantRead() {
  return _state & (PARSING_HEADERS | PARSING_BODY) ? true : false;
}

bool HttpClientSocket::wantWrite() {
  return _state & (TRANSACTION_BEGIN | FORMATTING_HEADERS | FLUSHING_HEADERS |
                   FORMATTING_BODY | FLUSHING_BODY)
             ? true
             : false;
}

bool HttpClientSocket::isIdle() {
  // TODO - implement idle timeout

  return false;
}

bool HttpClientSocket::handleAcceptEvent(ESB::SharedInt *isRunning,
                                         ESB::Logger *logger) {
  assert(!(HAS_BEEN_REMOVED & _state));

  if (_logger->isLoggable(ESB::Logger::Warning)) {
    _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Cannot handle accept events",
                 _socket.getSocketDescriptor());
  }

  return true;  // keep in multiplexer
}

bool HttpClientSocket::handleConnectEvent(ESB::SharedInt *isRunning,
                                          ESB::Logger *logger) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(_socket.isConnected());
  assert(_state & CONNECTING);

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] Connected", _socket.getSocketDescriptor());
  }

  _state &= ~CONNECTING;
  _state |= TRANSACTION_BEGIN;

  return handleWritableEvent(isRunning, logger);
}

bool HttpClientSocket::handleReadableEvent(ESB::SharedInt *isRunning,
                                           ESB::Logger *logger) {
  // returning true will keep the socket in the multiplexer
  // returning false will remove the socket from the multiplexer and ultimately
  // close it.

  assert(wantRead());
  assert(_socket.isConnected());
  assert(!(HAS_BEEN_REMOVED & _state));

  ESB::SSize result = 0;
  ESB::Error error = ESB_SUCCESS;

  while (isRunning->get()) {
    if (false == _transaction->getIOBuffer()->isWritable()) {
      if (_logger->isLoggable(ESB::Logger::Debug)) {
        _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                     "[socket:%d] compacting input buffer for",
                     _socket.getSocketDescriptor());
      }

      if (false == _transaction->getIOBuffer()->compact()) {
        if (_logger->isLoggable(ESB::Logger::Warning)) {
          _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                       "[socket:%d] parser jammed",
                       _socket.getSocketDescriptor());
        }

        return false;  // remove from multiplexer
      }
    }

    assert(_transaction->getIOBuffer()->isWritable());

    result = _socket.receive(_transaction->getIOBuffer());

    if (false == isRunning->get()) {
      break;
    }

    if (0 > result) {
      error = ESB::GetLastError();

      if (ESB_AGAIN == error) {
        if (_logger->isLoggable(ESB::Logger::Debug)) {
          _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                       "[socket:%d] not ready for read",
                       _socket.getSocketDescriptor());
        }

        return true;  // keep in multiplexer
      }

      if (ESB_INTR == error) {
        if (_logger->isLoggable(ESB::Logger::Debug)) {
          _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                       "[socket:%d] interrupted",
                       _socket.getSocketDescriptor());
        }

        continue;  // try _socket.receive again
      }

      return handleErrorEvent(error, isRunning, logger);
    }

    if (0 == result) {
      return handleEndOfFileEvent(isRunning, logger);
    }

    if (_logger->isLoggable(ESB::Logger::Debug)) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[socket:%d] Read %d bytes", _socket.getSocketDescriptor(),
                   result);
    }

    if (PARSING_HEADERS & _state) {
      error = parseResponseHeaders(isRunning, logger);

      if (ESB_AGAIN == error) {
        continue;  // read more data and repeat parse
      }

      if (ESB_SUCCESS != error) {
        return false;  // remove from multiplexer
      }

      if (HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CLOSE ==
          _transaction->getHandler()->receiveResponseHeaders(_transaction)) {
        if (_logger->isLoggable(ESB::Logger::Debug)) {
          _logger->log(
              ESB::Logger::Debug, __FILE__, __LINE__,
              "[socket:%d] Client request header handler aborting connection",
              _socket.getSocketDescriptor());
        }

        return false;  // remove from multiplexer
      }

      if (false == _transaction->getResponse()->hasBody()) {
        unsigned char byte = 0;

        if (HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CLOSE ==
            _transaction->getHandler()->receiveResponseBody(_transaction, &byte,
                                                            0)) {
          if (_logger->isLoggable(ESB::Logger::Debug)) {
            _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                         "[socket:%d] Client body handler aborting connection",
                         _socket.getSocketDescriptor());
          }

          return false;  // remove from multiplexer
        }

        _state &= ~PARSING_HEADERS;
        _state |= TRANSACTION_END;

        return false;  // remove from multiplexer
      }

      _state &= ~PARSING_HEADERS;
      _state |= PARSING_BODY;
    }

    assert(PARSING_BODY & _state);
    assert(_transaction->getResponse()->hasBody());

    error = parseResponseBody(isRunning, logger);

    if (ESB_AGAIN == error) {
      continue;  // read more data and repeat parse
    }

    return false;  // remove from multiplexer
  }

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(
        ESB::Logger::Debug, __FILE__, __LINE__,
        "[socket:%d] multiplexer shutdown, killing socket in parse state",
        _socket.getSocketDescriptor());
  }

  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleWritableEvent(ESB::SharedInt *isRunning,
                                           ESB::Logger *logger) {
  assert(wantWrite());
  assert(_socket.isConnected());
  assert(!(HAS_BEEN_REMOVED & _state));

  ESB::Error error = ESB_SUCCESS;

  if (_state & TRANSACTION_BEGIN) {
    // TODO make connection reuse more configurable
    if (false == HttpClientSocket::GetReuseConnections() &&
        0 == _transaction->getRequest()->getHeader(
                 (const unsigned char *)"Connection")) {
      error = _transaction->getRequest()->addHeader(
          (const unsigned char *)"Connection", (const unsigned char *)"close",
          _transaction->getAllocator());

      if (ESB_SUCCESS != error) {
        if (_logger->isLoggable(ESB::Logger::Err)) {
          char buffer[100];
          ESB::DescribeError(error, buffer, sizeof(buffer));
          _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                       "[socket:%d] cannot add Connection: close header: %s",
                       _socket.getSocketDescriptor(), buffer);
        }

        return false;  // remove from multiplexer
      }
    }

    // TODO add user agent, etc headers

    _state &= ~TRANSACTION_BEGIN;
    _state |= FORMATTING_HEADERS;
  }

  while (isRunning->get()) {
    if (FORMATTING_HEADERS & _state) {
      error = formatRequestHeaders(isRunning, logger);

      if (ESB_AGAIN == error) {
        error = flushBuffer(isRunning, logger);

        if (ESB_AGAIN == error) {
          return true;  // keep in multiplexer, wait for socket to become
                        // writable
        }

        if (ESB_SUCCESS != error) {
          return false;  // remove from multiplexer
        }

        continue;
      }

      if (ESB_SUCCESS != error) {
        return false;  // remove from multiplexer;
      }

      if (YieldAfterFormattingHeaders && false == FlushRequestHeaders) {
        return true;  // keep in multiplexer, but yield
      }
    }

    if (FLUSHING_HEADERS & _state) {
      error = flushBuffer(isRunning, logger);

      if (ESB_AGAIN == error) {
        return true;  // keep in multiplexer, wait for socket to become writable
      }

      if (ESB_SUCCESS != error) {
        return false;  // remove from multiplexer
      }

      _state &= ~FLUSHING_HEADERS;

      if (_transaction->getRequest()->hasBody()) {
        _state |= FORMATTING_BODY;

        if (YieldAfterFormattingHeaders) {
          return true;  // keep in multiplexer; but move on to the next
                        // connection
        }
      } else {
        _state |= PARSING_HEADERS;
      }
    }

    if (FORMATTING_BODY & _state) {
      error = formatRequestBody(isRunning, logger);

      if (ESB_AGAIN == error) {
        error = flushBuffer(isRunning, logger);

        if (ESB_AGAIN == error) {
          return true;  // keep in multiplexer, wait for socket to become
                        // writable
        }

        if (ESB_SUCCESS != error) {
          return false;  // remove from multiplexer
        }

        continue;
      }

      if (ESB_SUCCESS != error) {
        return false;  // remove from multiplexer;
      }

      if (FORMATTING_BODY & _state) {
        if (YieldAfterFormattingChunk) {
          return true;  // keep in multiplexer
        } else {
          continue;
        }
      }
    }

    assert(FLUSHING_BODY & _state);

    error = flushBuffer(isRunning, logger);

    if (ESB_AGAIN == error) {
      return true;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      return false;  // remove from multiplexer
    }

    _state &= ~FLUSHING_BODY;
    _state |= PARSING_HEADERS;

    return YieldAfterSendingRequest ? true
                                    : handleReadableEvent(isRunning, logger);
  }

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(
        ESB::Logger::Debug, __FILE__, __LINE__,
        "[socket:%d] multiplexer shutdown, killing socket in format state",
        _socket.getSocketDescriptor());
  }

  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleErrorEvent(ESB::Error errorCode,
                                        ESB::SharedInt *isRunning,
                                        ESB::Logger *logger) {
  assert(!(HAS_BEEN_REMOVED & _state));

  if (_logger->isLoggable(ESB::Logger::Info)) {
    char buffer[100];
    char dottedAddress[16];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB::DescribeError(errorCode, buffer, sizeof(buffer));
    _logger->log(ESB::Logger::Info, __FILE__, __LINE__,
                 "[socket:%d] Error from client %s: %s",
                 _socket.getSocketDescriptor(), dottedAddress, buffer);
  }

  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleEndOfFileEvent(ESB::SharedInt *isRunning,
                                            ESB::Logger *logger) {
  // TODO - this may just mean the client closed its half of the socket but is
  // still expecting a response.

  assert(!(HAS_BEEN_REMOVED & _state));

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    char dottedAddress[16];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] Client %s closed socket",
                 _socket.getSocketDescriptor(), dottedAddress);
  }

  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleIdleEvent(ESB::SharedInt *isRunning,
                                       ESB::Logger *logger) {
  assert(!(HAS_BEEN_REMOVED & _state));

  if (_logger->isLoggable(ESB::Logger::Info)) {
    char dottedAddress[16];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    _logger->log(ESB::Logger::Info, __FILE__, __LINE__,
                 "[socket:%d] Client %s is idle", _socket.getSocketDescriptor(),
                 dottedAddress);
  }

  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleRemoveEvent(ESB::SharedInt *isRunning,
                                         ESB::Logger *logger) {
  assert(!(HAS_BEEN_REMOVED & _state));

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    char dottedAddress[16];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] Closing socket for client %s",
                 _socket.getSocketDescriptor(), dottedAddress);
  }

  bool reuseConnection = false;

  if (_state & TRANSACTION_BEGIN) {
    _counters->getFailures()->addObservation(_transaction->getStartTime(),
                                             ESB::Date::Now());

    _transaction->getHandler()->end(
        _transaction, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_BEGIN);
  } else if (_state & CONNECTING) {
    _counters->getFailures()->addObservation(_transaction->getStartTime(),
                                             ESB::Date::Now());

    _transaction->getHandler()->end(
        _transaction, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
  } else if (_state & (FORMATTING_HEADERS | FLUSHING_HEADERS)) {
    _counters->getFailures()->addObservation(_transaction->getStartTime(),
                                             ESB::Date::Now());

    _transaction->getHandler()->end(
        _transaction,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS);
  } else if (_state & (FORMATTING_BODY | FLUSHING_BODY)) {
    _counters->getFailures()->addObservation(_transaction->getStartTime(),
                                             ESB::Date::Now());

    _transaction->getHandler()->end(
        _transaction,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY);
  } else if (_state & PARSING_HEADERS) {
    _counters->getFailures()->addObservation(_transaction->getStartTime(),
                                             ESB::Date::Now());

    _transaction->getHandler()->end(
        _transaction,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS);
  } else if (_state & PARSING_BODY) {
    _counters->getFailures()->addObservation(_transaction->getStartTime(),
                                             ESB::Date::Now());

    _transaction->getHandler()->end(
        _transaction,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY);
  } else if (_state & TRANSACTION_END) {
    _counters->getSuccesses()->addObservation(_transaction->getStartTime(),
                                              ESB::Date::Now());

    if (GetReuseConnections()) {
      const HttpHeader *header = _transaction->getResponse()->getHeader(
          (unsigned const char *)"Connection");

      if (header && header->getFieldValue() &&
          0 == strcasecmp("close", (const char *)header->getFieldValue())) {
        reuseConnection = false;
      } else {
        reuseConnection = true;
      }
    } else {
      reuseConnection = false;
    }

    _transaction->getHandler()->end(
        _transaction, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_END);
  } else if (_state & RETRY_STALE_CONNECTION) {
    if (_logger->isLoggable(ESB::Logger::Debug)) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[socket:%d] connection stale, retrying transaction",
                   _socket.getSocketDescriptor());
    }

    ESB::Error error = _pool->executeClientTransaction(_transaction);

    if (ESB_SUCCESS != error) {
      if (_logger->isLoggable(ESB::Logger::Warning)) {
        char buffer[100];
        ESB::DescribeError(error, buffer, sizeof(buffer));
        _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                     "[socket:%d] Cannot retry transaction: %s",
                     _socket.getSocketDescriptor(), buffer);
      }

      _state &= ~RETRY_STALE_CONNECTION;
      _state |= TRANSACTION_BEGIN;

      _transaction->getHandler()->end(
          _transaction, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_BEGIN);
    }
  }

  if (0x00 == (_state & RETRY_STALE_CONNECTION)) {
    _pool->destroyClientTransaction(_transaction);
  }

  if (false == reuseConnection) {
    _socket.close();
  }

  _transaction = 0;
  _pool = 0;
  _state = HAS_BEEN_REMOVED;

  return true;  // call cleanup handler on us after this returns
}

SOCKET HttpClientSocket::getSocketDescriptor() const {
  return _socket.getSocketDescriptor();
}

ESB::CleanupHandler *HttpClientSocket::getCleanupHandler() {
  return _cleanupHandler;
}

const char *HttpClientSocket::getName() const { return "HttpClientSocket"; }

bool HttpClientSocket::run(ESB::SharedInt *isRunning) {
  return false;  // todo - log warning
}

ESB::Error HttpClientSocket::parseResponseHeaders(ESB::SharedInt *isRunning,
                                                  ESB::Logger *logger) {
  ESB::Error error = _transaction->getParser()->parseHeaders(
      _transaction->getIOBuffer(), _transaction->getResponse());

  if (ESB_AGAIN == error) {
    if (_logger->isLoggable(ESB::Logger::Debug)) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[socket:%d] need more header data from stream",
                   _socket.getSocketDescriptor());
    }

    if (false == _transaction->getIOBuffer()->compact()) {
      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                     "[socket:%d] cannot parse headers: parser jammed",
                     _socket.getSocketDescriptor());
      }

      return ESB_OVERFLOW;  // remove from multiplexer
    }

    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Warning)) {
      // TODO describe error, not just error code

      _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                   "[socket:%d] cannot parse headers: %d",
                   _socket.getSocketDescriptor(), error);
    }

    return error;  // remove from multiplexer
  }

  // parse complete

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] headers parsed", _socket.getSocketDescriptor());

    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] StatusCode: %d", _socket.getSocketDescriptor(),
                 _transaction->getResponse()->getStatusCode());

    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] ReasonPhrase: %s", _socket.getSocketDescriptor(),
                 _transaction->getResponse()->getReasonPhrase());

    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] Version: HTTP/%d.%d\n",
                 _socket.getSocketDescriptor(),
                 _transaction->getResponse()->getHttpVersion() / 100,
                 _transaction->getResponse()->getHttpVersion() % 100 / 10);

    for (HttpHeader *header = (HttpHeader *)_transaction->getResponse()
                                  ->getHeaders()
                                  ->getFirst();
         header; header = (HttpHeader *)header->getNext()) {
      _logger->log(
          ESB::Logger::Debug, __FILE__, __LINE__, "[socket:%d] %s: %s\n",
          _socket.getSocketDescriptor(), (const char *)header->getFieldName(),
          0 == header->getFieldValue() ? "null"
                                       : (const char *)header->getFieldValue());
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::parseResponseBody(ESB::SharedInt *isRunning,
                                               ESB::Logger *logger) {
  int startingPosition = 0;
  int chunkSize = 0;
  ESB::Error error = ESB_SUCCESS;

  while (isRunning->get()) {
    error = _transaction->getParser()->parseBody(_transaction->getIOBuffer(),
                                                 &startingPosition, &chunkSize);

    if (ESB_AGAIN == error) {
      if (_logger->isLoggable(ESB::Logger::Debug)) {
        _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                     "[socket:%d] need more body data from stream",
                     _socket.getSocketDescriptor());
      }

      if (false == _transaction->getIOBuffer()->isWritable()) {
        if (_logger->isLoggable(ESB::Logger::Debug)) {
          _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                       "[socket:%d] compacting input buffer",
                       _socket.getSocketDescriptor());
        }

        if (false == _transaction->getIOBuffer()->compact()) {
          if (_logger->isLoggable(ESB::Logger::Warning)) {
            _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                         "[socket:%d] cannot parse body: parser jammed",
                         _socket.getSocketDescriptor());
          }

          return ESB_OVERFLOW;  // remove from multiplexer
        }
      }

      return ESB_AGAIN;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      if (_logger->isLoggable(ESB::Logger::Debug)) {
        _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                     "[socket:%d] error parsing body: %d",
                     _socket.getSocketDescriptor(), error);
      }

      return error;  // remove from multiplexer
    }

    if (0 == chunkSize) {
      if (_logger->isLoggable(ESB::Logger::Debug)) {
        _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                     "[socket:%d] parsed body", _socket.getSocketDescriptor());
      }

      unsigned char byte = 0;

      HttpClientHandler::Result result =
          _transaction->getHandler()->receiveResponseBody(_transaction, &byte,
                                                          0);

      if (HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CLOSE == result) {
        if (_logger->isLoggable(ESB::Logger::Debug)) {
          _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                       "[socket:%d] client handler aborting connection after "
                       "last body chunk",
                       _socket.getSocketDescriptor());
        }

        return ESB_INTR;
      }

      _state &= ~PARSING_BODY;
      _state |= TRANSACTION_END;

      return ESB_SUCCESS;
    }

    if (_logger->isLoggable(ESB::Logger::Debug)) {
      char buffer[4096];

      memcpy(buffer,
             _transaction->getIOBuffer()->getBuffer() + startingPosition,
             chunkSize > (int)sizeof(buffer) ? sizeof(buffer) : chunkSize);
      buffer[chunkSize > (int)sizeof(buffer) ? sizeof(buffer) : chunkSize] = 0;

      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[socket:%d] read chunk: %s", _socket.getSocketDescriptor(),
                   buffer);
    }

    HttpClientHandler::Result result;

    result = _transaction->getHandler()->receiveResponseBody(
        _transaction,
        _transaction->getIOBuffer()->getBuffer() + startingPosition, chunkSize);

    if (HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CLOSE == result) {
      if (_logger->isLoggable(ESB::Logger::Debug)) {
        _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                     "[socket:%d] client handler aborting connection before "
                     "last body chunk",
                     _socket.getSocketDescriptor());
      }

      return ESB_INTR;
    }
  }

  return ESB_SHUTDOWN;
}

ESB::Error HttpClientSocket::formatRequestHeaders(ESB::SharedInt *isRunning,
                                                  ESB::Logger *logger) {
  ESB::Error error = _transaction->getFormatter()->formatHeaders(
      _transaction->getIOBuffer(), _transaction->getRequest());

  if (ESB_AGAIN == error) {
    if (_logger->isLoggable(ESB::Logger::Debug)) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[socket:%d] partially formatted response headers",
                   _socket.getSocketDescriptor());
    }

    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Warning)) {
      char buffer[50];
      ESB::DescribeError(error, buffer, sizeof(buffer));
      _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                   "[socket:%d] error formatting response header: %s",
                   _socket.getSocketDescriptor(), buffer);
    }

    return error;
  }

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] formatted response headers",
                 _socket.getSocketDescriptor(), error);
  }

  _state &= ~FORMATTING_HEADERS;

  if (FlushRequestHeaders) {
    _state |= FLUSHING_HEADERS;
  } else {
    _state |= FORMATTING_BODY;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::formatRequestBody(ESB::SharedInt *isRunning,
                                               ESB::Logger *logger) {
  ESB::Error error;
  int availableSize = 0;
  int requestedSize = 0;

  while (isRunning->get()) {
    availableSize = 0;

    requestedSize =
        _transaction->getHandler()->reserveRequestChunk(_transaction);

    if (0 > requestedSize) {
      if (_logger->isLoggable(ESB::Logger::Debug)) {
        _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                     "[socket:%d] client handler aborted connection will "
                     "sending response body",
                     _socket.getSocketDescriptor());
      }

      return ESB_INTR;  // remove from multiplexer
    }

    if (0 == requestedSize) {
      break;  // format last chunk
    }

    error = _transaction->getFormatter()->beginBlock(
        _transaction->getIOBuffer(), requestedSize, &availableSize);

    if (ESB_AGAIN == error) {
      if (_logger->isLoggable(ESB::Logger::Debug)) {
        _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                     "[socket:%d] partially formatted response body",
                     _socket.getSocketDescriptor());
      }

      return ESB_AGAIN;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                     "[socket:%d] error formatting response body: %d",
                     _socket.getSocketDescriptor(), error);
      }

      return error;  // remove from multiplexer
    }

    if (requestedSize < availableSize) {
      availableSize = requestedSize;
    }

    // write the body data

    _transaction->getHandler()->fillRequestChunk(
        _transaction,
        _transaction->getIOBuffer()->getBuffer() +
            _transaction->getIOBuffer()->getWritePosition(),
        availableSize);

    _transaction->getIOBuffer()->setWritePosition(
        _transaction->getIOBuffer()->getWritePosition() + availableSize);

    _bodyBytesWritten += availableSize;

    if (_logger->isLoggable(ESB::Logger::Debug)) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[socket:%d] formatted chunk of size %d",
                   _socket.getSocketDescriptor(), availableSize);
    }

    // beginBlock reserves space for this operation

    error = _transaction->getFormatter()->endBlock(_transaction->getIOBuffer());

    if (ESB_SUCCESS != error) {
      if (_logger->isLoggable(ESB::Logger::Err)) {
        _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                     "[socket:%d] cannot format end block: %d",
                     _socket.getSocketDescriptor(), error);
      }

      return error;  // remove from multiplexer
    }

    if (YieldAfterFormattingChunk) {
      return ESB_SUCCESS;  // keep in multiplexer but yield to another
                           // connection
    }
  }

  if (false == isRunning->get()) {
    return ESB_SHUTDOWN;
  }

  // format last chunk

  error = _transaction->getFormatter()->endBody(_transaction->getIOBuffer());

  if (ESB_AGAIN == error) {
    if (_logger->isLoggable(ESB::Logger::Debug)) {
      _logger->log(
          ESB::Logger::Debug, __FILE__, __LINE__,
          "[socket:%d] insufficient space in output buffer to format end body",
          _socket.getSocketDescriptor());
    }

    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Warning)) {
      _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                   "[socket:%d] error formatting last chunk: %d",
                   _socket.getSocketDescriptor(), error);
    }

    return error;  // remove from multiplexer
  }

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] finshed formatting body",
                 _socket.getSocketDescriptor());
  }

  _state &= ~FORMATTING_BODY;
  _state |= FLUSHING_BODY;

  return ESB_SUCCESS;  // keep in multiplexer
}

ESB::Error HttpClientSocket::flushBuffer(ESB::SharedInt *isRunning,
                                         ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] flushing output buffer",
                 _socket.getSocketDescriptor());
  }

  if (false == _transaction->getIOBuffer()->isReadable()) {
    if (_logger->isLoggable(ESB::Logger::Warning)) {
      _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                   "[socket:%d] formatter jammed",
                   _socket.getSocketDescriptor());
    }

    return ESB_OVERFLOW;  // remove from multiplexer
  }

  ESB::SSize bytesSent;

  while (isRunning->get() && _transaction->getIOBuffer()->isReadable()) {
    bytesSent = _socket.send(_transaction->getIOBuffer());

    if (0 > bytesSent) {
      if (ESB_AGAIN == bytesSent) {
        if (_logger->isLoggable(ESB::Logger::Debug)) {
          _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                       "[socket:%d] would block flushing output buffer",
                       _socket.getSocketDescriptor());
        }

        return ESB_AGAIN;  // keep in multiplexer
      }

      if (FIRST_USE_AFTER_REUSE & _state) {
        _state &= ~FIRST_USE_AFTER_REUSE;
        _state |= RETRY_STALE_CONNECTION;
      }

      if (_logger->isLoggable(ESB::Logger::Warning)) {
        char buffer[100];

        ESB::DescribeError(bytesSent, buffer, sizeof(buffer));

        _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                     "[socket:%d] error flushing output buffer: %s",
                     _socket.getSocketDescriptor(), buffer);
      }

      return bytesSent;  // remove from multiplexer
    }

    _state &= ~FIRST_USE_AFTER_REUSE;

    if (_logger->isLoggable(ESB::Logger::Debug)) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[socket:%d] flushed %d bytes from output buffer",
                   _socket.getSocketDescriptor(), bytesSent);
    }
  }

  return _transaction->getIOBuffer()->isReadable() ? ESB_SHUTDOWN : ESB_SUCCESS;
}

}  // namespace ES
