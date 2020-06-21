#ifndef ES_HTTP_CLIENT_SOCKET_H
#include <ESHttpClientSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ES_HTTP_ALLOCATOR_SIZE
#define ES_HTTP_ALLOCATOR_SIZE 3072
#endif

// TODO add performance counters

#define HAS_BEEN_REMOVED (1 << 0)
#define CONNECTING (1 << 1)
#define TRANSACTION_BEGIN (1 << 2)
#define FORMATTING_HEADERS (1 << 3)
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

HttpClientSocket::HttpClientSocket(HttpClientHandler &handler, HttpMultiplexerExtended &multiplexer,
                                   ESB::SocketAddress &peerAddress, HttpClientCounters &counters,
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
      _socket(multiplexer.multiplexer().name(), "client", peerAddress, false) {}

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

ESB::Error HttpClientSocket::reset(bool reused, HttpClientTransaction *transaction) {
  assert(!_recvBuffer);
  assert(!_sendBuffer);
  assert(!_transaction);

  if (!transaction) {
    return ESB_NULL_POINTER;
  }

  stateTransition(TRANSACTION_BEGIN);
  _state = TRANSACTION_BEGIN;  // clear any other flags

  if (reused) {
    assert(isConnected());
    setFlag(FIRST_USE_AFTER_REUSE);
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

  if (_state & (TRANSACTION_BEGIN | FORMATTING_HEADERS | FORMATTING_BODY | FLUSHING_BODY)) {
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

ESB::Error HttpClientSocket::handleConnect() {
  assert(!(ABORTED & _state));
  assert(_socket.isConnected());

  ESB_LOG_INFO("[%s] Connected to peer", _socket.name());

  stateTransition(TRANSACTION_BEGIN);

  return handleWritable();
}

ESB::Error HttpClientSocket::currentChunkBytesAvailable(ESB::UInt32 *bytesAvailable, ESB::UInt32 *bufferOffset) {
  switch (ESB::Error error = _transaction->getParser()->parseBody(_recvBuffer, bufferOffset, bytesAvailable)) {
    case ESB_SUCCESS:
      ESB_LOG_DEBUG("[%s] %u response bytes available in current chunk", _socket.name(), *bytesAvailable);
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] insufficient bytes available in current chunk", _socket.name());
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] error parsing response body", _socket.name());
      return error;  // remove from multiplexer
  }
}

ESB::Error HttpClientSocket::responseBodyAvailable(ESB::UInt32 *bytesAvailable, ESB::UInt32 *bufferOffset) {
  if (!bytesAvailable || !bufferOffset) {
    return ESB_NULL_POINTER;
  }

  //
  // Since this function isn't invoked by the multiplexer, we have to explicitly
  // adjust the registered interest events here.
  //

  ESB::Error error = currentChunkBytesAvailable(bytesAvailable, bufferOffset);
  if (ESB_AGAIN == error) {
    if (ESB_SUCCESS == (error = fillReceiveBuffer())) {
      assert(_recvBuffer->isReadable());
      error = currentChunkBytesAvailable(bytesAvailable, bufferOffset);
    }
  }

  switch (error) {
    case ESB_SUCCESS:
      if (0U == *bytesAvailable) {
        // Last chunk has been read, finish the transaction
        ESB_LOG_DEBUG("[%s] finished parsing response body", _socket.name());
        stateTransition(TRANSACTION_END);
        if (ESB_SUCCESS != (error = pauseRecv(true))) {
          abort(true);
          return error;
        }
      }
      return ESB_SUCCESS;
    case ESB_AGAIN:
      if (ESB_SUCCESS != (error = resumeRecv(true))) {
        abort(true);
        return error;
      }
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot parse response body", _socket.name());
      abort(true);
      return error;
  }
}

ESB::Error HttpClientSocket::readResponseBody(unsigned char *chunk, ESB::UInt32 bytesRequested,
                                              ESB::UInt32 bufferOffset) {
  if (0U == bytesRequested) {
    // In case the caller calls this after responseBodyAvailable() returns 0
    // for the last chunk.
    return ESB_SUCCESS;
  }

  assert(chunk);
  assert(bytesRequested <= _recvBuffer->readable());

  if (!chunk) {
    return ESB_NULL_POINTER;
  }

  if (bytesRequested > _recvBuffer->readable()) {
    return ESB_INVALID_ARGUMENT;
  }

  memcpy(chunk, _recvBuffer->buffer() + bufferOffset, bytesRequested);

  ESB::Error error = _transaction->getParser()->consumeBody(_recvBuffer, bytesRequested);
  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot consume %u response chunk bytes", _socket.name(), bytesRequested);
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[%s] consumed %u response chunk bytes", _socket.name(), bytesRequested);

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::handleReadable() {
  assert(wantRead());
  assert(_socket.isConnected());
  assert(_transaction);

  if (!wantRead() || !_socket.isConnected() || !_transaction) {
    return ESB_INVALID_STATE;
  }

  return advanceStateMachine(true, false);
}

ESB::Error HttpClientSocket::sendRequestBody(unsigned const char *chunk, ESB::UInt32 bytesOffered,
                                             ESB::UInt32 *bytesConsumed) {
  //
  // Because this isn't invoked by the multiplexer, we have to make explicit
  // pause and resume calls to update this socket's registered interests.
  //
  switch (ESB::Error error = formatRequestBody(chunk, bytesOffered, bytesConsumed)) {
    case ESB_SUCCESS:
      if (0U == bytesOffered) {
        if (ESB_SUCCESS != (error = pauseSend(true))) {
          abort(true);
          return error;
        }
      }
      return ESB_SUCCESS;
    case ESB_AGAIN:
      if (ESB_SUCCESS != (error = resumeSend(true))) {
        abort(true);
        return error;
      }
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot send request body", _socket.name());
      abort(true);
      return error;
  }
}

ESB::Error HttpClientSocket::formatRequestBody(unsigned const char *chunk, ESB::UInt32 bytesOffered,
                                               ESB::UInt32 *bytesConsumed) {
  //
  // Send final chunk
  //

  if (0U == bytesOffered) {
    ESB::Error error = formatEndBody();
    switch (error) {
      case ESB_AGAIN:
        if (ESB_SUCCESS != (error = flushSendBuffer())) {
          return error;
        }
        if (ESB_SUCCESS != (error = formatEndBody())) {
          return error;
        }
        break;
      case ESB_SUCCESS:
        break;
      default:
        return error;
    }

    // TODO can this block be shared with the reactive sequence?
    error = flushSendBuffer();
    if (ESB_SUCCESS != error) {
      return error;
    }

    stateTransition(PARSING_HEADERS);

    error = _handler.endRequest(_multiplexer, *this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborted transaction on request end", _socket.name());
      return error;
    }
    return ESB_SUCCESS;
  }

  //
  // Send non-final chunk
  //

  assert(chunk);
  assert(bytesConsumed);

  if (!chunk || !bytesConsumed) {
    return ESB_NULL_POINTER;
  }

  ESB::UInt32 bytesSent = 0U;

  while (bytesSent < bytesOffered) {
    if (_multiplexer.shutdown()) {
      return ESB_SHUTDOWN;
    }

    ESB::UInt32 maxChunkSize = 0U;
    ESB::Error error = formatStartChunk(bytesOffered - bytesSent, &maxChunkSize);

    switch (error) {
      case ESB_SUCCESS:
        break;
      case ESB_AGAIN:
        if (ESB_SUCCESS != (error = flushSendBuffer())) {
          return error;
        }
        if (ESB_SUCCESS != (error = formatStartChunk(bytesOffered - bytesSent, &maxChunkSize))) {
          return error;
        }
        break;
      default:
        return error;
    }

    ESB::UInt32 chunkSize = ESB_MIN(bytesOffered - bytesSent, maxChunkSize);

    // consume the chunk

    memcpy(_sendBuffer->buffer() + _sendBuffer->writePosition(), chunk + bytesSent, chunkSize);
    _sendBuffer->setWritePosition(_sendBuffer->writePosition() + chunkSize);
    _bodyBytesWritten += chunkSize;
    bytesSent += chunkSize;
    *bytesConsumed = bytesSent;

    ESB_LOG_DEBUG("[%s] formatted request chunk of size %u", _socket.name(), chunkSize);

    // beginBlock reserves space for this operation, it should never fail

    if (ESB_SUCCESS != (error = formatEndChunk())) {
      return error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::handleWritable() {
  assert(wantWrite());
  assert(_socket.isConnected());
  assert(_transaction);

  if (!wantWrite() || !_socket.isConnected() || !_transaction) {
    return ESB_INVALID_STATE;
  }

  if (!_sendBuffer) {
    _sendBuffer = _multiplexer.acquireBuffer();
    if (!_sendBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer", _socket.name());
      return ESB_OUT_OF_MEMORY;
    }
  }

  ESB::Error error = ESB_SUCCESS;

  if (_state & TRANSACTION_BEGIN) {
    // TODO make connection reuse more configurable
    if (!HttpClientSocket::GetReuseConnections() && !_transaction->request().findHeader("Connection")) {
      error = _transaction->request().addHeader("Connection", "close", _transaction->allocator());

      if (ESB_SUCCESS != error) {
        ESB_LOG_ERROR_ERRNO(error, "[%s] cannot add connection: close header", _socket.name());
        return ESB_AGAIN == error ? ESB_OTHER_ERROR : error;
      }
    }

    // TODO add user agent, etc headers

    stateTransition(FORMATTING_HEADERS);
  }

  while (!_multiplexer.shutdown()) {
    if (FORMATTING_HEADERS & _state) {
      error = stateSendRequestHeaders();

      if (unlikely(ESB_AGAIN == error)) {
        switch (error = flushSendBuffer()) {
          case ESB_SUCCESS:
            break;
          case ESB_AGAIN:
            return ESB_AGAIN;
          default:
            return error;
        }
        continue;
      }

      if (unlikely(ESB_SUCCESS != error)) {
        return error;
      }
    }

    if (FORMATTING_BODY & _state) {
      error = stateSendRequestBody();

      if (ESB_SHUTDOWN == error) {
        continue;
      }

      if (ESB_AGAIN == error) {
        switch (error = flushSendBuffer()) {
          case ESB_SUCCESS:
            break;
          case ESB_AGAIN:
            return ESB_AGAIN;
          default:
            return error;
        }
        continue;
      }

      if (ESB_PAUSE == error) {
        return ESB_AGAIN;
      }

      if (ESB_SUCCESS != error) {
        return error;
      }

      if (FORMATTING_BODY & _state) {
        continue;
      }
    }

    assert(FLUSHING_BODY & _state);

    switch (error = flushSendBuffer()) {
      case ESB_SUCCESS:
        break;
      case ESB_AGAIN:
        return ESB_AGAIN;
      default:
        return error;
    }

    stateTransition(PARSING_HEADERS);

    error = _handler.endRequest(_multiplexer, *this);

    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborted transaction on request end", _socket.name());
      return error;
    }

    return handleReadable();
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown with socket in format state", _socket.name());
  return ESB_SHUTDOWN;
}

ESB::Error HttpClientSocket::stateEndTransaction() { return ESB_NOT_IMPLEMENTED; }

void HttpClientSocket::handleError(ESB::Error error) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] socket error", _socket.name());
}

void HttpClientSocket::handleRemoteClose() {
  // TODO - this may just mean the client closed its half of the socket but is
  // still expecting a response.
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] remote server closed socket", _socket.name());
}

void HttpClientSocket::handleIdle() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] server is idle", _socket.name());
}

bool HttpClientSocket::handleRemove() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] client socket has been removed", _socket.name());

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
    _counters.getFailures()->record(_transaction->startTime(), ESB::Date::Now());

    _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_BEGIN);
  } else if (_state & CONNECTING) {
    assert(_transaction);
    _counters.getFailures()->record(_transaction->startTime(), ESB::Date::Now());

    _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
  } else if (_state & FORMATTING_HEADERS) {
    assert(_transaction);
    _counters.getFailures()->record(_transaction->startTime(), ESB::Date::Now());

    _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS);
  } else if (_state & (FORMATTING_BODY | FLUSHING_BODY)) {
    assert(_transaction);
    _counters.getFailures()->record(_transaction->startTime(), ESB::Date::Now());
    _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY);
  } else if (_state & PARSING_HEADERS) {
    assert(_transaction);
    _counters.getFailures()->record(_transaction->startTime(), ESB::Date::Now());
    _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS);
  } else if (_state & PARSING_BODY) {
    assert(_transaction);
    _counters.getFailures()->record(_transaction->startTime(), ESB::Date::Now());
    _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY);
  } else if (_state & TRANSACTION_END) {
    assert(_transaction);
    _counters.getSuccesses()->record(_transaction->startTime(), ESB::Date::Now());

    if (GetReuseConnections() && !(_state & ABORTED)) {
      const HttpHeader *header = _transaction->response().findHeader("Connection");

      if (header && header->fieldValue() && !strcasecmp("close", (const char *)header->fieldValue())) {
        reuseConnection = false;
      } else {
        reuseConnection = true;
      }
    } else {
      reuseConnection = false;
    }

    _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_END);
  } else if (_state & RETRY_STALE_CONNECTION) {
    assert(_transaction);
    ESB_LOG_DEBUG("[%s] connection stale, retrying transaction", _socket.name());
    ESB::Error error = _multiplexer.executeClientTransaction(_transaction);

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] Cannot retry transaction", _socket.name());
      unsetFlag(RETRY_STALE_CONNECTION);
      stateTransition(TRANSACTION_BEGIN);

      _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_BEGIN);
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
    ESB_LOG_DEBUG("[%s] connection will not be reused", _socket.name());
    _socket.close();
  } else {
    ESB_LOG_DEBUG("[%s] adding connection to connection pool", _socket.name());
  }

  _transaction = NULL;
  stateTransition(HAS_BEEN_REMOVED);

  return true;  // call cleanup handler on us after this returns
}

SOCKET HttpClientSocket::socketDescriptor() const { return _socket.socketDescriptor(); }

ESB::CleanupHandler *HttpClientSocket::cleanupHandler() { return &_cleanupHandler; }

ESB::Error HttpClientSocket::advanceStateMachine(bool fillRecvBuffer, bool drainSendBuffer) {
  // TODO create statemachine function that combines handle read and handle write.
  // TODO pass in fillReceiveBuffer = true if handle read, else false
  // TODO pass in flushSendBuffer = true if handle write, else false
  // TODO pass in adaptor that either calls handler or delegates to application supplied buffers
  // TODO call state machine from handle read, handle write, sync read, and sync write

  if (!_recvBuffer) {
    _recvBuffer = _multiplexer.acquireBuffer();
    if (!_recvBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer", _socket.name());
      return ESB_OUT_OF_MEMORY;  // remove from multiplexer
    }
  }

  while (!_multiplexer.shutdown()) {
    ESB::Error error = ESB_OTHER_ERROR;

    if (fillRecvBuffer) {
      switch (error = fillReceiveBuffer()) {
        case ESB_SUCCESS:
          fillRecvBuffer = false;
          break;
        case ESB_AGAIN:
          return ESB_AGAIN;
        case ESB_CLOSED:
          handleRemoteClose();
          return ESB_CLOSED;
        default:
          handleError(error);
          return error;
      }
    }

    switch (_state & stateMask()) {
      case PARSING_HEADERS:
        error = stateReceiveResponseHeaders();
        break;
      case PARSING_BODY:
        error = stateReceiveResponseBody();
        break;
      default:
        assert(!"invalid state");
        ESB_LOG_WARNING_ERRNO(ESB_INVALID_STATE, "[%s] invalid transition to state %d", _socket.name(),
                              _state & stateMask());
        return ESB_INVALID_STATE;
    }

    switch (error) {
      case ESB_SUCCESS:
        if (TRANSACTION_END & _state) {
          return ESB_SUCCESS;
        }
        break;
      case ESB_AGAIN:
        fillRecvBuffer = true;
        break;
      case ESB_PAUSE:
        if (ESB_SUCCESS != (error = pauseRecv(false))) {
          return ESB_AGAIN == error ? ESB_OTHER_ERROR : error;
        }
        return ESB_AGAIN;
      default:
        return error;
    }
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown", _socket.name());
  return ESB_SHUTDOWN;
}

ESB::Error HttpClientSocket::stateBeginTransaction() { return ESB_NOT_IMPLEMENTED; }

ESB::Error HttpClientSocket::stateSendRequestHeaders() {
  ESB::Error error = _transaction->getFormatter()->formatHeaders(_sendBuffer, _transaction->request());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] partially formatted request headers", _socket.name());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error formatting request header", _socket.name());
    return error;
  }

  stateTransition(FORMATTING_BODY);

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::stateSendRequestBody() {
  assert(_transaction);

  while (!_multiplexer.shutdown()) {
    ESB::UInt32 maxChunkSize = 0;
    ESB::UInt32 offeredSize = 0;

    ESB::Error error = _handler.offerRequestBody(_multiplexer, *this, &offeredSize);
    switch (error) {
      case ESB_AGAIN:
      case ESB_PAUSE:
        if (ESB_SUCCESS != (error = pauseSend(false))) {
          return error;
        }
        return ESB_PAUSE;
      case ESB_SUCCESS:
        ESB_LOG_DEBUG("[%s] handler offers request chunk of %u bytes", _socket.name(), offeredSize);
        break;
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler error offering request chunk", _socket.name());
        return error;  // remove from multiplexer
    }

    if (0 == offeredSize) {
      return formatEndBody();
    }

    if (ESB_SUCCESS != (error = formatStartChunk(offeredSize, &maxChunkSize))) {
      return error;
    }

    ESB::UInt32 chunkSize = ESB_MIN(offeredSize, maxChunkSize);

    // let the handler consume chunkSize bytes of body data

    switch (error = _handler.produceRequestBody(_multiplexer, *this,
                                                _sendBuffer->buffer() + _sendBuffer->writePosition(), chunkSize)) {
      case ESB_SUCCESS:
        break;
      case ESB_AGAIN:
      case ESB_PAUSE:
        error = pauseSend(false);
        if (ESB_SUCCESS != error) {
          return error;
        }
        return ESB_PAUSE;
      default:
        ESB_LOG_INFO_ERRNO(error, "[%s] cannot format request chunk of size %u", _socket.name(), chunkSize);
        return error;  // remove from multiplexer
    }

    _sendBuffer->setWritePosition(_sendBuffer->writePosition() + chunkSize);
    _bodyBytesWritten += chunkSize;
    ESB_LOG_DEBUG("[%s] formatted request chunk of size %u", _socket.name(), chunkSize);

    // beginBlock reserves space for this operation, it should never fail
    if (ESB_SUCCESS != (error = formatEndChunk())) {
      return error;
    }
  }

  return ESB_SHUTDOWN;
}

ESB::Error HttpClientSocket::stateFlushRequestBody() { return ESB_NOT_IMPLEMENTED; }

ESB::Error HttpClientSocket::stateReceiveResponseHeaders() {
  ESB::Error error = _transaction->getParser()->parseHeaders(_recvBuffer, _transaction->response());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] need more response header data from stream", _socket.name());
    if (!_recvBuffer->compact()) {
      ESB_LOG_INFO("[%s] cannot parse response headers: parser jammed", _socket.name());
      return ESB_OVERFLOW;  // remove from multiplexer
    }
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot response parse headers", _socket.name());
    return error;  // remove from multiplexer
  }

  // parse complete

  if (ESB_DEBUG_LOGGABLE) {
    ESB_LOG_DEBUG("[%s] status line: HTTP/%d.%d %d %s", _socket.name(), _transaction->response().httpVersion() / 100,
                  _transaction->response().httpVersion() % 100 / 10, _transaction->response().statusCode(),
                  ESB_SAFE_STR(_transaction->response().reasonPhrase()));
    HttpHeader *header = (HttpHeader *)_transaction->response().headers().first();
    for (; header; header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("[%s] response header: %s: %s", _socket.name(), ESB_SAFE_STR(header->fieldName()),
                    ESB_SAFE_STR(header->fieldValue()));
    }
  }

  if (_transaction->response().hasBody()) {
    stateTransition(PARSING_BODY);
  } else {
    stateTransition(TRANSACTION_END);
  }

  switch (error = _handler.receiveResponseHeaders(_multiplexer, *this)) {
    case ESB_SUCCESS:
      break;
    case ESB_AGAIN:
    case ESB_PAUSE:
      return ESB_PAUSE;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] Client request header handler aborting connection", _socket.name());
      return error;  // remove from multiplexer
  }

  if (_transaction->response().hasBody()) {
    return ESB_SUCCESS;
  }

  unsigned char byte = 0;
  ESB::UInt32 bytesWritten = 0;

  switch (error = _handler.consumeResponseBody(_multiplexer, *this, &byte, 0, &bytesWritten)) {
    case ESB_SUCCESS:
      break;
    case ESB_PAUSE:
    case ESB_AGAIN:
      return ESB_PAUSE;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] client body handler aborted connection", _socket.name());
      return error;  // remove from multiplexer
  }

  assert(!(_state | RECV_PAUSED));
  return ESB_CLEANUP;  // remove from multiplexer
}

ESB::Error HttpClientSocket::stateReceiveResponseBody() {
  assert(_transaction);
  assert(_state & PARSING_BODY);
  assert(_transaction->response().hasBody());

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

    if (ESB_SUCCESS != (error = currentChunkBytesAvailable(&bytesAvailable, &bufferOffset))) {
      return error;
    }

    // if last chunk
    if (0 == bytesAvailable) {
      stateTransition(TRANSACTION_END);
      ESB_LOG_DEBUG("[%s] parsed response body", _socket.name());
      unsigned char byte = 0;
      switch (error = _handler.consumeResponseBody(_multiplexer, *this, &byte, 0U, &bytesConsumed)) {
        case ESB_SUCCESS:
          return ESB_SUCCESS;
        case ESB_PAUSE:
        case ESB_AGAIN:
          return ESB_PAUSE;
        default:
          ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborting connection after last response body chunk", _socket.name());
          return error;
      }
    }

    ESB_LOG_DEBUG("[%s] offering response chunk of size %u", _socket.name(), bytesAvailable);

    switch (error = _handler.consumeResponseBody(_multiplexer, *this, _recvBuffer->buffer() + bufferOffset,
                                                 bytesAvailable, &bytesConsumed)) {
      case ESB_SUCCESS:
        if (0 == bytesConsumed) {
          ESB_LOG_DEBUG("[%s] pausing response body receive until handler is ready", _socket.name());
          return ESB_PAUSE;
        }
        break;
      case ESB_AGAIN:
      case ESB_PAUSE:
        return ESB_PAUSE;
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborting connection before last response body chunk", _socket.name());
        return error;  // remove from multiplexer
    }

    error = _transaction->getParser()->consumeBody(_recvBuffer, bytesConsumed);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot consume %u response chunk bytes", _socket.name(), bytesAvailable);
      return error;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("[%s] consumed %u out of %u response chunk bytes", _socket.name(), bytesConsumed, bytesAvailable);
  }

  return ESB_SHUTDOWN;
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
    ESB_LOG_DEBUG("[%s] compacting input buffer", _socket.name());
    if (!_recvBuffer->compact()) {
      ESB_LOG_INFO("[%s] parser jammed", _socket.name());
      return ESB_OVERFLOW;
    }
  }

  // And read from the socket
  assert(_recvBuffer->isWritable());
  ESB::SSize result = _socket.receive(_recvBuffer);

  if (0 > result) {
    ESB::Error error = ESB::LastError();
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot refill client recv buffer", _socket.name());
    return error;
  }

  if (0 == result) {
    ESB_LOG_DEBUG("[%s] connection closed during client recv buffer refill", _socket.name());
    return ESB_CLOSED;
  }

  ESB_LOG_DEBUG("[%s] read %ld bytes into client recv buffer", _socket.name(), result);
  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::flushSendBuffer() {
  ESB_LOG_DEBUG("[%s] flushing request output buffer", _socket.name());

  if (!_sendBuffer->isReadable()) {
    ESB_LOG_INFO("[%s] request formatter jammed", _socket.name());
    return ESB_OVERFLOW;  // remove from multiplexer
  }

  while (!_multiplexer.shutdown() && _sendBuffer->isReadable()) {
    ESB::SSize bytesSent = _socket.send(_sendBuffer);

    if (0 > bytesSent) {
      if (ESB_AGAIN == bytesSent) {
        ESB_LOG_DEBUG("[%s] would block flushing request output buffer", _socket.name());
        return ESB_AGAIN;  // keep in multiplexer
      }

      if (FIRST_USE_AFTER_REUSE & _state) {
        unsetFlag(FIRST_USE_AFTER_REUSE);
        setFlag(RETRY_STALE_CONNECTION);
      }

      ESB::Error error = ESB::LastError();
      assert(ESB_SUCCESS != error);
      ESB_LOG_INFO_ERRNO(error, "[%s] error flushing request output buffer", _socket.name());
      return error;  // remove from multiplexer
    }

    unsetFlag(FIRST_USE_AFTER_REUSE);
    ESB_LOG_DEBUG("[%s] flushed %ld bytes from request output buffer", _socket.name(), bytesSent);
  }

  return _sendBuffer->isReadable() ? ESB_SHUTDOWN : ESB_SUCCESS;
}

const char *HttpClientSocket::logAddress() const { return _socket.name(); }

ESB::Error HttpClientSocket::abort(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] client connection aborted", _socket.name());

  setFlag(ABORTED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().removeMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot abort client connection", _socket.name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::pauseRecv(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (_state & RECV_PAUSED) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] pausing client response receive", _socket.name());

  setFlag(RECV_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause client response receive", _socket.name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
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

  ESB_LOG_DEBUG("[%s] resuming client response receive", _socket.name());

  unsetFlag(RECV_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume client response receive", _socket.name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::pauseSend(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (_state & SEND_PAUSED) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] pausing client request send", _socket.name());

  setFlag(SEND_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause client response send", _socket.name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
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

  ESB_LOG_DEBUG("[%s] resuming client request send", _socket.name());

  unsetFlag(SEND_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume client response send", _socket.name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
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

const ESB::SocketAddress &HttpClientSocket::peerAddress() const { return _socket.peerAddress(); }

ESB::Error HttpClientSocket::formatStartChunk(ESB::UInt32 chunkSize, ESB::UInt32 *maxChunkSize) {
  ESB::Error error = _transaction->getFormatter()->beginBlock(_sendBuffer, chunkSize, maxChunkSize);
  switch (error) {
    case ESB_SUCCESS:
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] insufficient send buffer space to begin chunk", _socket.name());
      return ESB_AGAIN;
    default:
      ESB_LOG_INFO_ERRNO(error, "[%s] error formatting request body", _socket.name());
      return error;
  }
}

ESB::Error HttpClientSocket::formatEndChunk() {
  ESB::Error error = _transaction->getFormatter()->endBlock(_sendBuffer);

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot format request chunk end block", _socket.name());
    return error;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::formatEndBody() {
  ESB::Error error = _transaction->getFormatter()->endBody(_sendBuffer);

  switch (error) {
    case ESB_SUCCESS:
      stateTransition(FLUSHING_BODY);
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] insufficient space in send buffer to format end body", _socket.name());
      return ESB_AGAIN;
    default:
      ESB_LOG_INFO_ERRNO(error, "[%s] error formatting last request chunk", _socket.name());
      return error;
  }
}

const char *HttpClientSocket::name() const { return _socket.name(); }

void HttpClientSocket::setFlag(int flag) {
  assert(flag & flagMask());
  assert(!(flag & stateMask()));
  _state |= flag;
}

void HttpClientSocket::unsetFlag(int flag) {
  assert(flag & flagMask());
  assert(!(flag & stateMask()));
  _state &= ~flag;
}

void HttpClientSocket::stateTransition(int state) {
  assert(state & stateMask());
  assert(!(state & flagMask()));
#ifndef NDEBUG
  int currentState = _state & stateMask();
#endif

  switch (state) {
    case HAS_BEEN_REMOVED:
      assert(!(currentState & HAS_BEEN_REMOVED));
      _state = HAS_BEEN_REMOVED;
      ESB_LOG_DEBUG("[%s] connection removed", _socket.name());
      break;
    case CONNECTING:
      assert(0 == "Cannot transition into state CONNECTING");
      break;
    case TRANSACTION_BEGIN:
      assert(currentState & (CONNECTING | HAS_BEEN_REMOVED));
      _state &= ~(CONNECTING | HAS_BEEN_REMOVED);
      _state |= TRANSACTION_BEGIN;
      ESB_LOG_DEBUG("[%s] transaction begun", _socket.name());
      break;
    case FORMATTING_HEADERS:
      assert(currentState & TRANSACTION_BEGIN);
      _state &= ~TRANSACTION_BEGIN;
      _state |= FORMATTING_HEADERS;
      ESB_LOG_DEBUG("[%s] formatting headers", _socket.name());
      break;
    case FORMATTING_BODY:
      assert(currentState & FORMATTING_HEADERS);
      _state &= ~FORMATTING_HEADERS;
      _state |= FORMATTING_BODY;
      ESB_LOG_DEBUG("[%s] formatting body", _socket.name());
      break;
    case FLUSHING_BODY:
      assert(currentState & FORMATTING_BODY);
      _state &= ~FORMATTING_BODY;
      _state |= FLUSHING_BODY;
      ESB_LOG_DEBUG("[%s] flushing body", _socket.name());
      break;
    case PARSING_HEADERS:
      assert(currentState & FLUSHING_BODY);
      _state &= ~FLUSHING_BODY;
      _state |= PARSING_HEADERS;
      ESB_LOG_DEBUG("[%s] parsing headers", _socket.name());
      break;
    case PARSING_BODY:
      assert(currentState & PARSING_HEADERS);
      _state &= ~PARSING_HEADERS;
      _state |= PARSING_BODY;
      ESB_LOG_DEBUG("[%s] parsing body", _socket.name());
      break;
    case TRANSACTION_END:
      assert(currentState & (PARSING_HEADERS | PARSING_BODY));
      _state &= ~(PARSING_HEADERS | PARSING_BODY);
      _state |= TRANSACTION_END;
      ESB_LOG_DEBUG("[%s] transaction complete", _socket.name());
      break;
    default:
      assert(0 == "Cannot transition into unknown state");
  }
}

int HttpClientSocket::stateMask() {
  return (HAS_BEEN_REMOVED | CONNECTING | TRANSACTION_BEGIN | FORMATTING_HEADERS | FORMATTING_BODY | FLUSHING_BODY |
          PARSING_HEADERS | PARSING_BODY | TRANSACTION_END);
}

int HttpClientSocket::flagMask() {
  return ~(HAS_BEEN_REMOVED | CONNECTING | TRANSACTION_BEGIN | FORMATTING_HEADERS | FORMATTING_BODY | FLUSHING_BODY |
           PARSING_HEADERS | PARSING_BODY | TRANSACTION_END);
}

}  // namespace ES
