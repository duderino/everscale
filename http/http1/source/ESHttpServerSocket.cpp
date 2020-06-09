#ifndef ES_HTTP_SERVER_SOCKET_H
#include <ESHttpServerSocket.h>
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

namespace ES {

// TODO add performance counters

#define HAS_BEEN_REMOVED (1 << 0)
#define PARSING_HEADERS (1 << 1)
#define PARSING_BODY (1 << 2)
#define SKIPPING_TRAILER (1 << 3)
#define FORMATTING_HEADERS (1 << 4)
#define FLUSHING_HEADERS (1 << 5)
#define FORMATTING_BODY (1 << 6)
#define FLUSHING_BODY (1 << 7)
#define TRANSACTION_BEGIN (1 << 8)
#define TRANSACTION_END (1 << 9)
#define CANNOT_REUSE_CONNECTION (1 << 10)
#define RECV_PAUSED (1 << 11)
#define SEND_PAUSED (1 << 12)
#define ABORTED (1 << 13)

// TODO - max requests per connection option (1 disables keepalives)
// TODO - max header size option
// TODO - max body size option

static bool CloseAfterErrorResponse = true;

HttpServerSocket::HttpServerSocket(HttpServerHandler &handler, HttpMultiplexerExtended &multiplexer,
                                   HttpServerCounters &counters, ESB::CleanupHandler &cleanupHandler)
    : _state(TRANSACTION_BEGIN),
      _bodyBytesWritten(0),
      _requestsPerConnection(0),
      _multiplexer(multiplexer),
      _handler(handler),
      _transaction(NULL),
      _counters(counters),
      _cleanupHandler(cleanupHandler),
      _recvBuffer(NULL),
      _sendBuffer(NULL),
      _socket() {}

HttpServerSocket::~HttpServerSocket() {
  if (_recvBuffer) {
    _multiplexer.releaseBuffer(_recvBuffer);
    _recvBuffer = NULL;
  }
  if (_sendBuffer) {
    _multiplexer.releaseBuffer(_sendBuffer);
    _sendBuffer = NULL;
  }
  if (_transaction) {
    _multiplexer.destroyServerTransaction(_transaction);
    _transaction = NULL;
  }
}

ESB::Error HttpServerSocket::reset(ESB::TCPSocket::State &state) {
  assert(!_recvBuffer);
  assert(!_sendBuffer);
  assert(!_transaction);

  _state = TRANSACTION_BEGIN;
  _bodyBytesWritten = 0;
  _socket.reset(state);

  return ESB_SUCCESS;
}

bool HttpServerSocket::wantAccept() { return false; }

bool HttpServerSocket::wantConnect() { return false; }

bool HttpServerSocket::wantRead() {
  if (_state & (RECV_PAUSED | ABORTED | HAS_BEEN_REMOVED)) {
    return false;
  }

  if (_state & (TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY | SKIPPING_TRAILER)) {
    return true;
  }

  return false;
}

bool HttpServerSocket::wantWrite() {
  if (_state & (SEND_PAUSED | ABORTED | HAS_BEEN_REMOVED)) {
    return false;
  }

  if (_state & (FORMATTING_HEADERS | FLUSHING_HEADERS | FORMATTING_BODY | FLUSHING_BODY)) {
    return true;
  }

  return false;
}

bool HttpServerSocket::isIdle() {
  // TODO - implement idle timeout
  return false;
}

ESB::Error HttpServerSocket::handleAccept() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_ERROR("[%d] Cannot handle accept events", _socket.socketDescriptor());
  return ESB_INVALID_STATE;  // remove from multiplexer
}

ESB::Error HttpServerSocket::handleConnect() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_ERROR("[%d] Cannot handle connect events", _socket.socketDescriptor());
  return ESB_INVALID_STATE;  // remove from multiplexer
}

ESB::Error HttpServerSocket::currentChunkBytesAvailable(ESB::UInt32 *bytesAvailable, ESB::UInt32 *bufferOffset) {
  switch (ESB::Error error = _transaction->getParser()->parseBody(_recvBuffer, bufferOffset, bytesAvailable)) {
    case ESB_SUCCESS:
      ESB_LOG_DEBUG("[%s] %u request bytes available in current chunk", _socket.logAddress(), *bytesAvailable);
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] insufficient bytes available in current chunk", _socket.logAddress());
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] error parsing request body", _socket.logAddress());
      return error;  // remove from multiplexer
  }
}

ESB::Error HttpServerSocket::requestBodyAvailable(ESB::UInt32 *bytesAvailable, ESB::UInt32 *bufferOffset) {
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
        // Last chunk has been read, send the response
        ESB_LOG_DEBUG("[%s] finished parsing request body", _socket.logAddress());
        _state &= ~PARSING_BODY;
        _state |= FORMATTING_HEADERS;
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
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot parse request body", _socket.logAddress());
      abort(true);
      return error;
  }
}

ESB::Error HttpServerSocket::readRequestBody(unsigned char *chunk, ESB::UInt32 bytesRequested,
                                             ESB::UInt32 bufferOffset) {
  if (0U == bytesRequested) {
    // In case the caller calls this after requestBodyAvailable() returns 0
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
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot consume %u request chunk bytes", _socket.logAddress(), bytesRequested);
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[%s] consumed %u request chunk bytes", _socket.logAddress(), bytesRequested);

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::handleReadable() {
  assert(wantRead());
  assert(_socket.isConnected());

  if (!wantRead() || !_socket.isConnected()) {
    return ESB_INVALID_STATE;
  }

  if (!_recvBuffer) {
    assert(_state & TRANSACTION_BEGIN);
    _recvBuffer = _multiplexer.acquireBuffer();
    if (!_recvBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer", _socket.logAddress());
      return ESB_OUT_OF_MEMORY;  // remove from multiplexer
    }
  }

  if (!_transaction) {
    assert(_state & TRANSACTION_BEGIN);
    _transaction = _multiplexer.createServerTransaction();
    if (!_transaction) {
      _multiplexer.releaseBuffer(_recvBuffer);
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create server trans", _socket.logAddress());
      return ESB_OUT_OF_MEMORY;  // remove from multiplexer
    }
  }

  ESB::Error error = ESB_SUCCESS;

  if (_state & TRANSACTION_BEGIN) {
    // If we don't fully read the request we can't reuse the socket.  Unset
    // this flag once the request has been fully read.
    _state |= CANNOT_REUSE_CONNECTION;
    _transaction->setPeerAddress(_socket.peerAddress());

    switch (error = _handler.beginTransaction(_multiplexer, *this)) {
      case ESB_SUCCESS:
        break;
      case ESB_AGAIN:
        return ESB_AGAIN;  // keep in multiplexer
      case ESB_SEND_RESPONSE:
        return sendResponse();
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborted connection", _socket.logAddress());
        return error;  // remove from multiplexer - immediately close
    }

    _state &= ~TRANSACTION_BEGIN;
    _state |= PARSING_HEADERS;
  }

  while (!_multiplexer.shutdown()) {
    switch (error = fillReceiveBuffer()) {
      case ESB_SUCCESS:
        break;
      case ESB_AGAIN:
        return ESB_AGAIN;  // keep in multiplexer
      case ESB_CLOSED:
        return handleRemoteClose();
      default:
        return handleError(error);
    }

    if (PARSING_HEADERS & _state) {
      error = parseRequestHeaders();

      if (ESB_AGAIN == error) {
        continue;  // read more data and repeat parse
      }

      if (ESB_SUCCESS != error) {
        if (IsHttpError(error)) {
          return sendBadRequestResponse();
        } else {
          return sendInternalServerErrorResponse();
        }
      }

      if (!_transaction->request().hasBody()) {
        _state &= ~CANNOT_REUSE_CONNECTION;
      }

      // TODO - check Expect header and maybe send a 100 Continue

      switch (error = _handler.receiveRequestHeaders(_multiplexer, *this)) {
        case ESB_SUCCESS:
          break;
        case ESB_PAUSE:
        case ESB_AGAIN:
          if (ESB_SUCCESS != (error = pauseRecv(false))) {
            return error;  // remove from multiplexer
          }
          return ESB_SUCCESS;  // keep in multiplexer but remove from read
                               // interest set
        case ESB_SEND_RESPONSE:
          return sendResponse();
        default:
          ESB_LOG_DEBUG_ERRNO(error, "[%s] Server request header handler aborting connection", _socket.logAddress());
          return error;  // remove from multiplexer
      }

      if (!_transaction->request().hasBody()) {
        unsigned char byte = 0;
        ESB::UInt32 bytesWritten = 0;

        switch (error = _handler.consumeRequestBody(_multiplexer, *this, &byte, 0, &bytesWritten)) {
          case ESB_SUCCESS:
          case ESB_SEND_RESPONSE:
            return sendResponse();
          case ESB_PAUSE:
          case ESB_AGAIN:
            if (ESB_SUCCESS != (error = pauseRecv(false))) {
              return error;  // remove from multiplexer
            }
            return ESB_SUCCESS;  // keep in multiplexer but remove from read
                                 // interest set
          default:
            ESB_LOG_DEBUG_ERRNO(error, "[%s] server body handler aborted connection", _socket.logAddress());
            return error;  // remove from multiplexer
        }
      }

      _state &= ~PARSING_HEADERS;
      _state |= PARSING_BODY;
    }

    if (PARSING_BODY & _state) {
      assert(_transaction->request().hasBody());
      error = parseRequestBody();

      if (ESB_AGAIN == error) {
        continue;  // read more data and repeat parse
      }

      if (ESB_SHUTDOWN == error) {
        continue;  // break out of the outer while loop
      }

      if (ESB_PAUSE == error) {
        return ESB_SUCCESS;  // keep in multiplexer but remove from read
                             // interest set
      }

      if (ESB_SUCCESS != error) {
        if (IsHttpError(error)) {
          return sendBadRequestResponse();
        } else {
          return sendInternalServerErrorResponse();
        }
      }

      if (_state & TRANSACTION_END) {
        // server handler decided to abort
        return ESB_OTHER_ERROR;  // remove from multiplexer
      }

      if (_state & FORMATTING_HEADERS) {
        // server handler decided to send response before finishing body
        assert(_state & CANNOT_REUSE_CONNECTION);
        return sendResponse();
      }

      assert(_state & SKIPPING_TRAILER);
    }

    assert(SKIPPING_TRAILER & _state);
    assert(_transaction->request().hasBody());
    error = skipTrailer();

    if (ESB_AGAIN == error) {
      continue;  // read more data and repeat parse
    }

    if (ESB_SUCCESS != error) {
      return error;  // remove from multiplexer
    }

    // Request has been fully read, so we can reuse the connection, but don't
    // start reading the next request until the response has been fully sent.

    _state &= ~CANNOT_REUSE_CONNECTION;
    if (ESB_SUCCESS != (error = pauseRecv(false))) {
      return error;  // remove from multiplexer
    }

    // server handler should have populated the response object in
    // parseRequestBody()
    return sendResponse();
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown with socket in parse state", _socket.logAddress());
  return ESB_SHUTDOWN;  // remove from multiplexer
}

ESB::Error HttpServerSocket::sendEmptyResponse(int statusCode, const char *reasonPhrase) {
  _transaction->response().setStatusCode(statusCode);
  _transaction->response().setReasonPhrase(reasonPhrase);
  _transaction->response().setHasBody(false);
  switch (ESB::Error error = sendResponse()) {
    case ESB_SUCCESS:
      if (ESB_SUCCESS != (error = pauseSend(true))) {
        abort(true);
        return error;
      }
      return ESB_SUCCESS;
    case ESB_AGAIN:
      if (ESB_SUCCESS != (error = resumeSend(true))) {
        abort(true);
        return error;
      }
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot send empty response", _socket.logAddress());
      abort(true);
      return error;
  }
}

ESB::Error HttpServerSocket::sendResponse(const HttpResponse &response) {
  ESB::Error error = _transaction->response().copy(&response, _transaction->allocator());
  if (ESB_SUCCESS != error) {
    abort(true);
    return error;
  }

  switch (error = sendResponse()) {
    case ESB_SUCCESS:
      if (ESB_SUCCESS != (error = pauseSend(true))) {
        abort(true);
        return error;
      }
      return ESB_SUCCESS;
    case ESB_AGAIN:
      if (ESB_SUCCESS != (error = resumeSend(true))) {
        abort(true);
        return error;
      }
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot send empty response", _socket.logAddress());
      abort(true);
      return error;
  }
}

ESB::Error HttpServerSocket::sendResponseBody(unsigned const char *chunk, ESB::UInt32 bytesOffered,
                                              ESB::UInt32 *bytesConsumed) {
  //
  // Because this isn't invoked by the multiplexer, we have to make explicit
  // pause and resume calls to update this socket's registered interests.
  //
  switch (ESB::Error error = formatResponseBody(chunk, bytesOffered, bytesConsumed)) {
    case ESB_SUCCESS:
      if (0U == bytesOffered) {
        if (ESB_SUCCESS != (error = pauseSend(true))) {
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
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot send request body", _socket.logAddress());
      abort(true);
      return error;
  }
}

ESB::Error HttpServerSocket::formatResponseBody(unsigned const char *chunk, ESB::UInt32 bytesOffered,
                                                ESB::UInt32 *bytesConsumed) {
  return ESB_NOT_IMPLEMENTED;
}

ESB::Error HttpServerSocket::handleWritable() {
  assert(wantWrite());
  assert(_socket.isConnected());
  assert(_transaction);

  if (!wantWrite() || !_socket.isConnected() || !_transaction) {
    return ESB_INVALID_STATE;
  }

  if (!_sendBuffer) {
    _sendBuffer = _multiplexer.acquireBuffer();
    if (!_sendBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer", _socket.logAddress());
      return ESB_OUT_OF_MEMORY;  // remove from multiplexer
    }
  }

  ESB::Error error = ESB_SUCCESS;

  while (!_multiplexer.shutdown()) {
    if (FORMATTING_HEADERS & _state) {
      error = formatResponseHeaders();

      if (ESB_AGAIN == error) {
        switch (error = flushSendBuffer()) {
          case ESB_SUCCESS:
            break;
          case ESB_AGAIN:
            return ESB_AGAIN;  // keep in multiplexer, wait for socket to become
            // writable
          default:
            return error;  // remove from multiplexer
        }
        continue;
      }

      if (ESB_SUCCESS != error) {
        return error;  // remove from multiplexer;
      }
    }

    if (FLUSHING_HEADERS & _state) {
      switch (error = flushSendBuffer()) {
        case ESB_SUCCESS:
          break;
        case ESB_AGAIN:
          return ESB_AGAIN;  // keep in multiplexer, wait for socket to become
          // writable
        default:
          return error;  // remove from multiplexer
      }

      _state &= ~FLUSHING_HEADERS;

      if (_transaction->response().hasBody()) {
        _state |= FORMATTING_BODY;
      } else {
        _state |= TRANSACTION_END;
      }
    }

    if (FORMATTING_BODY & _state) {
      error = formatResponseBody();

      if (ESB_SHUTDOWN == error) {
        continue;
      }

      if (ESB_AGAIN == error) {
        switch (error = flushSendBuffer()) {
          case ESB_SUCCESS:
            break;
          case ESB_AGAIN:
            return ESB_AGAIN;  // keep in multiplexer, wait for socket to become
            // writable
          default:
            return error;  // remove from multiplexer
        }
        continue;
      }

      if (ESB_PAUSE == error) {
        return ESB_SUCCESS;  // keep in multiplexer but remove from write
                             // interest set
      }

      if (ESB_SUCCESS != error) {
        return error;  // remove from multiplexer;
      }

      if (FORMATTING_BODY & _state) {
        continue;
      }
    }

    if (FLUSHING_BODY & _state) {
      switch (error = flushSendBuffer()) {
        case ESB_SUCCESS:
          break;
        case ESB_AGAIN:
          return ESB_AGAIN;  // keep in multiplexer, wait for socket to become
          // writable
        default:
          return error;  // remove from multiplexer
      }

      _state &= ~FLUSHING_BODY;
      _state |= TRANSACTION_END;
    }

    assert(_state & TRANSACTION_END);

    ++_requestsPerConnection;
    _handler.endTransaction(_multiplexer, *this, HttpServerHandler::ES_HTTP_SERVER_HANDLER_END);

    if (CANNOT_REUSE_CONNECTION & _state) {
      return ESB_CLEANUP;  // remove from multiplexer
    }

    if (CloseAfterErrorResponse && 300 <= _transaction->response().statusCode()) {
      return ESB_CLEANUP;  // remove from multiplexer
    }

    // TODO - close connection if max requests sent on connection

    if (!_transaction->request().reuseConnection()) {
      return ESB_CLEANUP;  // remove from multiplexer
    }

    // Resume receiving and immediately start processing the next request.

    if (ESB_SUCCESS != (error = resumeRecv(false))) {
      return error;  // remove from multiplexer
    }

    // TODO release buffers in between transactions

    _state = TRANSACTION_BEGIN;
    _bodyBytesWritten = 0;
    _transaction->reset();
    return handleReadable();
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown with socket in format state", _socket.logAddress());
  return ESB_SHUTDOWN;  // remove from multiplexer
}

bool HttpServerSocket::handleError(ESB::Error error) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO_ERRNO(error, "[%s] socket error", _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleRemoteClose() {
  // TODO - this may just mean the client closed its half of the socket but is
  // still expecting a response.
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] remote client closed socket", _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleIdle() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] client is idle", _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleRemove() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] closing server socket", _socket.logAddress());
  _socket.close();

  if (_sendBuffer) {
    _multiplexer.releaseBuffer(_sendBuffer);
    _sendBuffer = NULL;
  }
  if (_recvBuffer) {
    _multiplexer.releaseBuffer(_recvBuffer);
    _recvBuffer = NULL;
  }

  if (_state & PARSING_HEADERS) {
    assert(_transaction);
    _handler.endTransaction(_multiplexer, *this, HttpServerHandler::ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS);
  } else if (_state & PARSING_BODY) {
    assert(_transaction);
    _handler.endTransaction(_multiplexer, *this, HttpServerHandler::ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY);
  } else if (_state & (FORMATTING_HEADERS | FLUSHING_HEADERS)) {
    assert(_transaction);
    _handler.endTransaction(_multiplexer, *this, HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS);
  } else if (_state & (FORMATTING_BODY | FLUSHING_BODY)) {
    assert(_transaction);
    _handler.endTransaction(_multiplexer, *this, HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY);
  }

  if (_transaction) {
    _multiplexer.destroyServerTransaction(_transaction);
    _transaction = NULL;
  }

  _state = HAS_BEEN_REMOVED;
  _counters.getAverageTransactionsPerConnection()->add(_requestsPerConnection);
  _requestsPerConnection = 0;

  return true;  // call cleanup handler on us after this returns
}

SOCKET HttpServerSocket::socketDescriptor() const { return _socket.socketDescriptor(); }

ESB::CleanupHandler *HttpServerSocket::cleanupHandler() { return &_cleanupHandler; }

ESB::Error HttpServerSocket::parseRequestHeaders() {
  assert(_transaction);

  ESB::Error error = _transaction->getParser()->parseHeaders(_recvBuffer, _transaction->request());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] need more request header data from stream", _socket.logAddress());
    if (!_recvBuffer->compact()) {
      ESB_LOG_INFO("[%s] cannot parse request headers: parser jammed", _socket.logAddress());
      return ESB_OVERFLOW;  // remove from multiplexer
    }
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot parse request headers", _socket.logAddress());
    return error;  // remove from multiplexer
  }

  // parse complete

  if (ESB_DEBUG_LOGGABLE) {
    ESB_LOG_DEBUG("[%s] request headers parsed", _socket.logAddress());
    ESB_LOG_DEBUG("[%s] Method: %s", _socket.logAddress(), _transaction->request().method());

    switch (_transaction->request().requestUri().type()) {
      case HttpRequestUri::ES_URI_ASTERISK:
        ESB_LOG_DEBUG("[%s] Asterisk Request-URI", _socket.logAddress());
        break;
      case HttpRequestUri::ES_URI_HTTP:
      case HttpRequestUri::ES_URI_HTTPS:
        ESB_LOG_DEBUG("[%s] scheme: %s", _socket.logAddress(), _transaction->request().requestUri().typeString());
        ESB_LOG_DEBUG("[%s] host: %s", _socket.logAddress(), ESB_SAFE_STR(_transaction->request().requestUri().host()));
        ESB_LOG_DEBUG("[%s] port: %d", _socket.logAddress(), _transaction->request().requestUri().port());
        ESB_LOG_DEBUG("[%s] path: %s", _socket.logAddress(),
                      ESB_SAFE_STR(_transaction->request().requestUri().absPath()));
        ESB_LOG_DEBUG("[%s] query: %s", _socket.logAddress(),
                      ESB_SAFE_STR(_transaction->request().requestUri().query()));
        ESB_LOG_DEBUG("[%s] fragment: %s", _socket.logAddress(),
                      ESB_SAFE_STR(_transaction->request().requestUri().fragment()));
        break;
      case HttpRequestUri::ES_URI_OTHER:
        ESB_LOG_DEBUG("[%s] Other: %s", _socket.logAddress(),
                      ESB_SAFE_STR(_transaction->request().requestUri().other()));
        break;
    }

    ESB_LOG_DEBUG("[%s] Version: HTTP/%d.%d", _socket.logAddress(), _transaction->request().httpVersion() / 100,
                  _transaction->request().httpVersion() % 100 / 10);

    HttpHeader *header = (HttpHeader *)_transaction->request().headers().first();
    for (; header; header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("[%s] %s: %s", _socket.logAddress(), ESB_SAFE_STR(header->fieldName()),
                    ESB_SAFE_STR(header->fieldValue()));
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::parseRequestBody() {
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

    if (ESB_SUCCESS != (error = currentChunkBytesAvailable(&bytesAvailable, &bufferOffset))) {
      return error;
    }

    // if last chunk
    if (0 == bytesAvailable) {
      ESB_LOG_DEBUG("[%s] parsed request body", _socket.logAddress());
      unsigned char byte = 0;
      switch (error = _handler.consumeRequestBody(_multiplexer, *this, &byte, 0U, &bytesConsumed)) {
        case ESB_SUCCESS:
        case ESB_SEND_RESPONSE:
          _state |= SKIPPING_TRAILER;
          _state &= ~PARSING_BODY;
          assert(wantRead());
          return ESB_SUCCESS;
        case ESB_PAUSE:
        case ESB_AGAIN:
          if (ESB_SUCCESS != (error = pauseRecv(false))) {
            return error;
          }
          return ESB_PAUSE;
        default:
          ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborting connection after last request body chunk",
                              _socket.logAddress());
          _state |= TRANSACTION_END;
          return error;
      }
    }

    ESB_LOG_DEBUG("[%s] server socket offering request chunk of size %u", _socket.logAddress(), bytesAvailable);

    switch (error = _handler.consumeRequestBody(_multiplexer, *this, _recvBuffer->buffer() + bufferOffset,
                                                bytesAvailable, &bytesConsumed)) {
      case ESB_SUCCESS:
        if (0 == bytesConsumed) {
          ESB_LOG_DEBUG("[%s] pausing request body receive until handler is ready", _socket.logAddress());
          if (ESB_SUCCESS != (error = pauseRecv(false))) {
            return error;
          }
          return ESB_PAUSE;
        }
        break;
      case ESB_AGAIN:
      case ESB_PAUSE:
        if (ESB_SUCCESS != (error = pauseRecv(false))) {
          return error;
        }
        return ESB_PAUSE;
      case ESB_SEND_RESPONSE:
        ESB_LOG_DEBUG("[%s] handler sending response before last request body chunk", _socket.logAddress());
        _state &= ~PARSING_BODY;
        _state |= FORMATTING_HEADERS;
        return ESB_SUCCESS;
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborting connection before last request body chunk",
                            _socket.logAddress());
        _state &= ~PARSING_BODY;
        _state |= TRANSACTION_END;
        return ESB_SUCCESS;
    }

    error = _transaction->getParser()->consumeBody(_recvBuffer, bytesConsumed);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot consume %u request chunk bytes", _socket.logAddress(), bytesAvailable);
      return error;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("[%s] handler consumed %u out of %u response chunk bytes", _socket.logAddress(), bytesConsumed,
                  bytesAvailable);
  }

  return ESB_SHUTDOWN;
}

ESB::Error HttpServerSocket::skipTrailer() {
  assert(_state & SKIPPING_TRAILER);
  assert(_transaction);

  ESB::Error error = _transaction->getParser()->skipTrailer(_recvBuffer);

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] need more data from stream to skip trailer", _socket.logAddress());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error skipping trailer", _socket.logAddress());
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[%s] request trailer skipped", _socket.logAddress());
  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::formatResponseHeaders() {
  assert(_transaction);

  ESB::Error error = _transaction->getFormatter()->formatHeaders(_sendBuffer, _transaction->response());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] partially formatted response headers", _socket.logAddress());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error formatting response header", _socket.logAddress());
    return error;
  }

  ESB_LOG_DEBUG("[%s] formatted response headers", _socket.logAddress());

  _state &= ~FORMATTING_HEADERS;
  _state |= FORMATTING_BODY;

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::formatResponseBody() {
  assert(_transaction);

  while (!_multiplexer.shutdown()) {
    ESB::UInt32 maxChunkSize = 0;
    ESB::UInt32 offeredSize = 0;

    ESB::Error error = _handler.offerResponseBody(_multiplexer, *this, &offeredSize);
    switch (error) {
      case ESB_AGAIN:
      case ESB_PAUSE:
        if (ESB_SUCCESS != (error = pauseSend(false))) {
          return error;
        }
        return ESB_PAUSE;
      case ESB_SUCCESS:
        ESB_LOG_DEBUG("[%s] handler offers response chunk of %u bytes", _socket.logAddress(), offeredSize);
        break;
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler error offering response chunk", _socket.logAddress());
        return error;  // remove from multiplexer
    }

    if (0 == offeredSize) {
      return formatEndBody();
    }

    if (ESB_SUCCESS != (error = formatStartChunk(offeredSize, &maxChunkSize))) {
      return error;
    }

    ESB::UInt32 chunkSize = ESB_MIN(offeredSize, maxChunkSize);

    // ask the handler to produce chunkSize bytes of body data

    switch (error = _handler.produceResponseBody(_multiplexer, *this,
                                                 _sendBuffer->buffer() + _sendBuffer->writePosition(), chunkSize)) {
      case ESB_SUCCESS:
        break;
      case ESB_AGAIN:
      case ESB_PAUSE:
        if (ESB_SUCCESS != (error = pauseSend(false))) {
          return error;
        }
        return ESB_PAUSE;
      default:
        ESB_LOG_INFO_ERRNO(error, "[%s] cannot format response chunk of size %u", _socket.logAddress(), chunkSize);
        return error;  // remove from multiplexer
    }

    _sendBuffer->setWritePosition(_sendBuffer->writePosition() + chunkSize);
    _bodyBytesWritten += chunkSize;
    ESB_LOG_DEBUG("[%s] formatted response chunk of size %u", _socket.logAddress(), chunkSize);

    // beginBlock reserves space for this operation, it should never fail
    if (ESB_SUCCESS != (error = formatEndChunk())) {
      return error;
    }
  }

  if (_multiplexer.shutdown()) {
    return ESB_SHUTDOWN;
  }

  return ESB_SHUTDOWN;
}

ESB::Error HttpServerSocket::fillReceiveBuffer() {
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
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot refill server recv buffer", _socket.logAddress());
    return error;
  }

  if (0 == result) {
    ESB_LOG_DEBUG("[%s] connection closed during server recv buffer refill", _socket.logAddress());
    return ESB_CLOSED;
  }

  ESB_LOG_DEBUG("[%s] read %ld bytes into server recv buffer", _socket.logAddress(), result);
  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::flushSendBuffer() {
  assert(_transaction);
  assert(_sendBuffer);

  ESB_LOG_DEBUG("[%s] flushing response output buffer", _socket.logAddress());

  if (!_sendBuffer->isReadable()) {
    ESB_LOG_INFO("[%s] response formatter jammed", _socket.logAddress());
    return ESB_OVERFLOW;  // remove from multiplexer
  }

  while (!_multiplexer.shutdown() && _sendBuffer->isReadable()) {
    ESB::SSize bytesSent = _socket.send(_sendBuffer);

    if (0 > bytesSent) {
      if (ESB_AGAIN == bytesSent) {
        ESB_LOG_DEBUG("[%s] would block flushing response output buffer", _socket.logAddress());
        return ESB_AGAIN;  // keep in multiplexer
      }

      ESB::Error error = ESB::LastError();
      assert(ESB_SUCCESS != error);
      ESB_LOG_INFO_ERRNO(error, "[%s] error flushing response output buffer", _socket.logAddress());
      return error;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("[%s] flushed %ld bytes from response output buffer", _socket.logAddress(), bytesSent);
  }

  return _sendBuffer->isReadable() ? ESB_SHUTDOWN : ESB_SUCCESS;
}

ESB::Error HttpServerSocket::sendResponse() {
  if (0 == _transaction->response().statusCode()) {
    ESB_LOG_INFO(
        "[%s] server handler failed to build response, sending 500 Internal "
        "Server Error",
        _socket.logAddress());
    return sendInternalServerErrorResponse();
  }

  // TODO strip Transfer-Encoding, Content-Length, & Connection headers from the
  // response object
  // TODO add date header and any other  headers like that

  ESB::Error error = _transaction->response().addHeader("Transfer-Encoding", "chunked", _transaction->allocator());

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s} cannot build response", _socket.logAddress());
    return sendInternalServerErrorResponse();
  }

  if (110 <= _transaction->request().httpVersion() && !_transaction->request().reuseConnection()) {
    error = _transaction->response().addHeader("Connection", "close", _transaction->allocator());

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot build success response", _socket.logAddress());
      return sendInternalServerErrorResponse();
    }
  }

  ESB_LOG_DEBUG("[%s] sending response: %d %s", _socket.logAddress(), _transaction->response().statusCode(),
                _transaction->response().reasonPhrase());
  _state &= ~(TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY);
  _state |= FORMATTING_HEADERS;
  return handleWritable();
}

ESB::Error HttpServerSocket::sendBadRequestResponse() {
  _transaction->response().setStatusCode(400);
  _transaction->response().setReasonPhrase("Bad Request");
  _transaction->response().setHasBody(false);
  return sendResponse();
}

ESB::Error HttpServerSocket::sendInternalServerErrorResponse() {
  // TODO reserve a static read-only internal server error response for out of
  // memory conditions.

  _transaction->response().setStatusCode(500);
  _transaction->response().setReasonPhrase("Internal Server Error");
  _transaction->response().setHasBody(false);

  ESB::Error error = _transaction->response().addHeader("Content-Length", "0", _transaction->allocator());

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot create 500 response", _socket.logAddress());
  }

  if (110 <= _transaction->request().httpVersion() &&
      (!_transaction->request().reuseConnection() || CloseAfterErrorResponse)) {
    error = _transaction->response().addHeader("Connection", "close", _transaction->allocator());

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot create 500 response", _socket.logAddress());
    }
  }

  _state &= ~(TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY);
  _state |= FORMATTING_HEADERS;

  ESB_LOG_DEBUG("[%s] sending response: %d %s", _socket.logAddress(), _transaction->response().statusCode(),
                _transaction->response().reasonPhrase());
  return handleWritable();
}

const char *HttpServerSocket::logAddress() const { return _socket.logAddress(); }

ESB::Error HttpServerSocket::abort(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] server connection aborted", _socket.logAddress());

  _state |= ABORTED;
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().removeMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot abort server connection", _socket.logAddress());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::pauseRecv(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  assert(!(RECV_PAUSED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED | RECV_PAUSED)) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] pausing server request receive", _socket.logAddress());

  _state |= RECV_PAUSED;
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause server response receive", _socket.logAddress());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::resumeRecv(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  assert(RECV_PAUSED & _state);
  if (_state & (HAS_BEEN_REMOVED | ABORTED) || !(_state & RECV_PAUSED)) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] resuming server request receive", _socket.logAddress());

  _state &= ~RECV_PAUSED;
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume server response receive", _socket.logAddress());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::pauseSend(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  assert(!(SEND_PAUSED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED | SEND_PAUSED)) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] pausing server response send", _socket.logAddress());

  _state |= SEND_PAUSED;
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause server response send", _socket.logAddress());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::resumeSend(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  assert(SEND_PAUSED & _state);
  if (_state & (HAS_BEEN_REMOVED | ABORTED) || !(_state & SEND_PAUSED)) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] resuming server request send", _socket.logAddress());

  _state &= ~SEND_PAUSED;
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume server response send", _socket.logAddress());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Allocator &HttpServerSocket::allocator() {
  assert(_transaction);
  return _transaction->allocator();
}

const HttpRequest &HttpServerSocket::request() const {
  assert(_transaction);
  return _transaction->request();
}

HttpRequest &HttpServerSocket::request() {
  assert(_transaction);
  return _transaction->request();
}

const HttpResponse &HttpServerSocket::response() const {
  assert(_transaction);
  return _transaction->response();
}

HttpResponse &HttpServerSocket::response() {
  assert(_transaction);
  return _transaction->response();
}

void HttpServerSocket::setContext(void *context) {
  assert(_transaction);
  _transaction->setContext(context);
}

void *HttpServerSocket::context() {
  assert(_transaction);
  return _transaction->context();
}

const void *HttpServerSocket::context() const {
  assert(_transaction);
  return _transaction->context();
}

const ESB::SocketAddress &HttpServerSocket::peerAddress() const { return _socket.peerAddress(); }

ESB::Error HttpServerSocket::formatStartChunk(ESB::UInt32 chunkSize, ESB::UInt32 *maxChunkSize) {
  ESB::Error error = _transaction->getFormatter()->beginBlock(_sendBuffer, chunkSize, maxChunkSize);
  switch (error) {
    case ESB_SUCCESS:
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] insufficient send buffer space to begin chunk", _socket.logAddress());
      return ESB_AGAIN;
    default:
      ESB_LOG_INFO_ERRNO(error, "[%s] error formatting response body", _socket.logAddress());
      return error;
  }
}

ESB::Error HttpServerSocket::formatEndChunk() {
  ESB::Error error = _transaction->getFormatter()->endBlock(_sendBuffer);

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot format response chunk end block", _socket.logAddress());
    return error;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::formatEndBody() {
  ESB::Error error = _transaction->getFormatter()->endBody(_sendBuffer);

  switch (error) {
    case ESB_SUCCESS:
      ESB_LOG_DEBUG("[%s] finished formatting response body", _socket.logAddress());
      _state &= ~FORMATTING_BODY;
      _state |= FLUSHING_BODY;
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] insufficient space in send buffer to format end body", _socket.logAddress());
      return ESB_AGAIN;
    default:
      ESB_LOG_INFO_ERRNO(error, "[%s] error formatting last response chunk", _socket.logAddress());
      return error;
  }
}

}  // namespace ES
