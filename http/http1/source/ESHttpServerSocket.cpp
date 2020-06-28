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
#define FORMATTING_BODY (1 << 5)
#define FLUSHING_BODY (1 << 6)
#define TRANSACTION_BEGIN (1 << 7)
#define TRANSACTION_END (1 << 8)
#define CANNOT_REUSE_CONNECTION (1 << 9)
#define RECV_PAUSED (1 << 10)
#define SEND_PAUSED (1 << 11)
#define ABORTED (1 << 12)

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
      _socket(multiplexer.multiplexer().name(), "server") {}

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

  if (_state & (FORMATTING_HEADERS | FORMATTING_BODY | FLUSHING_BODY)) {
    return true;
  }

  return false;
}

bool HttpServerSocket::isIdle() {
  // TODO - implement idle timeout.  Return true if bytes transferred over time interval is too low
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
      ESB_LOG_DEBUG("[%s] %u request bytes available in current chunk", _socket.name(), *bytesAvailable);
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] insufficient bytes available in current chunk", _socket.name());
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] error parsing request body", _socket.name());
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
        stateTransition(FORMATTING_HEADERS);
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
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot parse request body", _socket.name());
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
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot consume %u request chunk bytes", _socket.name(), bytesRequested);
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[%s] consumed %u request chunk bytes", _socket.name(), bytesRequested);

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
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer", _socket.name());
      return ESB_OUT_OF_MEMORY;  // remove from multiplexer
    }
  }

  if (!_transaction) {
    assert(_state & TRANSACTION_BEGIN);
    _transaction = _multiplexer.createServerTransaction();
    if (!_transaction) {
      _multiplexer.releaseBuffer(_recvBuffer);
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create server trans", _socket.name());
      return ESB_OUT_OF_MEMORY;  // remove from multiplexer
    }
  }

  ESB::Error error = ESB_SUCCESS;

  if (_state & TRANSACTION_BEGIN) {
    // If we don't fully read the request we can't reuse the socket.  Unset this flag once the request has been fully
    // read.
    setFlag(CANNOT_REUSE_CONNECTION);
    _transaction->setPeerAddress(_socket.peerAddress());

    switch (error = _handler.beginTransaction(_multiplexer, *this)) {
      case ESB_SUCCESS:
        break;
      case ESB_AGAIN:
        return ESB_AGAIN;
      case ESB_SEND_RESPONSE:
        return sendResponse();
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborted connection", _socket.name());
        return error;
    }

    conditionalStateTransition(TRANSACTION_BEGIN, PARSING_HEADERS);
  }

  while (!_multiplexer.shutdown()) {
    switch (error = fillReceiveBuffer()) {
      case ESB_SUCCESS:
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
        unsetFlag(CANNOT_REUSE_CONNECTION);
      }

      // TODO - check Expect header and maybe send a 100 Continue

      switch (error = _handler.receiveRequestHeaders(_multiplexer, *this)) {
        case ESB_SUCCESS:
          break;
        case ESB_PAUSE:
        case ESB_AGAIN:
          if (ESB_SUCCESS != (error = pauseRecv(false))) {
            return ESB_AGAIN == error ? ESB_OTHER_ERROR : error;
          }
          return ESB_AGAIN;
        case ESB_SEND_RESPONSE:
          return sendResponse();
        default:
          ESB_LOG_DEBUG_ERRNO(error, "[%s] Server request header handler aborting connection", _socket.name());
          return error;
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
              return ESB_AGAIN == error ? ESB_OTHER_ERROR : error;
            }
            return ESB_AGAIN;
          default:
            ESB_LOG_DEBUG_ERRNO(error, "[%s] server body handler aborted connection", _socket.name());
            return error;
        }
      }

      conditionalStateTransition(PARSING_HEADERS, PARSING_BODY);
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
        return ESB_AGAIN;
      }

      if (ESB_SUCCESS != error) {
        if (IsHttpError(error)) {
          return sendBadRequestResponse();
        } else {
          return sendInternalServerErrorResponse();
        }
      }

      if (_state & FORMATTING_HEADERS) {
        // server handler decided to send response before finishing body
        assert(_state & CANNOT_REUSE_CONNECTION);
        return sendResponse();
      }

      assert(_state & SKIPPING_TRAILER);
    }

    if (SKIPPING_TRAILER & _state) {
      assert(_transaction->request().hasBody());
      error = skipTrailer();

      if (ESB_AGAIN == error) {
        continue;
      }

      if (ESB_SUCCESS != error) {
        return error;
      }

      // Request has been fully read, so we can reuse the connection, but don't
      // start reading the next request until the response has been fully sent.

      unsetFlag(CANNOT_REUSE_CONNECTION);
      if (ESB_SUCCESS != (error = pauseRecv(false))) {
        return ESB_AGAIN == error ? ESB_OTHER_ERROR : error;
      }
    }

    // server handler should have populated the response object in
    // parseRequestBody()
    return sendResponse();
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown with socket in parse state", _socket.name());
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
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot send empty response", _socket.name());
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
    case ESB_CLEANUP:
      // TODO not sure about htis
      return ESB_SUCCESS;
    case ESB_AGAIN:
      if (ESB_SUCCESS != (error = resumeSend(true))) {
        abort(true);
        return error;
      }
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot send response", _socket.name());
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
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot send request body", _socket.name());
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
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer", _socket.name());
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
            return ESB_AGAIN;
          default:
            return error;
        }
        continue;
      }

      if (ESB_SUCCESS != error) {
        return error;  // remove from multiplexer;
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

    if (FLUSHING_BODY & _state) {
      switch (error = flushSendBuffer()) {
        case ESB_SUCCESS:
          break;
        case ESB_AGAIN:
          return ESB_AGAIN;
        default:
          return error;  // remove from multiplexer
      }

      stateTransition(TRANSACTION_END);
    }

    assert(_state & TRANSACTION_END);

    ++_requestsPerConnection;
    _handler.endTransaction(_multiplexer, *this, HttpServerHandler::ES_HTTP_SERVER_HANDLER_END);

    if (CANNOT_REUSE_CONNECTION & _state) {
      return ESB_SUCCESS;
    }

    if (CloseAfterErrorResponse && 300 <= _transaction->response().statusCode()) {
      return ESB_CLEANUP;
    }

    // TODO - close connection if max requests sent on connection

    if (!_transaction->request().reuseConnection()) {
      return ESB_SUCCESS;
    }

    // Resume receiving and immediately start processing the next request.

    if (ESB_SUCCESS != (error = resumeRecv(false))) {
      return ESB_AGAIN == error ? ESB_OTHER_ERROR : error;
    }

    // TODO release buffers in between transactions

    stateTransition(TRANSACTION_BEGIN);
    _bodyBytesWritten = 0;
    _transaction->reset();
    return handleReadable();
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown with socket in format state", _socket.name());
  return ESB_SHUTDOWN;  // remove from multiplexer
}

void HttpServerSocket::handleError(ESB::Error error) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO_ERRNO(error, "[%s] socket error", _socket.name());
}

void HttpServerSocket::handleRemoteClose() {
  // TODO - this may just mean the client closed its half of the socket but is
  // still expecting a response.
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] remote client closed socket", _socket.name());
}

void HttpServerSocket::handleIdle() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] client is idle", _socket.name());
}

bool HttpServerSocket::handleRemove() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] closing server socket", _socket.name());
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
  } else if (_state & FORMATTING_HEADERS) {
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

  stateTransition(HAS_BEEN_REMOVED);
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
    ESB_LOG_DEBUG("[%s] need more request header data from stream", _socket.name());
    if (!_recvBuffer->compact()) {
      ESB_LOG_INFO("[%s] cannot parse request headers: parser jammed", _socket.name());
      return ESB_OVERFLOW;  // remove from multiplexer
    }
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot parse request headers", _socket.name());
    return error;  // remove from multiplexer
  }

  // parse complete

  if (ESB_DEBUG_LOGGABLE) {
    const char *method = (const char *)_transaction->request().method();
    int major = _transaction->request().httpVersion() / 100;
    int minor = _transaction->request().httpVersion() % 100 / 10;

    switch (_transaction->request().requestUri().type()) {
      case HttpRequestUri::ES_URI_ASTERISK:
        ESB_LOG_DEBUG("[%s] request line: %s * HTTP/%d.%d", _socket.name(), method, major, minor);
        break;
      case HttpRequestUri::ES_URI_HTTP:
      case HttpRequestUri::ES_URI_HTTPS:
        ESB_LOG_DEBUG("[%s] request line: %s %s//%s:%d%s?%s#%s HTTP/%d.%d", _socket.name(), method,
                      _transaction->request().requestUri().typeString(),
                      ESB_SAFE_STR(_transaction->request().requestUri().host()),
                      _transaction->request().requestUri().port(),
                      ESB_SAFE_STR(_transaction->request().requestUri().absPath()),
                      ESB_SAFE_STR(_transaction->request().requestUri().query()),
                      ESB_SAFE_STR(_transaction->request().requestUri().fragment()), major, minor);
        break;
      case HttpRequestUri::ES_URI_OTHER:
        ESB_LOG_DEBUG("[%s] request line: %s %s HTTP/%d.%d", _socket.name(), method,
                      ESB_SAFE_STR(_transaction->request().requestUri().other()), major, minor);
        break;
    }

    HttpHeader *header = (HttpHeader *)_transaction->request().headers().first();
    for (; header; header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("[%s] request header: %s: %s", _socket.name(), ESB_SAFE_STR(header->fieldName()),
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
      ESB_LOG_DEBUG("[%s] parsed request body", _socket.name());
      unsigned char byte = 0;
      switch (error = _handler.consumeRequestBody(_multiplexer, *this, &byte, 0U, &bytesConsumed)) {
        case ESB_SUCCESS:
        case ESB_SEND_RESPONSE:
          stateTransition(SKIPPING_TRAILER);
          assert(wantRead());
          return ESB_SUCCESS;
        case ESB_PAUSE:
        case ESB_AGAIN:
          if (ESB_SUCCESS != (error = pauseRecv(false))) {
            return error;
          }
          return ESB_PAUSE;
        default:
          ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborting connection after last request body chunk", _socket.name());
          return error;
      }
    }

    ESB_LOG_DEBUG("[%s] server socket offering request chunk of size %u", _socket.name(), bytesAvailable);

    switch (error = _handler.consumeRequestBody(_multiplexer, *this, _recvBuffer->buffer() + bufferOffset,
                                                bytesAvailable, &bytesConsumed)) {
      case ESB_SUCCESS:
        if (0 == bytesConsumed) {
          ESB_LOG_DEBUG("[%s] pausing request body receive until handler is ready", _socket.name());
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
        ESB_LOG_DEBUG("[%s] handler sending response before last request body chunk", _socket.name());
        stateTransition(FORMATTING_HEADERS);
        return ESB_SUCCESS;
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborting connection before last request body chunk", _socket.name());
        return error;
    }

    error = _transaction->getParser()->consumeBody(_recvBuffer, bytesConsumed);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot consume %u request chunk bytes", _socket.name(), bytesAvailable);
      return error;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("[%s] handler consumed %u out of %u response chunk bytes", _socket.name(), bytesConsumed,
                  bytesAvailable);
  }

  return ESB_SHUTDOWN;
}

ESB::Error HttpServerSocket::skipTrailer() {
  assert(_state & SKIPPING_TRAILER);
  assert(_transaction);

  ESB::Error error = _transaction->getParser()->skipTrailer(_recvBuffer);

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] need more data from stream to skip trailer", _socket.name());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error skipping trailer", _socket.name());
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[%s] request trailer skipped", _socket.name());
  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::formatResponseHeaders() {
  assert(_transaction);

  ESB::Error error = _transaction->getFormatter()->formatHeaders(_sendBuffer, _transaction->response());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] partially formatted response headers", _socket.name());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error formatting response header", _socket.name());
    return error;
  }

  ESB_LOG_DEBUG("[%s] formatted response headers", _socket.name());

  stateTransition(FORMATTING_BODY);

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
        ESB_LOG_DEBUG("[%s] handler offers response chunk of %u bytes", _socket.name(), offeredSize);
        break;
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler error offering response chunk", _socket.name());
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
        ESB_LOG_INFO_ERRNO(error, "[%s] cannot format response chunk of size %u", _socket.name(), chunkSize);
        return error;  // remove from multiplexer
    }

    _sendBuffer->setWritePosition(_sendBuffer->writePosition() + chunkSize);
    _bodyBytesWritten += chunkSize;
    ESB_LOG_DEBUG("[%s] formatted response chunk of size %u", _socket.name(), chunkSize);

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
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot refill recv buffer", _socket.name());
    return error;
  }

  if (0 == result) {
    ESB_LOG_DEBUG("[%s] connection closed during recv buffer refill", _socket.name());
    return ESB_CLOSED;
  }

  ESB_LOG_DEBUG("[%s] read %ld bytes into recv buffer", _socket.name(), result);
  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::flushSendBuffer() {
  assert(_transaction);
  assert(_sendBuffer);

  ESB_LOG_DEBUG("[%s] flushing response output buffer", _socket.name());

  if (!_sendBuffer->isReadable()) {
    ESB_LOG_INFO("[%s] response formatter jammed", _socket.name());
    return ESB_OVERFLOW;  // remove from multiplexer
  }

  while (!_multiplexer.shutdown() && _sendBuffer->isReadable()) {
    ESB::SSize bytesSent = _socket.send(_sendBuffer);

    if (0 > bytesSent) {
      if (ESB_AGAIN == bytesSent) {
        ESB_LOG_DEBUG("[%s] would block flushing response output buffer", _socket.name());
        return ESB_AGAIN;  // keep in multiplexer
      }

      ESB::Error error = ESB::LastError();
      assert(ESB_SUCCESS != error);
      ESB_LOG_INFO_ERRNO(error, "[%s] error flushing response output buffer", _socket.name());
      return error;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("[%s] flushed %ld bytes from response output buffer", _socket.name(), bytesSent);
  }

  return _sendBuffer->isReadable() ? ESB_SHUTDOWN : ESB_SUCCESS;
}

ESB::Error HttpServerSocket::sendResponse() {
  if (0 == _transaction->response().statusCode()) {
    ESB_LOG_INFO(
        "[%s] server handler failed to build response, sending 500 Internal "
        "Server Error",
        _socket.name());
    return sendInternalServerErrorResponse();
  }

  // TODO strip Transfer-Encoding, Content-Length, & Connection headers from the
  // response object
  // TODO add date header and any other  headers like that

  ESB::Error error = _transaction->response().addHeader("Transfer-Encoding", "chunked", _transaction->allocator());

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s} cannot build response", _socket.name());
    return sendInternalServerErrorResponse();
  }

  if (110 <= _transaction->request().httpVersion() && !_transaction->request().reuseConnection()) {
    error = _transaction->response().addHeader("Connection", "close", _transaction->allocator());

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot build success response", _socket.name());
      return sendInternalServerErrorResponse();
    }
  }

  ESB_LOG_DEBUG("[%s] sending response: %d %s", _socket.name(), _transaction->response().statusCode(),
                _transaction->response().reasonPhrase());
  if (!(FORMATTING_HEADERS & _state)) {
    stateTransition(FORMATTING_HEADERS);
  }
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
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot create 500 response", _socket.name());
  }

  if (110 <= _transaction->request().httpVersion() &&
      (!_transaction->request().reuseConnection() || CloseAfterErrorResponse)) {
    error = _transaction->response().addHeader("Connection", "close", _transaction->allocator());

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot create 500 response", _socket.name());
    }
  }

  stateTransition(FORMATTING_HEADERS);

  ESB_LOG_DEBUG("[%s] sending response: %d %s", _socket.name(), _transaction->response().statusCode(),
                _transaction->response().reasonPhrase());
  return handleWritable();
}

const char *HttpServerSocket::logAddress() const { return _socket.name(); }

ESB::Error HttpServerSocket::abort(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] server connection aborted", _socket.name());

  setFlag(ABORTED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().removeMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot abort server connection", _socket.name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::pauseRecv(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (_state & RECV_PAUSED) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] pausing server request receive", _socket.name());

  setFlag(RECV_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause server response receive", _socket.name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::resumeRecv(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (!(_state & RECV_PAUSED)) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] resuming server request receive", _socket.name());

  unsetFlag(RECV_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume server response receive", _socket.name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::pauseSend(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (_state & SEND_PAUSED) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] pausing server response send", _socket.name());

  setFlag(SEND_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause server response send", _socket.name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::resumeSend(bool updateMultiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(ABORTED & _state));
  if (_state & (HAS_BEEN_REMOVED | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (!(_state & SEND_PAUSED)) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] resuming server request send", _socket.name());

  unsetFlag(SEND_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume server response send", _socket.name());
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
      ESB_LOG_DEBUG("[%s] insufficient send buffer space to begin chunk", _socket.name());
      return ESB_AGAIN;
    default:
      ESB_LOG_INFO_ERRNO(error, "[%s] error formatting response body", _socket.name());
      return error;
  }
}

ESB::Error HttpServerSocket::formatEndChunk() {
  ESB::Error error = _transaction->getFormatter()->endBlock(_sendBuffer);

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot format response chunk end block", _socket.name());
    return error;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::formatEndBody() {
  ESB::Error error = _transaction->getFormatter()->endBody(_sendBuffer);

  switch (error) {
    case ESB_SUCCESS:
      ESB_LOG_DEBUG("[%s] finished formatting response body", _socket.name());
      stateTransition(FLUSHING_BODY);
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] insufficient space in send buffer to format end body", _socket.name());
      return ESB_AGAIN;
    default:
      ESB_LOG_INFO_ERRNO(error, "[%s] error formatting last response chunk", _socket.name());
      return error;
  }
}

const char *HttpServerSocket::name() const { return _socket.name(); }

void HttpServerSocket::setFlag(int flag) {
  assert(flag & flagMask());
  assert(!(flag & stateMask()));
  _state |= flag;
}

void HttpServerSocket::unsetFlag(int flag) {
  assert(flag & flagMask());
  assert(!(flag & stateMask()));
  _state &= ~flag;
}

void HttpServerSocket::stateTransition(int state) {
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
    case PARSING_HEADERS:
      assert(currentState & TRANSACTION_BEGIN);
      _state &= ~TRANSACTION_BEGIN;
      _state |= PARSING_HEADERS;
      ESB_LOG_DEBUG("[%s] parsing headers", _socket.name());
      break;
    case PARSING_BODY:
      assert(currentState & PARSING_HEADERS);
      _state &= ~PARSING_HEADERS;
      _state |= PARSING_BODY;
      ESB_LOG_DEBUG("[%s] parsing body", _socket.name());
      break;
    case SKIPPING_TRAILER:
      assert(currentState & PARSING_BODY);
      _state &= ~PARSING_BODY;
      _state |= SKIPPING_TRAILER;
      ESB_LOG_DEBUG("[%s] skipping trailer", _socket.name());
      break;
    case FORMATTING_HEADERS:
      assert(currentState & (TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY | SKIPPING_TRAILER));
      _state &= ~(TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY | SKIPPING_TRAILER);
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
    case TRANSACTION_BEGIN:
      assert(currentState & TRANSACTION_END);
      _state &= ~TRANSACTION_END;
      _state |= TRANSACTION_BEGIN;
      ESB_LOG_DEBUG("[%s] transaction begun", _socket.name());
      break;
    case TRANSACTION_END:
      assert(currentState & FLUSHING_BODY);
      _state &= ~FLUSHING_BODY;
      _state |= TRANSACTION_END;
      ESB_LOG_DEBUG("[%s] transaction complete", _socket.name());
      break;
    default:
      assert(0 == "Cannot transition into unknown state");
  }
}

int HttpServerSocket::stateMask() {
  return (HAS_BEEN_REMOVED | PARSING_HEADERS | PARSING_BODY | SKIPPING_TRAILER | FORMATTING_HEADERS | FORMATTING_BODY |
          FLUSHING_BODY | TRANSACTION_BEGIN | TRANSACTION_END);
}

int HttpServerSocket::flagMask() {
  return ~(HAS_BEEN_REMOVED | PARSING_HEADERS | PARSING_BODY | SKIPPING_TRAILER | FORMATTING_HEADERS | FORMATTING_BODY |
           FLUSHING_BODY | TRANSACTION_BEGIN | TRANSACTION_END);
}

}  // namespace ES
