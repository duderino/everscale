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
#define ABORTED (1 << 14)

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

bool HttpClientSocket::wantConnect() { return (_state & CONNECTING) != 0; }

bool HttpClientSocket::wantRead() {
  if (_state & (RECV_PAUSED | ABORTED | HAS_BEEN_REMOVED)) {
    return false;
  }

  if (_state & (PARSING_HEADERS | PARSING_BODY)) {
    return true;
  }

  return false;
}

bool HttpClientSocket::wantWrite() {
  if (_state & (SEND_PAUSED | ABORTED | HAS_BEEN_REMOVED)) {
    return false;
  }

  if (_state & (TRANSACTION_BEGIN | FORMATTING_HEADERS | FLUSHING_HEADERS |
                FORMATTING_BODY | FLUSHING_BODY)) {
    return true;
  }

  return false;
}

bool HttpClientSocket::isIdle() {
  // TODO - implement idle timeout
  return false;
}

ESB::Error HttpClientSocket::handleAccept() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_ERROR("[%d] Cannot handle accept events", _socket.socketDescriptor());
  return ESB_SUCCESS;  // keep in multiplexer
}

bool HttpClientSocket::handleConnect() {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  assert(_socket.isConnected());
  assert(_state & CONNECTING);

  ESB_LOG_INFO("[%s] Connected to peer", _socket.logAddress());

  _state &= ~CONNECTING;
  _state |= TRANSACTION_BEGIN;

  return handleWritable();
}

ESB::Error HttpClientSocket::currentChunkBytesAvailable(
    ESB::UInt32 *bytesAvailable, ESB::UInt32 *bufferOffset) {
  switch (ESB::Error error = _transaction->getParser()->parseBody(
              _recvBuffer, bufferOffset, bytesAvailable)) {
    case ESB_SUCCESS:
      ESB_LOG_DEBUG("[%s] %u response bytes available in current chunk",
                    _socket.logAddress(), *bytesAvailable);
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] insufficient bytes available in current chunk",
                    _socket.logAddress());
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] error parsing response body",
                          _socket.logAddress());
      return error;  // remove from multiplexer
  }
}

ESB::Error HttpClientSocket::responseBodyAvailable(ESB::UInt32 *bytesAvailable,
                                                   ESB::UInt32 *bufferOffset) {
  if (!bytesAvailable || !bufferOffset) {
    return ESB_NULL_POINTER;
  }

  switch (ESB::Error error =
              currentChunkBytesAvailable(bytesAvailable, bufferOffset)) {
    case ESB_AGAIN:
      if (ESB_SUCCESS != (error = fillReceiveBuffer())) {
        return error;
      }
      assert(_recvBuffer->isReadable());
      return currentChunkBytesAvailable(bytesAvailable, bufferOffset);
    default:
      return error;
  }
}

ESB::Error HttpClientSocket::readResponseBody(unsigned char *chunk,
                                              ESB::UInt32 bytesRequested,
                                              ESB::UInt32 bufferOffset) {
  assert(chunk);
  assert(bytesRequested <= _recvBuffer->readable());

  if (!chunk) {
    return ESB_NULL_POINTER;
  }

  if (bytesRequested > _recvBuffer->readable()) {
    return ESB_INVALID_ARGUMENT;
  }

  memcpy(chunk, _recvBuffer->buffer() + bufferOffset, bytesRequested);

  ESB::Error error =
      _transaction->getParser()->consumeBody(_recvBuffer, bytesRequested);
  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot consume %u response chunk bytes",
                        _socket.logAddress(), bytesRequested);
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[%s] consumed %u response chunk bytes", _socket.logAddress(),
                bytesRequested);

  return ESB_SUCCESS;
}

bool HttpClientSocket::handleReadable() {
  // returning true will keep the socket in the multiplexer
  // returning false will remove the socket from the multiplexer and ultimately
  // close it.

  assert(wantRead());
  assert(_socket.isConnected());
  assert(_transaction);

  if (!_recvBuffer) {
    _recvBuffer = _multiplexer.acquireBuffer();
    if (!_recvBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer",
                          _socket.logAddress());
      return false;  // remove from multiplexer
    }
  }

  ESB::Error error = ESB_SUCCESS;

  while (!_multiplexer.shutdown()) {
    switch (error = fillReceiveBuffer()) {
      case ESB_SUCCESS:
        break;
      case ESB_AGAIN:
        return true;  // keep in multiplexer
      case ESB_CLOSED:
        return handleRemoteClose();
      default:
        return handleError(error);
    }

    if (PARSING_HEADERS & _state) {
      error = parseResponseHeaders();

      if (ESB_AGAIN == error) {
        continue;  // read more data and repeat parse
      }

      if (ESB_SUCCESS != error) {
        return false;  // remove from multiplexer
      }

      switch (error = _handler.receiveResponseHeaders(_multiplexer, *this)) {
        case ESB_SUCCESS:
          break;
        case ESB_AGAIN:
        case ESB_PAUSE:
          error = pauseRecv(false);
          if (ESB_SUCCESS != error) {
            ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause client receive",
                                _socket.logAddress());
            return false;  // remove from multiplexer
          }
          return true;  // keep in multiplexer but remove from read interest set
        default:
          ESB_LOG_DEBUG_ERRNO(
              error, "[%s] Client request header handler aborting connection",
              _socket.logAddress());
          return false;  // remove from multiplexer
      }

      if (!_transaction->response().hasBody()) {
        unsigned char byte = 0;
        ESB::UInt32 bytesWritten = 0;

        switch (error = _handler.consumeResponseChunk(
                    _multiplexer, *this, &byte, 0, &bytesWritten)) {
          case ESB_SUCCESS:
            break;
          case ESB_PAUSE:
          case ESB_AGAIN:
            error = pauseRecv(false);
            if (ESB_SUCCESS != error) {
              ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot pause client receive",
                                  _socket.logAddress());
              return false;  // remove from multiplexer
            }
            return true;  // keep in multiplexer but remove from read interest
                          // set
          default:
            ESB_LOG_DEBUG_ERRNO(error,
                                "[%s] client body handler aborted connection",
                                _socket.logAddress());
            return false;  // remove from multiplexer
        }

        assert(!(_state | RECV_PAUSED));
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
      return true;  // keep in multiplexer, but remove from read interest set.
    }

    return false;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown with socket in parse state",
                _socket.logAddress());
  return false;  // remove from multiplexer
}

ESB::Error HttpClientSocket::sendRequestBody(unsigned const char *chunk,
                                             ESB::UInt32 chunkSize,
                                             ESB::UInt32 *bytesConsumed) {
  return ESB_NOT_IMPLEMENTED;
}

bool HttpClientSocket::handleWritable() {
  assert(wantWrite());
  assert(_socket.isConnected());
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
        switch (error = flushSendBuffer()) {
          case ESB_SUCCESS:
            break;
          case ESB_AGAIN:
            return true;  // keep in multiplexer, wait for socket to become
            // writable
          default:
            return false;  // remove from multiplexer
        }
        continue;
      }

      if (ESB_SUCCESS != error) {
        return false;  // remove from multiplexer;
      }
    }

    if (FLUSHING_HEADERS & _state) {
      switch (error = flushSendBuffer()) {
        case ESB_SUCCESS:
          break;
        case ESB_AGAIN:
          return true;  // keep in multiplexer, wait for socket to become
          // writable
        default:
          return false;  // remove from multiplexer
      }

      _state &= ~FLUSHING_HEADERS;

      if (_transaction->request().hasBody()) {
        _state |= FORMATTING_BODY;
      } else {
        _state |= PARSING_HEADERS;

        error = _handler.endRequest(_multiplexer, *this);
        if (ESB_SUCCESS != error) {
          ESB_LOG_DEBUG_ERRNO(error,
                              "[%s] handler aborted transaction on request end",
                              _socket.logAddress());
          return false;  // remove from multiplexer
        }

        return handleReadable();
      }
    }

    if (FORMATTING_BODY & _state) {
      error = formatRequestBody();

      if (ESB_SHUTDOWN == error) {
        continue;
      }

      if (ESB_AGAIN == error) {
        switch (error = flushSendBuffer()) {
          case ESB_SUCCESS:
            break;
          case ESB_AGAIN:
            return true;  // keep in multiplexer, wait for socket to become
            // writable
          default:
            return false;  // remove from multiplexer
        }
        continue;
      }

      if (ESB_PAUSE == error) {
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

    error = flushSendBuffer();

    if (ESB_AGAIN == error) {
      return true;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      return false;  // remove from multiplexer
    }

    _state &= ~FLUSHING_BODY;
    _state |= PARSING_HEADERS;

    error = _handler.endRequest(_multiplexer, *this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error,
                          "[%s] handler aborted transaction on request end",
                          _socket.logAddress());
      return false;  // remove from multiplexer
    }

    return handleReadable();
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown with socket in format state",
                _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleError(ESB::Error error) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] socket error", _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleRemoteClose() {
  // TODO - this may just mean the client closed its half of the socket but is
  // still expecting a response.
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] remote server closed socket", _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleIdle() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] server is idle", _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpClientSocket::handleRemove() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] client socket has been removed", _socket.logAddress());

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

    if (GetReuseConnections() && !(_state & ABORTED)) {
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
    // HttpClientSocketFactory::release() is invoked by the cleanup handler.
    // Sockets that are closed are not returned to the connection pool.
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
    ESB::Error error = ESB_SUCCESS;

    if (ESB_SUCCESS !=
        (error = currentChunkBytesAvailable(&bytesAvailable, &bufferOffset))) {
      return error;
    }

    // if last chunk
    if (0 == bytesAvailable) {
      ESB_LOG_DEBUG("[%s] parsed response body", _socket.logAddress());
      unsigned char byte = 0;
      switch (error = _handler.consumeResponseChunk(_multiplexer, *this, &byte,
                                                    0U, &bytesConsumed)) {
        case ESB_SUCCESS:
          _state &= ~PARSING_BODY;
          _state |= TRANSACTION_END;
          return ESB_SUCCESS;
        case ESB_PAUSE:
        case ESB_AGAIN:
          error = pauseRecv(false);
          if (ESB_SUCCESS != error) {
            ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot pause client receive",
                                _socket.logAddress());
            return error;
          }
          return ESB_PAUSE;
        default:
          ESB_LOG_DEBUG_ERRNO(
              error,
              "[%s] handler aborting connection after last response body chunk",
              _socket.logAddress());
          return error;
      }
    }

    ESB_LOG_DEBUG("[%s] offering response chunk of size %u",
                  _socket.logAddress(), bytesAvailable);

    switch (error = _handler.consumeResponseChunk(
                _multiplexer, *this, _recvBuffer->buffer() + bufferOffset,
                bytesAvailable, &bytesConsumed)) {
      case ESB_SUCCESS:
        if (0 == bytesConsumed) {
          ESB_LOG_DEBUG(
              "[%s] pausing response body receive until handler is ready",
              _socket.logAddress());
          error = pauseRecv(false);
          if (ESB_SUCCESS != error) {
            ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot pause client receive",
                                _socket.logAddress());
            return error;
          }
          return ESB_PAUSE;
        }
        break;
      case ESB_AGAIN:
      case ESB_PAUSE:
        error = pauseRecv(false);
        if (ESB_SUCCESS != error) {
          ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot pause client receive",
                              _socket.logAddress());
          return error;
        }
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

    switch (error =
                _handler.offerRequestChunk(_multiplexer, *this, &offeredSize)) {
      case ESB_AGAIN:
      case ESB_PAUSE:
        error = pauseSend(false);
        if (ESB_SUCCESS != error) {
          ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot pause client send",
                              _socket.logAddress());
          return error;
        }
        return ESB_PAUSE;
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

    switch (error = _transaction->getFormatter()->beginBlock(
                _sendBuffer, offeredSize, &maxChunkSize)) {
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

    switch (error = _handler.produceRequestChunk(
                _multiplexer, *this,
                _sendBuffer->buffer() + _sendBuffer->writePosition(),
                chunkSize)) {
      case ESB_SUCCESS:
        break;
      case ESB_AGAIN:
      case ESB_PAUSE:
        error = pauseSend(false);
        if (ESB_SUCCESS != error) {
          ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot pause client send",
                              _socket.logAddress());
          return error;
        }
        return ESB_PAUSE;
      default:
        ESB_LOG_INFO_ERRNO(error, "[%s] cannot format request chunk of size %u",
                           _socket.logAddress(), chunkSize);
        return error;  // remove from multiplexer
    }

    _sendBuffer->setWritePosition(_sendBuffer->writePosition() + chunkSize);
    _bodyBytesWritten += chunkSize;
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

ESB::Error HttpClientSocket::fillReceiveBuffer() {
  if (!_recvBuffer || !_transaction) {
    return ESB_INVALID_STATE;
  }

  if (_recvBuffer->isReadable()) {
    return ESB_SUCCESS;
  }

  // If there is no data in the recv buffer, read some more from the socket
  // If there is no space left in the recv buffer, make room if possible
  if (!_recvBuffer->isWritable()) {
    ESB_LOG_DEBUG("[%s] compacting input buffer", _socket.logAddress());
    if (!_recvBuffer->compact()) {
      ESB_LOG_INFO("[%s] parser jammed", _socket.logAddress());
      return ESB_OVERFLOW;
    }
  }

  // And read from the socket
  assert(_recvBuffer->isWritable());
  ESB::SSize result = _socket.receive(_recvBuffer);

  if (0 > result) {
    ESB::Error error = ESB::LastError();
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot refill client recv buffer",
                        _socket.logAddress());
    return error;
  }

  if (0 == result) {
    ESB_LOG_DEBUG("[%s] connection closed during client recv buffer refill",
                  _socket.logAddress());
    return ESB_CLOSED;
  }

  ESB_LOG_DEBUG("[%s] read %ld bytes into client recv buffer",
                _socket.logAddress(), result);
  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::flushSendBuffer() {
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

      ESB::Error error = ESB::LastError();
      assert(ESB_SUCCESS != error);
      ESB_LOG_INFO_ERRNO(error, "[%s] error flushing request output buffer",
                         _socket.logAddress());
      return error;  // remove from multiplexer
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

ESB::Error HttpClientSocket::abort(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] connection aborted", _socket.logAddress());

  _state |= ABORTED;
  if (updateMultiplexer) {
    return _multiplexer.multiplexer().removeMultiplexedSocket(this);
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::pauseRecv(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  assert(!(RECV_PAUSED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED | RECV_PAUSED)) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] pausing client response receive", _socket.logAddress());

  _state |= RECV_PAUSED;
  if (updateMultiplexer) {
    return _multiplexer.multiplexer().updateMultiplexedSocket(this);
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::resumeRecv(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  assert(RECV_PAUSED & _state);
  if (_state & (HAS_BEEN_REMOVED | ABORTED) || !(_state & RECV_PAUSED)) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] resuming client response receive", _socket.logAddress());

  _state &= ~RECV_PAUSED;
  if (updateMultiplexer) {
    return _multiplexer.multiplexer().updateMultiplexedSocket(this);
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::pauseSend(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  assert(!(SEND_PAUSED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED | SEND_PAUSED)) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] pausing client request send", _socket.logAddress());

  _state |= SEND_PAUSED;
  if (updateMultiplexer) {
    return _multiplexer.multiplexer().updateMultiplexedSocket(this);
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::resumeSend(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  assert(SEND_PAUSED & _state);
  if (_state & (HAS_BEEN_REMOVED | ABORTED) || !(_state & SEND_PAUSED)) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] resuming client request send", _socket.logAddress());

  _state &= ~SEND_PAUSED;
  if (updateMultiplexer) {
    return _multiplexer.multiplexer().updateMultiplexedSocket(this);
  }

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
