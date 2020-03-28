#ifndef ES_HTTP_CLIENT_SOCKET_H
#include <ESHttpClientSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
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
                                   ESB::CleanupHandler *cleanupHandler)
    : _state(CONNECTING),
      _bodyBytesWritten(0),
      _pool(pool),
      _transaction(transaction),
      _counters(counters),
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

bool HttpClientSocket::handleAccept(ESB::SocketMultiplexer &multiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_WARNING("socket:%d Cannot handle accept events",
                  _socket.socketDescriptor());
  return true;  // keep in multiplexer
}

bool HttpClientSocket::handleConnect(ESB::SocketMultiplexer &multiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(_socket.isConnected());
  assert(_state & CONNECTING);

  ESB_LOG_INFO("socket:%d Connected", _socket.socketDescriptor());

  _state &= ~CONNECTING;
  _state |= TRANSACTION_BEGIN;

  return handleWritable(multiplexer);
}

bool HttpClientSocket::handleReadable(ESB::SocketMultiplexer &multiplexer) {
  // returning true will keep the socket in the multiplexer
  // returning false will remove the socket from the multiplexer and ultimately
  // close it.

  assert(wantRead());
  assert(_socket.isConnected());
  assert(!(HAS_BEEN_REMOVED & _state));

  ESB::SSize result = 0;
  ESB::Error error = ESB_SUCCESS;

  while (multiplexer.isRunning()) {
    if (!_transaction->getIOBuffer()->isWritable()) {
      ESB_LOG_DEBUG("socket:%d compacting input buffer for",
                    _socket.socketDescriptor());

      if (!_transaction->getIOBuffer()->compact()) {
        ESB_LOG_INFO("socket:%d parser jammed", _socket.socketDescriptor());
        return false;  // remove from multiplexer
      }
    }

    assert(_transaction->getIOBuffer()->isWritable());
    result = _socket.receive(_transaction->getIOBuffer());

    if (!multiplexer.isRunning()) {
      break;
    }

    if (0 > result) {
      error = ESB::LastError();

      if (ESB_AGAIN == error) {
        return true;  // keep in multiplexer
      }

      if (ESB_INTR == error) {
        continue;  // try _socket.receive again
      }

      return handleError(error, multiplexer);
    }

    if (0 == result) {
      return handleRemoteClose(multiplexer);
    }

    ESB_LOG_DEBUG("socket:%d Read %ld bytes", _socket.socketDescriptor(),
                  result);

    if (PARSING_HEADERS & _state) {
      error = parseResponseHeaders(multiplexer);

      if (ESB_AGAIN == error) {
        continue;  // read more data and repeat parse
      }

      if (ESB_SUCCESS != error) {
        return false;  // remove from multiplexer
      }

      if (HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CLOSE ==
          _transaction->getHandler()->receiveResponseHeaders(multiplexer,
                                                             _transaction)) {
        ESB_LOG_DEBUG(
            "socket:%d Client request header handler aborting connection",
            _socket.socketDescriptor());
        return false;  // remove from multiplexer
      }

      if (false == _transaction->getResponse()->hasBody()) {
        unsigned char byte = 0;

        if (HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CLOSE ==
            _transaction->getHandler()->receiveResponseBody(
                multiplexer, _transaction, &byte, 0)) {
          ESB_LOG_DEBUG("socket:%d Client body handler aborting connection",
                        _socket.socketDescriptor());
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

    error = parseResponseBody(multiplexer);

    if (ESB_AGAIN == error) {
      continue;  // read more data and repeat parse
    }

    return false;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("socket:%d multiplexer shutdown with socket in parse state",
                _socket.socketDescriptor());
  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleWritable(ESB::SocketMultiplexer &multiplexer) {
  assert(wantWrite());
  assert(_socket.isConnected());
  assert(!(HAS_BEEN_REMOVED & _state));

  ESB::Error error = ESB_SUCCESS;

  if (_state & TRANSACTION_BEGIN) {
    // TODO make connection reuse more configurable
    if (!HttpClientSocket::GetReuseConnections() &&
        !_transaction->getRequest()->getHeader(
            (const unsigned char *)"Connection")) {
      error = _transaction->getRequest()->addHeader(
          (const unsigned char *)"Connection", (const unsigned char *)"close",
          _transaction->getAllocator());

      if (ESB_SUCCESS != error) {
        ESB_LOG_ERROR_ERRNO(error,
                            "socket:%d cannot add connection: close "
                            "header",
                            _socket.socketDescriptor());
        return false;  // remove from multiplexer
      }
    }

    // TODO add user agent, etc headers

    _state &= ~TRANSACTION_BEGIN;
    _state |= FORMATTING_HEADERS;
  }

  while (multiplexer.isRunning()) {
    if (FORMATTING_HEADERS & _state) {
      error = formatRequestHeaders(multiplexer);

      if (ESB_AGAIN == error) {
        error = flushBuffer(multiplexer);

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
      error = flushBuffer(multiplexer);

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
      error = formatRequestBody(multiplexer);

      if (ESB_AGAIN == error) {
        error = flushBuffer(multiplexer);

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

    error = flushBuffer(multiplexer);

    if (ESB_AGAIN == error) {
      return true;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      return false;  // remove from multiplexer
    }

    _state &= ~FLUSHING_BODY;
    _state |= PARSING_HEADERS;

    return YieldAfterSendingRequest ? true : handleReadable(multiplexer);
  }

  ESB_LOG_DEBUG("socket:%d multiplexer shutdown with socket in format state",
                _socket.socketDescriptor());
  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleError(ESB::Error error,
                                   ESB::SocketMultiplexer &multiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));

  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.peerAddress().presentationAddress(dottedAddress,
                                              sizeof(dottedAddress));
    ESB_LOG_INFO("socket:%d to %s:%u has error", _socket.socketDescriptor(),
                 dottedAddress, _socket.peerAddress().port());
  }

  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleRemoteClose(ESB::SocketMultiplexer &multiplexer) {
  // TODO - this may just mean the client closed its half of the socket but is
  // still expecting a response.

  assert(!(HAS_BEEN_REMOVED & _state));

  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.peerAddress().presentationAddress(dottedAddress,
                                              sizeof(dottedAddress));
    ESB_LOG_INFO("socket:%d to %s:%u was closed by peer",
                 _socket.socketDescriptor(), dottedAddress,
                 _socket.peerAddress().port());
  }

  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleIdle(ESB::SocketMultiplexer &multiplexerg) {
  assert(!(HAS_BEEN_REMOVED & _state));

  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.peerAddress().presentationAddress(dottedAddress,
                                              sizeof(dottedAddress));
    ESB_LOG_INFO("socket:%d to %s:%u is idle", _socket.socketDescriptor(),
                 dottedAddress, _socket.peerAddress().port());
  }

  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleRemove(ESB::SocketMultiplexer &multiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));

  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.peerAddress().presentationAddress(dottedAddress,
                                              sizeof(dottedAddress));
    ESB_LOG_INFO("socket:%d to %s:%u has been closed",
                 _socket.socketDescriptor(), dottedAddress,
                 _socket.peerAddress().port());
  }

  bool reuseConnection = false;

  if (_state & TRANSACTION_BEGIN) {
    _counters->getFailures()->record(_transaction->getStartTime(),
                                     ESB::Date::Now());

    _transaction->getHandler()->endClientTransaction(
        multiplexer, _transaction,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_BEGIN);
  } else if (_state & CONNECTING) {
    _counters->getFailures()->record(_transaction->getStartTime(),
                                     ESB::Date::Now());

    _transaction->getHandler()->endClientTransaction(
        multiplexer, _transaction,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
  } else if (_state & (FORMATTING_HEADERS | FLUSHING_HEADERS)) {
    _counters->getFailures()->record(_transaction->getStartTime(),
                                     ESB::Date::Now());

    _transaction->getHandler()->endClientTransaction(
        multiplexer, _transaction,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS);
  } else if (_state & (FORMATTING_BODY | FLUSHING_BODY)) {
    _counters->getFailures()->record(_transaction->getStartTime(),
                                     ESB::Date::Now());

    _transaction->getHandler()->endClientTransaction(
        multiplexer, _transaction,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY);
  } else if (_state & PARSING_HEADERS) {
    _counters->getFailures()->record(_transaction->getStartTime(),
                                     ESB::Date::Now());

    _transaction->getHandler()->endClientTransaction(
        multiplexer, _transaction,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS);
  } else if (_state & PARSING_BODY) {
    _counters->getFailures()->record(_transaction->getStartTime(),
                                     ESB::Date::Now());

    _transaction->getHandler()->endClientTransaction(
        multiplexer, _transaction,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY);
  } else if (_state & TRANSACTION_END) {
    _counters->getSuccesses()->record(_transaction->getStartTime(),
                                      ESB::Date::Now());

    if (GetReuseConnections()) {
      const HttpHeader *header = _transaction->getResponse()->getHeader(
          (unsigned const char *)"Connection");

      if (header && header->getFieldValue() &&
          !strcasecmp("close", (const char *)header->getFieldValue())) {
        reuseConnection = false;
      } else {
        reuseConnection = true;
      }
    } else {
      reuseConnection = false;
    }

    _transaction->getHandler()->endClientTransaction(
        multiplexer, _transaction,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_END);
  } else if (_state & RETRY_STALE_CONNECTION) {
    ESB_LOG_DEBUG("socket:%d connection stale, retrying transaction",
                  _socket.socketDescriptor());

    ESB::Error error = _pool->executeClientTransaction(_transaction);

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "socket:%d Cannot retry transaction: %s",
                         _socket.socketDescriptor(), buffer);

      _state &= ~RETRY_STALE_CONNECTION;
      _state |= TRANSACTION_BEGIN;

      _transaction->getHandler()->endClientTransaction(
          multiplexer, _transaction,
          HttpClientHandler::ES_HTTP_CLIENT_HANDLER_BEGIN);
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

SOCKET HttpClientSocket::socketDescriptor() const {
  return _socket.socketDescriptor();
}

ESB::CleanupHandler *HttpClientSocket::cleanupHandler() {
  return _cleanupHandler;
}

const char *HttpClientSocket::getName() const { return "HttpClientSocket"; }

ESB::Error HttpClientSocket::parseResponseHeaders(
    ESB::SocketMultiplexer &multiplexer) {
  ESB::Error error = _transaction->getParser()->parseHeaders(
      _transaction->getIOBuffer(), _transaction->getResponse());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("socket:%d need more header data from stream",
                  _socket.socketDescriptor());

    if (!_transaction->getIOBuffer()->compact()) {
      ESB_LOG_INFO("socket:%d cannot parse headers: parser jammed",
                   _socket.socketDescriptor());
      return ESB_OVERFLOW;  // remove from multiplexer
    }

    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "socket:%d cannot parse headers",
                       _socket.socketDescriptor());
    return error;  // remove from multiplexer
  }

  // parse complete

  if (ESB_DEBUG_LOGGABLE) {
    ESB_LOG_DEBUG("socket:%d headers parsed", _socket.socketDescriptor());
    ESB_LOG_DEBUG("socket:%d StatusCode: %d", _socket.socketDescriptor(),
                  _transaction->getResponse()->getStatusCode());
    ESB_LOG_DEBUG("socket:%d ReasonPhrase: %s", _socket.socketDescriptor(),
                  ESB_SAFE_STR(_transaction->getResponse()->getReasonPhrase()));
    ESB_LOG_DEBUG("socket:%d Version: HTTP/%d.%d", _socket.socketDescriptor(),
                  _transaction->getResponse()->getHttpVersion() / 100,
                  _transaction->getResponse()->getHttpVersion() % 100 / 10);

    HttpHeader *header =
        (HttpHeader *)_transaction->getResponse()->getHeaders()->first();
    for (; header; header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("socket:%d %s: %s", _socket.socketDescriptor(),
                    ESB_SAFE_STR(header->getFieldName()),
                    ESB_SAFE_STR(header->getFieldValue()));
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::parseResponseBody(
    ESB::SocketMultiplexer &multiplexer) {
  int startingPosition = 0;
  int chunkSize = 0;
  ESB::Error error = ESB_SUCCESS;

  while (multiplexer.isRunning()) {
    error = _transaction->getParser()->parseBody(_transaction->getIOBuffer(),
                                                 &startingPosition, &chunkSize);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("socket:%d need more body data from stream",
                    _socket.socketDescriptor());
      if (!_transaction->getIOBuffer()->isWritable()) {
        ESB_LOG_DEBUG("socket:%d compacting input buffer",
                      _socket.socketDescriptor());
        if (!_transaction->getIOBuffer()->compact()) {
          ESB_LOG_INFO("socket:%d cannot parse body: parser jammed",
                       _socket.socketDescriptor());
          return ESB_OVERFLOW;  // remove from multiplexer
        }
      }
      return ESB_AGAIN;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "socket:%d error parsing body",
                         _socket.socketDescriptor());
      return error;  // remove from multiplexer
    }

    if (0 == chunkSize) {
      ESB_LOG_DEBUG("socket:%d parsed body", _socket.socketDescriptor());
      unsigned char byte = 0;
      HttpClientHandler::Result result =
          _transaction->getHandler()->receiveResponseBody(
              multiplexer, _transaction, &byte, 0);

      if (HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CLOSE == result) {
        ESB_LOG_DEBUG(
            "socket:%d client handler aborting connection after "
            "last body chunk",
            _socket.socketDescriptor());
        return ESB_INTR;
      }

      _state &= ~PARSING_BODY;
      _state |= TRANSACTION_END;

      return ESB_SUCCESS;
    }

    if (ESB_DEBUG_LOGGABLE) {
      char buffer[4096];
      memcpy(buffer, _transaction->getIOBuffer()->buffer() + startingPosition,
             chunkSize > (int)sizeof(buffer) ? sizeof(buffer) : chunkSize);
      buffer[chunkSize > (int)sizeof(buffer) ? sizeof(buffer) : chunkSize] = 0;

      ESB_LOG_DEBUG("socket:%d read chunk: %s", _socket.socketDescriptor(),
                    buffer);
    }

    HttpClientHandler::Result result;

    result = _transaction->getHandler()->receiveResponseBody(
        multiplexer, _transaction,
        _transaction->getIOBuffer()->buffer() + startingPosition, chunkSize);

    if (HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CLOSE == result) {
      ESB_LOG_DEBUG(
          "socket:%d client handler aborting connection before "
          "last body chunk",
          _socket.socketDescriptor());
      return ESB_INTR;
    }
  }

  return ESB_SHUTDOWN;
}

ESB::Error HttpClientSocket::formatRequestHeaders(
    ESB::SocketMultiplexer &multiplexer) {
  ESB::Error error = _transaction->getFormatter()->formatHeaders(
      _transaction->getIOBuffer(), _transaction->getRequest());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("socket:%d partially formatted response headers",
                  _socket.socketDescriptor());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "socket:%d error formatting response header",
                       _socket.socketDescriptor());
    return error;
  }

  ESB_LOG_DEBUG("socket:%d formatted response headers",
                _socket.socketDescriptor());
  _state &= ~FORMATTING_HEADERS;

  if (FlushRequestHeaders) {
    _state |= FLUSHING_HEADERS;
  } else {
    _state |= FORMATTING_BODY;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::formatRequestBody(
    ESB::SocketMultiplexer &multiplexer) {
  ESB::Error error;
  int availableSize = 0;
  int requestedSize = 0;

  while (multiplexer.isRunning()) {
    availableSize = 0;

    requestedSize = _transaction->getHandler()->reserveRequestChunk(
        multiplexer, _transaction);

    if (0 > requestedSize) {
      ESB_LOG_DEBUG(
          "socket:%d client handler aborted connection will "
          "sending response body",
          _socket.socketDescriptor());
      return ESB_INTR;  // remove from multiplexer
    }

    if (0 == requestedSize) {
      break;  // format last chunk
    }

    error = _transaction->getFormatter()->beginBlock(
        _transaction->getIOBuffer(), requestedSize, &availableSize);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("socket:%d partially formatted response body",
                    _socket.socketDescriptor());
      return ESB_AGAIN;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "socket:%d error formatting response body",
                         _socket.socketDescriptor());
      return error;  // remove from multiplexer
    }

    if (requestedSize < availableSize) {
      availableSize = requestedSize;
    }

    // write the body data

    _transaction->getHandler()->fillRequestChunk(
        multiplexer, _transaction,
        _transaction->getIOBuffer()->buffer() +
            _transaction->getIOBuffer()->writePosition(),
        availableSize);
    _transaction->getIOBuffer()->setWritePosition(
        _transaction->getIOBuffer()->writePosition() + availableSize);
    _bodyBytesWritten += availableSize;

    ESB_LOG_DEBUG("socket:%d formatted chunk of size %d",
                  _socket.socketDescriptor(), availableSize);

    // beginBlock reserves space for this operation

    error = _transaction->getFormatter()->endBlock(_transaction->getIOBuffer());

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "socket:%d cannot format end block",
                         _socket.socketDescriptor());
      return error;  // remove from multiplexer
    }

    if (YieldAfterFormattingChunk) {
      return ESB_SUCCESS;  // keep in multiplexer but yield to another
                           // connection
    }
  }

  if (!multiplexer.isRunning()) {
    return ESB_SHUTDOWN;
  }

  // format last chunk

  error = _transaction->getFormatter()->endBody(_transaction->getIOBuffer());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG(
        "socket:%d insufficient space in output buffer to format end body",
        _socket.socketDescriptor());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "socket:%d error formatting last chunk",
                       _socket.socketDescriptor());
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("socket:%d finshed formatting body",
                _socket.socketDescriptor());

  _state &= ~FORMATTING_BODY;
  _state |= FLUSHING_BODY;

  return ESB_SUCCESS;  // keep in multiplexer
}

ESB::Error HttpClientSocket::flushBuffer(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_DEBUG("socket:%d flushing output buffer", _socket.socketDescriptor());

  if (!_transaction->getIOBuffer()->isReadable()) {
    ESB_LOG_INFO("socket:%d formatter jammed", _socket.socketDescriptor());
    return ESB_OVERFLOW;  // remove from multiplexer
  }

  ESB::SSize bytesSent;

  while (multiplexer.isRunning() && _transaction->getIOBuffer()->isReadable()) {
    bytesSent = _socket.send(_transaction->getIOBuffer());

    if (0 > bytesSent) {
      if (ESB_AGAIN == bytesSent) {
        ESB_LOG_DEBUG("socket:%d would block flushing output buffer",
                      _socket.socketDescriptor());
        return ESB_AGAIN;  // keep in multiplexer
      }

      if (FIRST_USE_AFTER_REUSE & _state) {
        _state &= ~FIRST_USE_AFTER_REUSE;
        _state |= RETRY_STALE_CONNECTION;
      }

      ESB_LOG_INFO_ERRNO(bytesSent, "socket:%d error flushing output buffer",
                         _socket.socketDescriptor());
      return bytesSent;  // remove from multiplexer
    }

    _state &= ~FIRST_USE_AFTER_REUSE;
    ESB_LOG_DEBUG("socket:%d flushed %ld bytes from output buffer",
                  _socket.socketDescriptor(), bytesSent);
  }

  return _transaction->getIOBuffer()->isReadable() ? ESB_SHUTDOWN : ESB_SUCCESS;
}

}  // namespace ES
