#ifndef ES_HTTP_SERVER_SOCKET_H
#include <ESHttpServerSocket.h>
#endif

#ifndef ES_HTTP_ERROR_H
#include <ESHttpError.h>
#endif

namespace ES {

// TODO - add performance counters
// TODO - max requests per connection option (1 disables keepalives)
// TODO - max header size option
// TODO - max body size option

static bool CloseAfterErrorResponse = true;

/**
 * The state machine is built around the concept of a set of callback functions that produce and consume on demand, but
 * some APIs support direct passing in of buffers.  This class wraps a buffer that was directly passed in with a
 * callback adaptor that can be subsequently invoked by the state machine.
 *
 */
class HttpResponseBodyProducer : public HttpServerHandler {
 public:
  HttpResponseBodyProducer(unsigned const char *buffer, ESB::UInt32 size)
      : _buffer(buffer), _size(size), _bytesProduced(0) {}
  virtual ~HttpResponseBodyProducer() {}

  virtual ESB::Error acceptConnection(HttpMultiplexer &multiplexer, ESB::SocketAddress *address) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error receiveRequestHeaders(HttpMultiplexer &multiplexer, HttpServerStream &serverStream) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                       ESB::UInt32 *bytesAvailable) {
    if (!bytesAvailable) {
      return ESB_NULL_POINTER;
    }

    if (!_size) {
      // end body, transition to flushing state
      *bytesAvailable = 0;
      return ESB_SUCCESS;
    }

    *bytesAvailable = _size - _bytesProduced;
    return 0 == *bytesAvailable ? ESB_BREAK : ESB_SUCCESS;
  }

  virtual ESB::Error produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                         unsigned char *body, ESB::UInt32 bytesRequested) {
    assert(bytesRequested <= _size - _bytesProduced);
    if (bytesRequested > _size - _bytesProduced) {
      return ESB_INVALID_ARGUMENT;
    }

    memcpy(body, _buffer + _bytesProduced, bytesRequested);
    _bytesProduced += bytesRequested;
    return ESB_SUCCESS;
  }

  virtual ESB::Error consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                        const unsigned char *body, ESB::UInt32 bytesOffered,
                                        ESB::UInt32 *bytesConsumed) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error endRequest(HttpMultiplexer &multiplexer, HttpServerStream &serverStream) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream, State state) {
    assert(!"function should not be called");
  }

  inline ESB::UInt32 bytesProduced() const { return _bytesProduced; };

 private:
  HttpResponseBodyProducer(const HttpResponseBodyProducer &disabled);
  void operator=(const HttpResponseBodyProducer &disabled);

  unsigned const char *_buffer;
  ESB::UInt32 _size;
  ESB::UInt32 _bytesProduced;
};

/**
 * The state machine is built around the concept of a set of callback functions that produce and consume on demand, but
 * some APIs support direct passing in of buffers.  This class wraps a buffer that was directly passed in with a
 * callback adaptor that can be subsequently invoked by the state machine.
 *
 */
class HttpRequestBodyConsumer : public HttpServerHandler {
 public:
  HttpRequestBodyConsumer(unsigned const char *buffer, ESB::UInt32 size)
      : _buffer(buffer), _size(size), _bytesConsumed(0), _bytesOffered(0) {}
  virtual ~HttpRequestBodyConsumer() {}

  virtual ESB::Error acceptConnection(HttpMultiplexer &multiplexer, ESB::SocketAddress *address) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error receiveRequestHeaders(HttpMultiplexer &multiplexer, HttpServerStream &serverStream) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                       ESB::UInt32 *bytesAvailable) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                         unsigned char *body, ESB::UInt32 bytesRequested) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                        const unsigned char *body, ESB::UInt32 bytesOffered,
                                        ESB::UInt32 *bytesConsumed) {
    if (!body || !bytesConsumed) {
      return ESB_NULL_POINTER;
    }

    _bytesOffered = bytesOffered;
    ESB::UInt32 bytesToCopy = MIN(_size - _bytesConsumed, bytesOffered);

    if (0 == bytesOffered) {
      // last chunk so advance state machine
      *bytesConsumed = 0U;
      return ESB_SUCCESS;
    }

    if (0 == bytesToCopy) {
      // we've filled the buffer but more body remains
      *bytesConsumed = 0U;
      return ESB_BREAK;
    }

    memcpy((void *)(_buffer + _bytesConsumed), body, bytesToCopy);
    *bytesConsumed = bytesToCopy;
    _bytesConsumed += bytesToCopy;
    return ESB_SUCCESS;
  }

  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream, State state) {
    assert(!"function should not be called");
  }

  inline ESB::UInt32 bytesConsumed() const { return _bytesConsumed; };

  inline ESB::UInt32 bytesOffered() const { return _bytesOffered; }

 private:
  HttpRequestBodyConsumer(const HttpRequestBodyConsumer &disabled);
  void operator=(const HttpRequestBodyConsumer &disabled);

  unsigned const char *_buffer;
  ESB::UInt32 _size;
  ESB::UInt32 _bytesConsumed;
  ESB::UInt32 _bytesOffered;
};

HttpServerSocket::HttpServerSocket(ESB::ConnectedSocket *socket, HttpServerHandler &handler,
                                   HttpMultiplexerExtended &multiplexer, HttpServerCounters &counters,
                                   ESB::CleanupHandler &cleanupHandler)
    : _state(SERVER_TRANSACTION_BEGIN),
      _bodyBytesWritten(0),
      _requestsPerConnection(0),
      _bytesAvailable(0),
      _multiplexer(multiplexer),
      _handler(handler),
      _transaction(NULL),
      _counters(counters),
      _cleanupHandler(cleanupHandler),
      _recvBuffer(NULL),
      _sendBuffer(NULL),
      _socket(socket) {}

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

ESB::Error HttpServerSocket::reset(ESB::Socket::State &state) {
  assert(!_recvBuffer);
  assert(!_sendBuffer);
  assert(!_transaction);

  _state = SERVER_TRANSACTION_BEGIN;
  _bodyBytesWritten = 0;
  _socket->reset(state);

  return ESB_SUCCESS;
}

bool HttpServerSocket::wantAccept() { return false; }

bool HttpServerSocket::wantConnect() { return false; }

bool HttpServerSocket::wantRead() {
  if (_state & (SERVER_RECV_PAUSED | SERVER_ABORTED | SERVER_INACTIVE)) {
    return false;
  }

  return _state & (SERVER_TRANSACTION_BEGIN | SERVER_RECV_STATE_MASK);
}

bool HttpServerSocket::wantWrite() {
  if (_state & (SERVER_SEND_PAUSED | SERVER_ABORTED | SERVER_INACTIVE)) {
    return false;
  }

  return _state & SERVER_SEND_STATE_MASK;
}

bool HttpServerSocket::isIdle() {
  // TODO - implement idle timeout.  Return true if bytes transferred over time interval is too low
  return false;
}

ESB::Error HttpServerSocket::handleAccept() {
  assert(!(SERVER_INACTIVE & _state));
  ESB_LOG_ERROR("[%d] Cannot handle accept events", _socket->socketDescriptor());
  return ESB_INVALID_STATE;  // remove from multiplexer
}

ESB::Error HttpServerSocket::handleConnect() {
  assert(!(SERVER_INACTIVE & _state));
  ESB_LOG_ERROR("[%d] Cannot handle connect events", _socket->socketDescriptor());
  return ESB_INVALID_STATE;  // remove from multiplexer
}

ESB::Error HttpServerSocket::requestBodyAvailable(ESB::UInt32 *bytesAvailable) {
  if (!bytesAvailable) {
    return ESB_NULL_POINTER;
  }

  HttpRequestBodyConsumer adaptor(NULL, 0);

  ESB::Error error = advanceStateMachine(adaptor, SERVER_UPDATE_MULTIPLEXER | SERVER_ADVANCE_RECV);
  *bytesAvailable = adaptor.bytesOffered();
  return error;
}

ESB::Error HttpServerSocket::readRequestBody(unsigned char *chunk, ESB::UInt32 bytesRequested, ESB::UInt32 *bytesRead) {
  assert(chunk);
  if (!chunk) {
    return ESB_NULL_POINTER;
  }

  HttpRequestBodyConsumer adaptor(chunk, bytesRequested);
  ESB::Error error = advanceStateMachine(adaptor, SERVER_UPDATE_MULTIPLEXER | SERVER_ADVANCE_RECV);
  *bytesRead = adaptor.bytesConsumed();

  return error;
}

ESB::Error HttpServerSocket::handleReadable() {
  assert(wantRead());
  assert(_socket->connected());

  if (!wantRead() || !_socket->connected()) {
    return ESB_INVALID_STATE;
  }

  return advanceStateMachine(_handler, SERVER_INITIAL_FILL_RECV_BUFFER | SERVER_ADVANCE_RECV | SERVER_ADVANCE_SEND);
}

ESB::Error HttpServerSocket::sendEmptyResponse(int statusCode, const char *reasonPhrase) {
  ESB_LOG_DEBUG("[%s] sending response %d %s", _socket->name(), statusCode, reasonPhrase);

  ESB::Error error = setResponse(statusCode, reasonPhrase);
  if (ESB_SUCCESS != error) {
    abort(true);
    return error;
  }

  return advanceStateMachine(_handler, SERVER_ADVANCE_SEND);
}

ESB::Error HttpServerSocket::sendResponse(const HttpResponse &response) {
  ESB_LOG_DEBUG("[%s] sending response %d %s", _socket->name(), response.statusCode(), response.reasonPhrase());

  ESB::Error error = _transaction->response().copy(&response, _transaction->allocator());
  if (ESB_SUCCESS != error) {
    abort(true);
    return error;
  }

  // This can send the full response including body
  return advanceStateMachine(_handler, SERVER_ADVANCE_SEND);
}

ESB::Error HttpServerSocket::sendResponseBody(unsigned const char *chunk, ESB::UInt32 bytesOffered,
                                              ESB::UInt32 *bytesConsumed) {
  if (!chunk || !bytesConsumed) {
    abort(true);
    return ESB_NULL_POINTER;
  }

  int state = _state & SERVER_STATE_MASK;
  if (state != SERVER_FORMATTING_BODY) {
    abort(true);
    return ESB_INVALID_STATE;
  }

  HttpResponseBodyProducer adaptor(chunk, bytesOffered);

  ESB::Error error = advanceStateMachine(adaptor, SERVER_UPDATE_MULTIPLEXER | SERVER_ADVANCE_SEND);
  *bytesConsumed = adaptor.bytesProduced();
  return error;
}

ESB::Error HttpServerSocket::handleWritable() {
  assert(wantWrite());
  assert(_socket->connected());
  assert(_transaction);

  if (!wantWrite() || !_socket->connected() || !_transaction) {
    return ESB_INVALID_STATE;
  }

  // TODO measure impact of adding SERVER_INITIAL_DRAIN_SEND_BUFFER here
  return advanceStateMachine(_handler, SERVER_ADVANCE_RECV | SERVER_ADVANCE_SEND);
}

void HttpServerSocket::handleError(ESB::Error error) {
  assert(!(SERVER_INACTIVE & _state));
  ESB_LOG_INFO_ERRNO(error, "[%s] socket error", _socket->name());
}

void HttpServerSocket::handleRemoteClose() {
  // TODO - this may just mean the client closed its half of the socket but is
  // still expecting a response.
  assert(!(SERVER_INACTIVE & _state));
  ESB_LOG_INFO("[%s] remote client closed socket", _socket->name());
}

void HttpServerSocket::handleIdle() {
  assert(!(SERVER_INACTIVE & _state));
  ESB_LOG_INFO("[%s] client is idle", _socket->name());
}

bool HttpServerSocket::handleRemove() {
  assert(!(SERVER_INACTIVE & _state));
  ESB_LOG_INFO("[%s] closing server socket", _socket->name());
  _socket->close();

  if (_sendBuffer) {
    _multiplexer.releaseBuffer(_sendBuffer);
    _sendBuffer = NULL;
  }
  if (_recvBuffer) {
    _multiplexer.releaseBuffer(_recvBuffer);
    _recvBuffer = NULL;
  }

  if (_state & SERVER_PARSING_HEADERS) {
    assert(_transaction);
    _handler.endTransaction(_multiplexer, *this, HttpServerHandler::ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS);
  } else if (_state & SERVER_PARSING_BODY) {
    assert(_transaction);
    _handler.endTransaction(_multiplexer, *this, HttpServerHandler::ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY);
  } else if (_state & SERVER_FORMATTING_HEADERS) {
    assert(_transaction);
    _handler.endTransaction(_multiplexer, *this, HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS);
  } else if (_state & (SERVER_FORMATTING_BODY | SERVER_FLUSHING_BODY)) {
    assert(_transaction);
    _handler.endTransaction(_multiplexer, *this, HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY);
  }

  if (_transaction) {
    _multiplexer.destroyServerTransaction(_transaction);
    _transaction = NULL;
  }

  stateTransition(SERVER_INACTIVE);
  _counters.getAverageTransactionsPerConnection()->add(_requestsPerConnection);
  _requestsPerConnection = 0;

  return true;  // call cleanup handler on us after this returns
}

SOCKET HttpServerSocket::socketDescriptor() const { return _socket->socketDescriptor(); }

ESB::CleanupHandler *HttpServerSocket::cleanupHandler() { return &_cleanupHandler; }

ESB::Error HttpServerSocket::advanceStateMachine(HttpServerHandler &handler, int flags) {
  bool fillRecvBuffer = flags & SERVER_INITIAL_FILL_RECV_BUFFER;
  bool drainSendBuffer = flags & SERVER_INITIAL_DRAIN_SEND_BUFFER;
  bool updateMultiplexer = flags & SERVER_UPDATE_MULTIPLEXER;

  while (!_multiplexer.shutdown()) {
    bool inRecvState = _state & SERVER_RECV_STATE_MASK;
    bool inSendState = _state & SERVER_SEND_STATE_MASK;

    if (inRecvState && !(flags & SERVER_ADVANCE_RECV)) {
      return ESB_SUCCESS;
    }

    if (inSendState && !(flags & SERVER_ADVANCE_SEND)) {
      return ESB_SUCCESS;
    }

    if (fillRecvBuffer) {
      switch (ESB::Error error = fillReceiveBuffer()) {
        case ESB_SUCCESS:
          fillRecvBuffer = false;
          break;
        case ESB_AGAIN:
          return ESB_AGAIN;
        case ESB_CLOSED:
          handleRemoteClose();
          abort(updateMultiplexer);
          return ESB_CLOSED;
        default:
          handleError(error);
          abort(updateMultiplexer);
          return error;
      }
    }

    ESB::Error error;
    int state = _state & SERVER_STATE_MASK;

    switch (state) {
      case SERVER_TRANSACTION_BEGIN:
        error = stateBeginTransaction();
        break;
      case SERVER_PARSING_HEADERS:
        error = stateReceiveRequestHeaders();
        break;
      case SERVER_PARSING_BODY:
        error = stateReceiveRequestBody(handler);
        break;
      case SERVER_SKIPPING_TRAILER:
        error = stateSkipTrailer();
        break;
      case SERVER_FORMATTING_HEADERS:
        error = stateSendResponseHeaders();
        break;
      case SERVER_FORMATTING_BODY:
        error = stateSendResponseBody(handler);
        break;
      case SERVER_FLUSHING_BODY:
        if (ESB_AGAIN == (error = stateFlushResponseBody())) {
          return ESB_AGAIN;  // don't try to write to a socket with full send buffers twice in a row.
        }
        break;
      case SERVER_TRANSACTION_END:
        error = stateEndTransaction();
        break;
      default:
        assert(!"invalid state");
        ESB_LOG_WARNING_ERRNO(ESB_INVALID_STATE, "[%s] invalid transition to state %d", _socket->name(), state);
        abort(updateMultiplexer);
        return ESB_INVALID_STATE;
    }

    switch (error) {
      case ESB_CLEANUP:
        // close connection
        return ESB_CLEANUP;
      case ESB_SUCCESS:
        break;
      case ESB_BREAK:
        // For proactive/synchronous calls, the handler has finished sending or receiving the requested amount of data
        return ESB_SUCCESS;
      case ESB_AGAIN:
        fillRecvBuffer = inRecvState;
        drainSendBuffer = inSendState;
        break;
      case ESB_PAUSE:
        // The handler cannot make progress
        return ESB_PAUSE;
      default:
        abort(updateMultiplexer);
        return error;
    }

    if (drainSendBuffer) {
      switch (error = flushSendBuffer()) {
        case ESB_SUCCESS:
          drainSendBuffer = false;
          break;
        case ESB_AGAIN:
          return ESB_AGAIN;
        default:
          abort(updateMultiplexer);
          return error;
      }
    }
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown", _socket->name());
  abort(updateMultiplexer);
  return ESB_SHUTDOWN;
}

ESB::Error HttpServerSocket::stateBeginTransaction() {
  assert(_state & SERVER_TRANSACTION_BEGIN);
  assert(_transaction);

  // If we don't fully read the request we can't reuse the socket.  Unset this flag once the request has been fully
  // read.
  addFlag(SERVER_CANNOT_REUSE_CONNECTION);
  _transaction->setPeerAddress(_socket->peerAddress());

  switch (ESB::Error error = _handler.beginTransaction(_multiplexer, *this)) {
    case ESB_SUCCESS:
      break;
    case ESB_AGAIN:
    case ESB_PAUSE:
      return ESB_PAUSE;
    case ESB_SEND_RESPONSE:
      stateTransition(SERVER_FORMATTING_HEADERS);
      return ESB_SUCCESS;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborted connection", _socket->name());
      return error;
  }

  conditionalStateTransition(SERVER_TRANSACTION_BEGIN, SERVER_PARSING_HEADERS);

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::stateReceiveRequestHeaders() {
  assert(_transaction);
  assert(_recvBuffer);
  assert(_socket->connected());
  assert(SERVER_PARSING_HEADERS & _state);

  ESB::Error error = _transaction->getParser()->parseHeaders(_recvBuffer, _transaction->request());

  switch (error) {
    case ESB_SUCCESS:
      break;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] need more request header data from stream", _socket->name());
      return ESB_AGAIN;
    default:
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot parse request headers", _socket->name());
      if (IsHttpError(error)) {
        setResponse(400, "Bad Request");
      } else {
        setResponse(500, "Internal Server Error");
      }
      stateTransition(SERVER_FORMATTING_HEADERS);
      return ESB_SUCCESS;
  }

  // parse complete

  if (ESB_DEBUG_LOGGABLE) {
    const char *method = (const char *)_transaction->request().method();
    int major = _transaction->request().httpVersion() / 100;
    int minor = _transaction->request().httpVersion() % 100 / 10;

    switch (_transaction->request().requestUri().type()) {
      case HttpRequestUri::ES_URI_ASTERISK:
        ESB_LOG_DEBUG("[%s] request line: %s * HTTP/%d.%d", _socket->name(), method, major, minor);
        break;
      case HttpRequestUri::ES_URI_HTTP:
      case HttpRequestUri::ES_URI_HTTPS:
        ESB_LOG_DEBUG("[%s] request line: %s %s//%s:%d%s?%s#%s HTTP/%d.%d", _socket->name(), method,
                      _transaction->request().requestUri().typeString(),
                      ESB_SAFE_STR(_transaction->request().requestUri().host()),
                      _transaction->request().requestUri().port(),
                      ESB_SAFE_STR(_transaction->request().requestUri().absPath()),
                      ESB_SAFE_STR(_transaction->request().requestUri().query()),
                      ESB_SAFE_STR(_transaction->request().requestUri().fragment()), major, minor);
        break;
      case HttpRequestUri::ES_URI_OTHER:
        ESB_LOG_DEBUG("[%s] request line: %s %s HTTP/%d.%d", _socket->name(), method,
                      ESB_SAFE_STR(_transaction->request().requestUri().other()), major, minor);
        break;
    }

    HttpHeader *header = (HttpHeader *)_transaction->request().headers().first();
    for (; header; header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("[%s] request header: %s: %s", _socket->name(), ESB_SAFE_STR(header->fieldName()),
                    ESB_SAFE_STR(header->fieldValue()));
    }
  }

  stateTransition(SERVER_PARSING_BODY);

  // TODO - check Expect header and maybe send a 100 Continue

  switch (error = _handler.receiveRequestHeaders(_multiplexer, *this)) {
    case ESB_SUCCESS:
      return ESB_SUCCESS;
    case ESB_PAUSE:
    case ESB_AGAIN:
      return ESB_PAUSE;
    case ESB_SEND_RESPONSE:
      conditionalStateTransition(SERVER_PARSING_BODY, SERVER_FORMATTING_HEADERS);
      return ESB_SUCCESS;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] Server request header handler aborting connection", _socket->name());
      return error;
  }
}

ESB::Error HttpServerSocket::stateReceiveRequestBody(HttpServerHandler &handler) {
  assert(_transaction);
  assert(_socket->connected());
  assert(_recvBuffer);
  assert(_state & SERVER_PARSING_BODY);

  //
  // Until the body is read or either the parser or handler return ESB_AGAIN,
  // ask the parser how much body data is ready to be read, pass the available
  // body data to the handler, and pass back the body data actually consumed to
  // the parser.
  //

  while (!_multiplexer.shutdown()) {
    ESB::UInt32 bytesAvailable = 0U;
    ESB::UInt32 bytesConsumed = 0U;
    ESB::Error error = ESB_SUCCESS;

    if (ESB_SUCCESS != (error = currentChunkBytesAvailable(&bytesAvailable))) {
      return error;
    }

    // if last chunk
    if (0 == bytesAvailable) {
      ESB_LOG_DEBUG("[%s] offering last chunk", _socket->name());
      stateTransition(SERVER_SKIPPING_TRAILER);
      unsigned char byte = 0;
      switch (error = handler.consumeRequestBody(_multiplexer, *this, &byte, 0U, &bytesConsumed)) {
        case ESB_BREAK:
          return ESB_BREAK;
        case ESB_SUCCESS:
        case ESB_SEND_RESPONSE:
          return ESB_SUCCESS;
        case ESB_PAUSE:
        case ESB_AGAIN:
          return ESB_PAUSE;
        default:
          ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborting connection after last request body chunk", _socket->name());
          return error;
      }
    }

    ESB_LOG_DEBUG("[%s] offering request chunk of size %u", _socket->name(), bytesAvailable);

    switch (error = handler.consumeRequestBody(_multiplexer, *this, _recvBuffer->buffer() + _recvBuffer->readPosition(),
                                               bytesAvailable, &bytesConsumed)) {
      case ESB_BREAK:
        return ESB_BREAK;
      case ESB_SUCCESS:
        break;
      case ESB_AGAIN:
      case ESB_PAUSE:
        return ESB_PAUSE;
      case ESB_SEND_RESPONSE:
        ESB_LOG_DEBUG("[%s] handler sending response before last request body chunk", _socket->name());
        conditionalStateTransition(SERVER_PARSING_BODY, SERVER_FORMATTING_HEADERS);
        return ESB_SUCCESS;
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborting connection before last request body chunk", _socket->name());
        return error;
    }

    error = _transaction->getParser()->consumeBody(_recvBuffer, bytesConsumed);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot consume %u request chunk bytes", _socket->name(), bytesAvailable);
      return error;  // remove from multiplexer
    }
    _bytesAvailable -= bytesConsumed;

    ESB_LOG_DEBUG("[%s] handler consumed %u out of %u request chunk bytes", _socket->name(), bytesConsumed,
                  bytesAvailable);
  }

  return ESB_SHUTDOWN;
}

ESB::Error HttpServerSocket::stateSkipTrailer() {
  assert(_transaction);
  assert(_socket->connected());
  assert(_state & SERVER_SKIPPING_TRAILER);

  ESB::Error error = _transaction->getParser()->skipTrailer(_recvBuffer);

  switch (error) {
    case ESB_SUCCESS:
      // Request has been fully read, so we can reuse the connection, but don't start reading the next request until the
      // response has been fully sent.
      clearFlag(SERVER_CANNOT_REUSE_CONNECTION);
      stateTransition(SERVER_FORMATTING_HEADERS);
      ESB_LOG_DEBUG("[%s] request trailer skipped", _socket->name());
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] need more data from stream to skip trailer", _socket->name());
      return ESB_AGAIN;
    default:
      ESB_LOG_INFO_ERRNO(error, "[%s] error skipping trailer", _socket->name());
      return error;
  }
}

ESB::Error HttpServerSocket::stateSendResponseHeaders() {
  assert(_transaction);
  assert(_socket->connected());
  assert(SERVER_FORMATTING_HEADERS & _state);

  if (!_sendBuffer) {
    _sendBuffer = _multiplexer.acquireBuffer();
    if (!_sendBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer", _socket->name());
      return ESB_OUT_OF_MEMORY;  // remove from multiplexer
    }
  }

  if (ESB_DEBUG_LOGGABLE) {
    HttpResponse &response = _transaction->response();
    int major = response.httpVersion() / 100;
    int minor = response.httpVersion() % 100 / 10;
    int statusCode = response.statusCode();
    const char *reasonPhrase = ESB_SAFE_STR(response.reasonPhrase());
    ESB_LOG_DEBUG("[%s] sending response status-line: HTTP%d/%d %d %s", _socket->name(), major, minor, statusCode,
                  reasonPhrase);

    for (HttpHeader *header = (HttpHeader *)response.headers().first(); header; header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("[%s] response header: %s: %s", _socket->name(), ESB_SAFE_STR(header->fieldName()),
                    ESB_SAFE_STR(header->fieldValue()));
    }
  }

  ESB::Error error = _transaction->getFormatter()->formatHeaders(_sendBuffer, _transaction->response());

  switch (error) {
    case ESB_SUCCESS:
      ESB_LOG_DEBUG("[%s] formatted response headers", _socket->name());
      stateTransition(SERVER_FORMATTING_BODY);
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] partially formatted response headers", _socket->name());
      return ESB_AGAIN;
    default:
      ESB_LOG_CRITICAL_ERRNO(error, "[%s] error formatting response header", _socket->name());
      return error;
  }
}

ESB::Error HttpServerSocket::stateSendResponseBody(HttpServerHandler &handler) {
  assert(_transaction);
  assert(_socket->connected());
  assert(_sendBuffer);
  assert(SERVER_FORMATTING_BODY & _state);

  while (!_multiplexer.shutdown()) {
    ESB::UInt32 maxChunkSize = 0;
    ESB::UInt32 offeredSize = 0;

    ESB::Error error = handler.offerResponseBody(_multiplexer, *this, &offeredSize);
    switch (error) {
      case ESB_CLEANUP:
        return ESB_CLEANUP;
      case ESB_AGAIN:
      case ESB_PAUSE:
        return ESB_PAUSE;
      case ESB_BREAK:
        return ESB_BREAK;
      case ESB_SUCCESS:
        ESB_LOG_DEBUG("[%s] handler offers response chunk of %u bytes", _socket->name(), offeredSize);
        break;
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler error offering response chunk", _socket->name());
        return error;
    }

    if (0 == offeredSize) {
      ESB_LOG_DEBUG("[%s] sending last chunk", _socket->name());
      error = _transaction->getFormatter()->endBody(_sendBuffer);
      switch (error) {
        case ESB_SUCCESS:
          stateTransition(SERVER_FLUSHING_BODY);
          return ESB_SUCCESS;
        case ESB_AGAIN:
          ESB_LOG_DEBUG("[%s] insufficient space in send buffer to format end body", _socket->name());
          return ESB_AGAIN;
        default:
          ESB_LOG_INFO_ERRNO(error, "[%s] error formatting last response chunk", _socket->name());
          return error;
      }
    }

    if (ESB_SUCCESS != (error = formatStartChunk(offeredSize, &maxChunkSize))) {
      return error;
    }

    ESB::UInt32 chunkSize = MIN(offeredSize, maxChunkSize);

    // ask the handler to produce chunkSize bytes of body data

    // TODO KEEF extract bytes actually produce and pass to buffer instead of asserting the full amount is produced.

    switch (error = handler.produceResponseBody(_multiplexer, *this,
                                                _sendBuffer->buffer() + _sendBuffer->writePosition(), chunkSize)) {
      case ESB_SUCCESS:
      case ESB_AGAIN:
      case ESB_PAUSE:
        break;
      default:
        ESB_LOG_INFO_ERRNO(error, "[%s] cannot format response chunk of size %u", _socket->name(), chunkSize);
        return error;
    }

    _sendBuffer->setWritePosition(_sendBuffer->writePosition() + chunkSize);
    _bodyBytesWritten += chunkSize;
    ESB_LOG_DEBUG("[%s] formatted response chunk of size %u", _socket->name(), chunkSize);

    // beginBlock reserves space for this operation, it should never fail
    if (ESB_SUCCESS != (error = formatEndChunk())) {
      return error;
    }
  }

  return ESB_SHUTDOWN;
}

ESB::Error HttpServerSocket::stateFlushResponseBody() {
  assert(_transaction);
  assert(_socket->connected());
  assert(_sendBuffer);
  assert(SERVER_FLUSHING_BODY & _state);

  ESB::Error error = flushSendBuffer();
  if (unlikely(ESB_SUCCESS != error)) {
    return error;
  }

  stateTransition(SERVER_TRANSACTION_END);
  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::stateEndTransaction() {
  assert(_state & SERVER_TRANSACTION_END);

  ++_requestsPerConnection;
  _handler.endTransaction(_multiplexer, *this, HttpServerHandler::ES_HTTP_SERVER_HANDLER_END);

  if (SERVER_CANNOT_REUSE_CONNECTION & _state) {
    return ESB_CLEANUP;
  }

  if (CloseAfterErrorResponse && 300 <= _transaction->response().statusCode()) {
    return ESB_CLEANUP;
  }

  // TODO - close connection if max requests sent on connection

  if (!_transaction->request().reuseConnection()) {
    return ESB_CLEANUP;
  }

  // TODO release buffers in between transactions

  stateTransition(SERVER_TRANSACTION_BEGIN);
  _bodyBytesWritten = 0;
  _transaction->reset();
  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::currentChunkBytesAvailable(ESB::UInt32 *bytesAvailable) {
  if (0 < _bytesAvailable) {
    *bytesAvailable = _bytesAvailable;
    return ESB_SUCCESS;
  }

  if (_state & SERVER_LAST_CHUNK_RECEIVED || !_transaction->request().hasBody()) {
    *bytesAvailable = 0;
    return ESB_SUCCESS;
  }

  ESB::UInt32 bufferOffset = 0U;  // TODO can probably kill bufferOffset entirely
  switch (ESB::Error error = _transaction->getParser()->parseBody(_recvBuffer, &bufferOffset, bytesAvailable)) {
    case ESB_SUCCESS:
      _bytesAvailable = *bytesAvailable;
      if (0 == _bytesAvailable) {
        ESB_LOG_DEBUG("[%s] parsed last chunk", _socket->name());
        addFlag(SERVER_LAST_CHUNK_RECEIVED);
      } else {
        ESB_LOG_DEBUG("[%s] %u request bytes available in current chunk", _socket->name(), *bytesAvailable);
      }
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] insufficient bytes available in current chunk", _socket->name());
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] error parsing request body", _socket->name());
      return error;  // remove from multiplexer
  }
}

ESB::Error HttpServerSocket::formatStartChunk(ESB::UInt32 chunkSize, ESB::UInt32 *maxChunkSize) {
  ESB::Error error = _transaction->getFormatter()->beginBlock(_sendBuffer, chunkSize, maxChunkSize);
  switch (error) {
    case ESB_SUCCESS:
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] insufficient send buffer space to begin chunk", _socket->name());
      return ESB_AGAIN;
    default:
      ESB_LOG_INFO_ERRNO(error, "[%s] error formatting response body", _socket->name());
      return error;
  }
}

ESB::Error HttpServerSocket::formatEndChunk() {
  ESB::Error error = _transaction->getFormatter()->endBlock(_sendBuffer);

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot format response chunk end block", _socket->name());
    return error;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::fillReceiveBuffer() {
  if (!_recvBuffer) {
    // TODO peek for received data. If no data in socket, then return EAGAIN before allocating the recv buffer
    _recvBuffer = _multiplexer.acquireBuffer();
    if (!_recvBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer", _socket->name());
      return ESB_OUT_OF_MEMORY;  // remove from multiplexer
    }
  }

  if (!_transaction) {
    _transaction = _multiplexer.createServerTransaction();
    if (!_transaction) {
      _multiplexer.releaseBuffer(_recvBuffer);
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create server trans", _socket->name());
      return ESB_OUT_OF_MEMORY;  // remove from multiplexer
    }
  }

  // If there is no data in the recv buffer, read some more from the socket
  // If there is no space left in the recv buffer, make room if possible
  if (!_recvBuffer->isWritable()) {
    ESB_LOG_DEBUG("[%s] compacting input buffer", _socket->name());
    if (!_recvBuffer->compact()) {
      ESB_LOG_INFO("[%s] parser jammed", _socket->name());
      return ESB_OVERFLOW;
    }
  }

  // And read from the socket
  assert(_recvBuffer->isWritable());
  ESB::SSize result = _socket->receive(_recvBuffer);

  if (0 > result) {
    ESB::Error error = ESB::LastError();
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot refill recv buffer", _socket->name());
    return error;
  }

  if (0 == result) {
    ESB_LOG_DEBUG("[%s] connection closed during recv buffer refill", _socket->name());
    return ESB_CLOSED;
  }

  ESB_LOG_DEBUG("[%s] read %ld bytes into recv buffer", _socket->name(), result);
  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::flushSendBuffer() {
  assert(_transaction);
  assert(_sendBuffer);

  ESB_LOG_DEBUG("[%s] flushing response output buffer", _socket->name());

  if (!_sendBuffer->isReadable()) {
    ESB_LOG_INFO("[%s] response formatter jammed", _socket->name());
    return ESB_OVERFLOW;  // remove from multiplexer
  }

  while (!_multiplexer.shutdown() && _sendBuffer->isReadable()) {
    ESB::SSize bytesSent = _socket->send(_sendBuffer);

    if (0 > bytesSent) {
      if (ESB_AGAIN == bytesSent) {
        ESB_LOG_DEBUG("[%s] would block flushing response output buffer", _socket->name());
        return ESB_AGAIN;  // keep in multiplexer
      }

      ESB::Error error = ESB::LastError();
      assert(ESB_SUCCESS != error);
      ESB_LOG_INFO_ERRNO(error, "[%s] error flushing response output buffer", _socket->name());
      return error;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("[%s] flushed %ld bytes from response output buffer", _socket->name(), bytesSent);
  }

  return _sendBuffer->isReadable() ? ESB_SHUTDOWN : ESB_SUCCESS;
}

ESB::Error HttpServerSocket::setResponse(int statusCode, const char *reasonPhrase) {
  _transaction->response().setStatusCode(statusCode);
  _transaction->response().setReasonPhrase(reasonPhrase);
  _transaction->response().setHasBody(false);

  ESB::Error error = _transaction->response().addHeader("Content-Length", "0", _transaction->allocator());

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot create %d response", _socket->name(), statusCode);
  }

  if (110 <= _transaction->request().httpVersion() &&
      (!_transaction->request().reuseConnection() || CloseAfterErrorResponse)) {
    error = _transaction->response().addHeader("Connection", "close", _transaction->allocator());

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot create %d response", _socket->name(), statusCode);
    }
  }

  ESB_LOG_DEBUG("[%s] created response: %d %s", _socket->name(), _transaction->response().statusCode(),
                _transaction->response().reasonPhrase());
  return ESB_SUCCESS;
}

const char *HttpServerSocket::logAddress() const { return _socket->name(); }

ESB::Error HttpServerSocket::abort(bool updateMultiplexer) {
  assert(!(SERVER_ABORTED & _state));
  if (_state & SERVER_ABORTED) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] server connection aborted", _socket->name());

  addFlag(SERVER_ABORTED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().removeMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot abort server connection", _socket->name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::pauseRecv(bool updateMultiplexer) {
  assert(!(SERVER_INACTIVE & _state));
  assert(!(SERVER_ABORTED & _state));
  if (_state & (SERVER_INACTIVE | SERVER_ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (_state & SERVER_RECV_PAUSED) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] pausing server request receive", _socket->name());

  addFlag(SERVER_RECV_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause server response receive", _socket->name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::resumeRecv(bool updateMultiplexer) {
  assert(!(SERVER_INACTIVE & _state));
  assert(!(SERVER_ABORTED & _state));
  if (_state & (SERVER_INACTIVE | SERVER_ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (!(_state & SERVER_RECV_PAUSED)) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] resuming server request receive", _socket->name());

  clearFlag(SERVER_RECV_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume server response receive", _socket->name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::pauseSend(bool updateMultiplexer) {
  assert(!(SERVER_INACTIVE & _state));
  assert(!(SERVER_ABORTED & _state));
  if (_state & (SERVER_INACTIVE | SERVER_ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (_state & SERVER_SEND_PAUSED) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] pausing server response send", _socket->name());

  addFlag(SERVER_SEND_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause server response send", _socket->name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::resumeSend(bool updateMultiplexer) {
  assert(!(SERVER_INACTIVE & _state));
  assert(!(SERVER_ABORTED & _state));
  if (_state & (SERVER_INACTIVE | SERVER_ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (!(_state & SERVER_SEND_PAUSED)) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] resuming server request send", _socket->name());

  clearFlag(SERVER_SEND_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume server response send", _socket->name());
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

const ESB::SocketAddress &HttpServerSocket::peerAddress() const { return _socket->peerAddress(); }

const char *HttpServerSocket::name() const { return _socket->name(); }

void HttpServerSocket::stateTransition(int state) {
  assert(state & SERVER_STATE_MASK);
  assert(!(state & SERVER_FLAG_MASK));
#ifndef NDEBUG
  int currentState = _state & SERVER_STATE_MASK;
#endif

  switch (state & SERVER_STATE_MASK) {
    case SERVER_INACTIVE:
      assert(!(currentState & SERVER_INACTIVE));
      _state = SERVER_INACTIVE;
      ESB_LOG_DEBUG("[%s] connection removed", _socket->name());
      break;
    case SERVER_PARSING_HEADERS:
      assert(currentState & SERVER_TRANSACTION_BEGIN);
      _state &= ~SERVER_TRANSACTION_BEGIN;
      _state |= SERVER_PARSING_HEADERS;
      ESB_LOG_DEBUG("[%s] parsing headers", _socket->name());
      break;
    case SERVER_PARSING_BODY:
      assert(currentState & SERVER_PARSING_HEADERS);
      _state &= ~SERVER_PARSING_HEADERS;
      _state |= SERVER_PARSING_BODY;
      ESB_LOG_DEBUG("[%s] parsing body", _socket->name());
      break;
    case SERVER_SKIPPING_TRAILER:
      assert(currentState & SERVER_PARSING_BODY);
      _state &= ~SERVER_PARSING_BODY;
      _state |= SERVER_SKIPPING_TRAILER;
      ESB_LOG_DEBUG("[%s] skipping trailer", _socket->name());
      break;
    case SERVER_FORMATTING_HEADERS:
      assert(currentState &
             (SERVER_TRANSACTION_BEGIN | SERVER_PARSING_HEADERS | SERVER_PARSING_BODY | SERVER_SKIPPING_TRAILER));
      _state &= ~(SERVER_TRANSACTION_BEGIN | SERVER_PARSING_HEADERS | SERVER_PARSING_BODY | SERVER_SKIPPING_TRAILER);
      _state |= SERVER_FORMATTING_HEADERS;
      ESB_LOG_DEBUG("[%s] formatting headers", _socket->name());
      break;
    case SERVER_FORMATTING_BODY:
      assert(currentState & SERVER_FORMATTING_HEADERS);
      _state &= ~SERVER_FORMATTING_HEADERS;
      _state |= SERVER_FORMATTING_BODY;
      ESB_LOG_DEBUG("[%s] formatting body", _socket->name());
      break;
    case SERVER_FLUSHING_BODY:
      assert(currentState & SERVER_FORMATTING_BODY);
      _state &= ~SERVER_FORMATTING_BODY;
      _state |= SERVER_FLUSHING_BODY;
      ESB_LOG_DEBUG("[%s] flushing body", _socket->name());
      break;
    case SERVER_TRANSACTION_BEGIN:
      assert(currentState & SERVER_TRANSACTION_END);
      _state = SERVER_TRANSACTION_BEGIN;
      ESB_LOG_DEBUG("[%s] transaction begun", _socket->name());
      break;
    case SERVER_TRANSACTION_END:
      assert(currentState & SERVER_FLUSHING_BODY);
      _state &= ~SERVER_FLUSHING_BODY;
      _state |= SERVER_TRANSACTION_END;
      ESB_LOG_DEBUG("[%s] transaction complete", _socket->name());
      break;
    default:
      assert(0 == "Cannot transition into unknown state");
      ESB_LOG_ERROR("[%s] cannot transition to multiple states: %d", _socket->name(), state);
  }
}

ESB::Error HttpServerSocket::updateInterestList(bool updateMultiplexer) {
  if (!updateMultiplexer) {
    return ESB_SUCCESS;
  }

  ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);

  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot update interest list", _socket->name());
    return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
  }

  return ESB_SUCCESS;
}

const void *HttpServerSocket::key() const { return _socket->key(); }

}  // namespace ES
