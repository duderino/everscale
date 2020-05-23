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
#define RECV_PAUSED (1 << 12)
#define SEND_PAUSED (1 << 13)

// TODO - max requests per connection option (1 disables keepalives)
// TODO - max header size option
// TODO - max body size option

namespace ES {

bool HttpClientSocket::_ReuseConnections = true;

HttpClientSocket::HttpClientSocket(HttpClientHandler &handler,
                                   HttpMultiplexerExtended &multiplexer,
                                   ESB::SocketAddress &peerAddress,
                                   HttpClientCounters &counters,
                                   ESB::CleanupHandler &cleanupHandler)
    : _state(CONNECTING),
      _bodyBytesWritten(0),
      _multiplexer(multiplexer),
      _handler(handler),
      _transaction(NULL),
      _counters(counters),
      _cleanupHandler(cleanupHandler),
      _recvBuffer(NULL),
      _sendBuffer(NULL),
      _socket(peerAddress, false) {}

HttpClientSocket::~HttpClientSocket() {
  if (_recvBuffer) {
    _multiplexer.releaseBuffer(_recvBuffer);
    _recvBuffer = NULL;
  }
  if (_sendBuffer) {
    _multiplexer.releaseBuffer(_sendBuffer);
    _sendBuffer = NULL;
  }
  if (_transaction) {
    _multiplexer.destroyClientTransaction(_transaction);
    _transaction = NULL;
  }
}

ESB::Error HttpClientSocket::reset(bool reused,
                                   HttpClientTransaction *transaction) {
  assert(!_recvBuffer);
  assert(!_sendBuffer);
  assert(!_transaction);

  if (!transaction) {
    return ESB_NULL_POINTER;
  }

  _state = TRANSACTION_BEGIN;

  if (reused) {
    assert(isConnected());

    _state |= FIRST_USE_AFTER_REUSE;
  } else {
    assert(!isConnected());
  }

  _bodyBytesWritten = 0;
  _transaction = transaction;

  return ESB_SUCCESS;
}

bool HttpClientSocket::wantAccept() { return false; }

bool HttpClientSocket::wantConnect() { return _state & CONNECTING; }

bool HttpClientSocket::wantRead() {
  if (_state & RECV_PAUSED) {
    return false;
  }

  return (_state & (PARSING_HEADERS | PARSING_BODY)) != 0;
}

bool HttpClientSocket::wantWrite() {
  if (_state & SEND_PAUSED) {
    return false;
  }

  return (_state & (TRANSACTION_BEGIN | FORMATTING_HEADERS | FLUSHING_HEADERS |
                    FORMATTING_BODY | FLUSHING_BODY)) != 0;
}

bool HttpClientSocket::isIdle() {
  // TODO - implement idle timeout

  return false;
}

ESB::Error HttpClientSocket::handleAccept() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_WARNING("[%d] Cannot handle accept events",
                  _socket.socketDescriptor());
  return ESB_SUCCESS;  // keep in multiplexer
}

bool HttpClientSocket::handleConnect() {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(_socket.isConnected());
  assert(_state & CONNECTING);

  ESB_LOG_INFO("[%s] Connected to peer", _socket.logAddress());

  _state &= ~CONNECTING;
  _state |= TRANSACTION_BEGIN;

  return handleWritable();
}

bool HttpClientSocket::handleReadable() {
  // returning true will keep the socket in the multiplexer
  // returning false will remove the socket from the multiplexer and ultimately
  // close it.

  assert(wantRead());
  assert(_socket.isConnected());
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(_transaction);

  if (!_recvBuffer) {
    _recvBuffer = _multiplexer.acquireBuffer();
    if (!_recvBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer",
                          _socket.logAddress());
      return false;  // remove from multiplexer
    }
  }

  ESB::SSize result = 0;
  ESB::Error error = ESB_SUCCESS;

  while (!_multiplexer.shutdown()) {
    if (!_recvBuffer->isWritable()) {
      ESB_LOG_DEBUG("[%s] compacting input buffer", _socket.logAddress());
      if (!_recvBuffer->compact()) {
        ESB_LOG_WARNING("[%s] parser jammed", _socket.logAddress());
        return false;  // remove from multiplexer
      }
    }

    assert(_recvBuffer->isWritable());
    result = _socket.receive(_recvBuffer);

    if (_multiplexer.shutdown()) {
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

      return handleError(error);
    }

    if (0 == result) {
      return handleRemoteClose();
    }

    ESB_LOG_DEBUG("[%s] Read %ld bytes", _socket.logAddress(), result);

    if (PARSING_HEADERS & _state) {
      error = parseResponseHeaders();

      if (ESB_AGAIN == error) {
        continue;  // read more data and repeat parse
      }

      if (ESB_SUCCESS != error) {
        return false;  // remove from multiplexer
      }

      error = _handler.receiveResponseHeaders(_multiplexer, *this);
      if (ESB_SUCCESS != error) {
        ESB_LOG_DEBUG_ERRNO(
            error, "[%s] Client request header handler aborting connection",
            _socket.logAddress());
        return false;  // remove from multiplexer
      }

      if (!_transaction->response().hasBody()) {
        unsigned char byte = 0;
        ESB::UInt32 bytesWritten = 0;

        error = _handler.consumeResponseChunk(_multiplexer, *this, &byte, 0,
                                              &bytesWritten);
        if (ESB_SUCCESS != error) {
          ESB_LOG_DEBUG_ERRNO(error,
                              "[%s] Client body handler aborting connection",
                              _socket.logAddress());
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
    assert(_transaction->response().hasBody());

    error = parseResponseBody();

    if (ESB_AGAIN == error) {
      continue;  // read more data and repeat parse
    }

    if (ESB_PAUSE == error) {
      _state |= RECV_PAUSED;
      return true;  // keep in multiplexer, but remove from read interest set.
    }

    return false;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown with socket in parse state",
                _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleWritable() {
  assert(wantWrite());
  assert(_socket.isConnected());
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(SEND_PAUSED & _state));
  assert(_transaction);

  if (!_sendBuffer) {
    _sendBuffer = _multiplexer.acquireBuffer();
    if (!_sendBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer",
                          _socket.logAddress());
      return false;  // remove from multiplexer
    }
  }

  ESB::Error error = ESB_SUCCESS;

  if (_state & TRANSACTION_BEGIN) {
    // TODO make connection reuse more configurable
    if (!HttpClientSocket::GetReuseConnections() &&
        !_transaction->request().findHeader("Connection")) {
      error = _transaction->request().addHeader("Connection", "close",
                                                _transaction->allocator());

      if (ESB_SUCCESS != error) {
        ESB_LOG_ERROR_ERRNO(error, "[%s] cannot add connection: close header",
                            _socket.logAddress());
        return false;  // remove from multiplexer
      }
    }

    // TODO add user agent, etc headers

    _state &= ~TRANSACTION_BEGIN;
    _state |= FORMATTING_HEADERS;
  }

  while (!_multiplexer.shutdown()) {
    if (FORMATTING_HEADERS & _state) {
      error = formatRequestHeaders();

      if (ESB_AGAIN == error) {
        error = flushBuffer();

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
    }

    if (FLUSHING_HEADERS & _state) {
      error = flushBuffer();

      if (ESB_AGAIN == error) {
        return true;  // keep in multiplexer, wait for socket to become writable
      }

      if (ESB_SUCCESS != error) {
        return false;  // remove from multiplexer
      }

      _state &= ~FLUSHING_HEADERS;

      if (_transaction->request().hasBody()) {
        _state |= FORMATTING_BODY;
      } else {
        _state |= PARSING_HEADERS;
      }
    }

    if (FORMATTING_BODY & _state) {
      error = formatRequestBody();

      if (ESB_AGAIN == error) {
        error = flushBuffer();

        if (ESB_AGAIN == error) {
          return true;  // keep in multiplexer, wait for socket to become
                        // writable
        }

        if (ESB_SUCCESS != error) {
          return false;  // remove from multiplexer
        }

        continue;
      }

      if (ESB_PAUSE == error) {
        _state |= SEND_PAUSED;
        return true;  // keep in multiplexer but remove from write interest
      }

      if (ESB_SUCCESS != error) {
        return false;  // remove from multiplexer;
      }

      if (FORMATTING_BODY & _state) {
        continue;
      }
    }

    assert(FLUSHING_BODY & _state);

    error = flushBuffer();

    if (ESB_AGAIN == error) {
      return true;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      return false;  // remove from multiplexer
    }

    _state &= ~FLUSHING_BODY;
    _state |= PARSING_HEADERS;

    return handleReadable();
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown with socket in format state",
                _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleError(ESB::Error error) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] socket had error", _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleRemoteClose() {
  // TODO - this may just mean the client closed its half of the socket but is
  // still expecting a response.
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] socket was closed by peer", _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleIdle() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] socket is idle", _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleRemove() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] socket has been removed", _socket.logAddress());

  if (_sendBuffer) {
    _multiplexer.releaseBuffer(_sendBuffer);
    _sendBuffer = NULL;
  }
  if (_recvBuffer) {
    _multiplexer.releaseBuffer(_recvBuffer);
    _recvBuffer = NULL;
  }

  bool reuseConnection = false;

  if (_state & TRANSACTION_BEGIN) {
    assert(_transaction);
    _counters.getFailures()->record(_transaction->startTime(),
                                    ESB::Date::Now());

    _handler.endTransaction(_multiplexer, *this,
                            HttpClientHandler::ES_HTTP_CLIENT_HANDLER_BEGIN);
  } else if (_state & CONNECTING) {
    assert(_transaction);
    _counters.getFailures()->record(_transaction->startTime(),
                                    ESB::Date::Now());

    _handler.endTransaction(_multiplexer, *this,
                            HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
  } else if (_state & (FORMATTING_HEADERS | FLUSHING_HEADERS)) {
    assert(_transaction);
    _counters.getFailures()->record(_transaction->startTime(),
                                    ESB::Date::Now());

    _handler.endTransaction(
        _multiplexer, *this,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS);
  } else if (_state & (FORMATTING_BODY | FLUSHING_BODY)) {
    assert(_transaction);
    _counters.getFailures()->record(_transaction->startTime(),
                                    ESB::Date::Now());
    _handler.endTransaction(
        _multiplexer, *this,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY);
  } else if (_state & PARSING_HEADERS) {
    assert(_transaction);
    _counters.getFailures()->record(_transaction->startTime(),
                                    ESB::Date::Now());
    _handler.endTransaction(
        _multiplexer, *this,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS);
  } else if (_state & PARSING_BODY) {
    assert(_transaction);
    _counters.getFailures()->record(_transaction->startTime(),
                                    ESB::Date::Now());
    _handler.endTransaction(
        _multiplexer, *this,
        HttpClientHandler::ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY);
  } else if (_state & TRANSACTION_END) {
    assert(_transaction);
    _counters.getSuccesses()->record(_transaction->startTime(),
                                     ESB::Date::Now());

    if (GetReuseConnections()) {
      const HttpHeader *header =
          _transaction->response().findHeader("Connection");

      if (header && header->fieldValue() &&
          !strcasecmp("close", (const char *)header->fieldValue())) {
        reuseConnection = false;
      } else {
        reuseConnection = true;
      }
    } else {
      reuseConnection = false;
    }

    _handler.endTransaction(_multiplexer, *this,
                            HttpClientHandler::ES_HTTP_CLIENT_HANDLER_END);
  } else if (_state & RETRY_STALE_CONNECTION) {
    assert(_transaction);
    ESB_LOG_DEBUG("[%s] connection stale, retrying transaction",
                  _socket.logAddress());
    ESB::Error error = _multiplexer.executeClientTransaction(_transaction);

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] Cannot retry transaction",
                         _socket.logAddress());
      _state &= ~RETRY_STALE_CONNECTION;
      _state |= TRANSACTION_BEGIN;

      _handler.endTransaction(_multiplexer, *this,
                              HttpClientHandler::ES_HTTP_CLIENT_HANDLER_BEGIN);
    }
  }

  if (0x00 == (_state & RETRY_STALE_CONNECTION)) {
    if (_transaction) {
      _multiplexer.destroyClientTransaction(_transaction);
      _transaction = NULL;
    }
  }

  if (!reuseConnection) {
    _socket.close();
  }

  _transaction = NULL;
  _state = HAS_BEEN_REMOVED;

  return true;  // call cleanup handler on us after this returns
}

SOCKET HttpClientSocket::socketDescriptor() const {
  return _socket.socketDescriptor();
}

ESB::CleanupHandler *HttpClientSocket::cleanupHandler() {
  return &_cleanupHandler;
}

const char *HttpClientSocket::name() const { return logAddress(); }

ESB::Error HttpClientSocket::parseResponseHeaders() {
  ESB::Error error = _transaction->getParser()->parseHeaders(
      _recvBuffer, _transaction->response());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] need more response header data from stream",
                  _socket.logAddress());

    if (!_recvBuffer->compact()) {
      ESB_LOG_INFO("[%s] cannot parse response headers: parser jammed",
                   _socket.logAddress());
      return ESB_OVERFLOW;  // remove from multiplexer
    }

    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot response parse headers",
                       _socket.logAddress());
    return error;  // remove from multiplexer
  }

  // parse complete

  if (ESB_DEBUG_LOGGABLE) {
    ESB_LOG_DEBUG("[%s] response headers parsed", _socket.logAddress());
    ESB_LOG_DEBUG("[%s] StatusCode: %d", _socket.logAddress(),
                  _transaction->response().statusCode());
    ESB_LOG_DEBUG("[%s] ReasonPhrase: %s", _socket.logAddress(),
                  ESB_SAFE_STR(_transaction->response().reasonPhrase()));
    ESB_LOG_DEBUG("[%s] Version: HTTP/%d.%d", _socket.logAddress(),
                  _transaction->response().httpVersion() / 100,
                  _transaction->response().httpVersion() % 100 / 10);

    HttpHeader *header =
        (HttpHeader *)_transaction->response().headers().first();
    for (; header; header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("[%s] %s: %s", _socket.logAddress(),
                    ESB_SAFE_STR(header->fieldName()),
                    ESB_SAFE_STR(header->fieldValue()));
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::parseResponseBody() {
  assert(_transaction);

  //
  // Until the body is read or either the parser or handler return ESB_AGAIN,
  // ask the parser how much body data is ready to be read, pass the available
  // body data to the handler, and pass back the body data actually consumed to
  // the parser.
  //

  while (!_multiplexer.shutdown()) {
    ESB::UInt32 bufferOffset = 0U;
    ESB::UInt32 bytesAvailable = 0U;
    ESB::UInt32 bytesConsumed = 0U;

    ESB::Error error = _transaction->getParser()->parseBody(
        _recvBuffer, &bufferOffset, &bytesAvailable);
    switch (error) {
      case ESB_SUCCESS:
        break;
      case ESB_AGAIN:
        ESB_LOG_DEBUG("[%s] need more response body data from stream",
                      _socket.logAddress());
        if (!_recvBuffer->isWritable()) {
          ESB_LOG_DEBUG("[%s] compacting response input buffer",
                        _socket.logAddress());
          if (!_recvBuffer->compact()) {
            ESB_LOG_INFO("[%s] cannot parse response body: parser jammed",
                         _socket.logAddress());
            return ESB_OVERFLOW;  // remove from multiplexer
          }
        }
        return ESB_AGAIN;  // keep in multiplexer
      default:
        ESB_LOG_INFO_ERRNO(error, "[%s] error parsing response body",
                           _socket.logAddress());
        return error;  // remove from multiplexer
    }

    if (0 == bytesAvailable) {
      // Last chunk has been read
      ESB_LOG_DEBUG("[%s] parsed response body", _socket.logAddress());
      unsigned char byte = 0;
      error = _handler.consumeResponseChunk(_multiplexer, *this, &byte, 0U,
                                            &bytesConsumed);

      if (ESB_SUCCESS != error) {
        ESB_LOG_DEBUG_ERRNO(
            error,
            "[%s] handler aborting connection after last response body chunk",
            _socket.logAddress());
        return ESB_AGAIN == error ? ESB_INTR : error;
      }

      _state &= ~PARSING_BODY;
      _state |= TRANSACTION_END;
      return ESB_SUCCESS;
    }

    ESB_LOG_DEBUG("[%s] offering response chunk of size %u",
                  _socket.logAddress(), bytesAvailable);

    error = _handler.consumeResponseChunk(_multiplexer, *this,
                                          _recvBuffer->buffer() + bufferOffset,
                                          bytesAvailable, &bytesConsumed);
    switch (error) {
      case ESB_SUCCESS:
        if (0 == bytesConsumed) {
          ESB_LOG_DEBUG(
              "[%s] pausing response body receive until handler is ready",
              _socket.logAddress());
          return ESB_PAUSE;
        }
        break;
      case ESB_AGAIN:
        ESB_LOG_DEBUG(
            "[%s] pausing response body receive until handler is ready",
            _socket.logAddress());
        return ESB_PAUSE;
      default:
        ESB_LOG_DEBUG_ERRNO(
            error,
            "[%s] handler aborting connection before last response body chunk",
            _socket.logAddress());
        return error;  // remove from multiplexer
    }

    error = _transaction->getParser()->consumeBody(_recvBuffer, bytesConsumed);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot consume %u response chunk bytes",
                          _socket.logAddress(), bytesAvailable);
      return error;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("[%s] consumed %u out of %u response chunk bytes",
                  _socket.logAddress(), bytesConsumed, bytesAvailable);
  }

  return ESB_SHUTDOWN;
}

ESB::Error HttpClientSocket::formatRequestHeaders() {
  ESB::Error error = _transaction->getFormatter()->formatHeaders(
      _sendBuffer, _transaction->request());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] partially formatted request headers",
                  _socket.logAddress());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error formatting request header",
                       _socket.logAddress());
    return error;
  }

  ESB_LOG_DEBUG("[%s] formatted request headers", _socket.logAddress());
  _state &= ~FORMATTING_HEADERS;
  _state |= FORMATTING_BODY;

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::formatRequestBody() {
  assert(_transaction);
  ESB::Error error = ESB_SUCCESS;
  ESB::UInt32 maxChunkSize = 0;
  ESB::UInt32 offeredSize = 0;

  while (!_multiplexer.shutdown()) {
    maxChunkSize = 0;
    offeredSize = 0;

    error = _handler.offerRequestChunk(_multiplexer, *this, &offeredSize);

    switch (error) {
      case ESB_AGAIN:
        ESB_LOG_DEBUG("[%s] handler cannot offer request chunk, pausing socket",
                      _socket.logAddress());
        return ESB_PAUSE;  // keep in multiplexer
      case ESB_SUCCESS:
        ESB_LOG_DEBUG("[%s] handler offers request chunk of %u bytes",
                      _socket.logAddress(), offeredSize);
        break;
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler error offering request chunk",
                            _socket.logAddress());
        return error;  // remove from multiplexer
    }

    if (0 == offeredSize) {
      // last chunk
      break;
    }

    error = _transaction->getFormatter()->beginBlock(_sendBuffer, offeredSize,
                                                     &maxChunkSize);

    switch (error) {
      case ESB_AGAIN:
        ESB_LOG_DEBUG("[%s] partially formatted request body",
                      _socket.logAddress());
        return ESB_AGAIN;  // keep in multiplexer
      case ESB_SUCCESS:
        break;
      default:
        ESB_LOG_INFO_ERRNO(error, "[%s] error formatting request body",
                           _socket.logAddress());
        return error;  // remove from multiplexer
    }

    ESB::UInt32 chunkSize = ESB_MIN(offeredSize, maxChunkSize);

    // write the body data

    error = _handler.produceRequestChunk(
        _multiplexer, *this,
        _sendBuffer->buffer() + _sendBuffer->writePosition(), chunkSize);
    _sendBuffer->setWritePosition(_sendBuffer->writePosition() + chunkSize);
    _bodyBytesWritten += chunkSize;

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] Cannot format request chunk of size %u",
                         _socket.logAddress(), chunkSize);
      return error;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("[%s] formatted request chunk of size %u",
                  _socket.logAddress(), chunkSize);

    // beginBlock reserves space for this operation

    error = _transaction->getFormatter()->endBlock(_sendBuffer);

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot format request chunk end block",
                         _socket.logAddress());
      return error;  // remove from multiplexer
    }
  }

  if (_multiplexer.shutdown()) {
    return ESB_SHUTDOWN;
  }

  // format last chunk

  error = _transaction->getFormatter()->endBody(_sendBuffer);

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] insufficient space in output buffer to format end body",
                  _socket.logAddress());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error formatting last request chunk",
                       _socket.logAddress());
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[%s] finshed formatting request body", _socket.logAddress());

  _state &= ~FORMATTING_BODY;
  _state |= FLUSHING_BODY;

  return ESB_SUCCESS;  // keep in multiplexer
}

ESB::Error HttpClientSocket::flushBuffer() {
  ESB_LOG_DEBUG("[%s] flushing request output buffer", _socket.logAddress());

  if (!_sendBuffer->isReadable()) {
    ESB_LOG_INFO("[%s] request formatter jammed", _socket.logAddress());
    return ESB_OVERFLOW;  // remove from multiplexer
  }

  ESB::SSize bytesSent;

  while (!_multiplexer.shutdown() && _sendBuffer->isReadable()) {
    bytesSent = _socket.send(_sendBuffer);

    if (0 > bytesSent) {
      if (ESB_AGAIN == bytesSent) {
        ESB_LOG_DEBUG("[%s] would block flushing request output buffer",
                      _socket.logAddress());
        return ESB_AGAIN;  // keep in multiplexer
      }

      if (FIRST_USE_AFTER_REUSE & _state) {
        _state &= ~FIRST_USE_AFTER_REUSE;
        _state |= RETRY_STALE_CONNECTION;
      }

      ESB_LOG_INFO_ERRNO(bytesSent, "[%s] error flushing request output buffer",
                         _socket.logAddress());
      return bytesSent;  // remove from multiplexer
    }

    _state &= ~FIRST_USE_AFTER_REUSE;
    ESB_LOG_DEBUG("[%s] flushed %ld bytes from request output buffer",
                  _socket.logAddress(), bytesSent);
  }

  return _sendBuffer->isReadable() ? ESB_SHUTDOWN : ESB_SUCCESS;
}

const char *HttpClientSocket::logAddress() const {
  return _socket.logAddress();
}

ESB::Error HttpClientSocket::abort() {
  assert(_state | RECV_PAUSED);
  assert(!(HAS_BEEN_REMOVED & _state));

  if (!(_state | RECV_PAUSED) || _state | HAS_BEEN_REMOVED) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] local close during paused response body read",
                _socket.logAddress());
  _state &= ~RECV_PAUSED;

  _multiplexer.multiplexer().removeMultiplexedSocket(this);

  return ESB_SUCCESS;
}

ESB::Allocator &HttpClientSocket::allocator() {
  assert(_transaction);
  return _transaction->allocator();
}

const HttpRequest &HttpClientSocket::request() const {
  assert(_transaction);
  return _transaction->request();
}

HttpRequest &HttpClientSocket::request() {
  assert(_transaction);
  return _transaction->request();
}

const HttpResponse &HttpClientSocket::response() const {
  assert(_transaction);
  return _transaction->response();
}

HttpResponse &HttpClientSocket::response() {
  assert(_transaction);
  return _transaction->response();
}

void HttpClientSocket::setContext(void *context) {
  assert(_transaction);
  _transaction->setContext(context);
}

void *HttpClientSocket::context() {
  assert(_transaction);
  return _transaction->context();
}

const void *HttpClientSocket::context() const {
  assert(_transaction);
  return _transaction->context();
}

const ESB::SocketAddress &HttpClientSocket::peerAddress() const {
  return _socket.peerAddress();
}

}  // namespace ES
