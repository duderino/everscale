#ifndef ES_HTTP_CLIENT_SOCKET_H
#include <ESHttpClientSocket.h>
#endif

namespace ES {

// TODO - add performance counters
// TODO - max requests per connection option (1 disables keepalives)
// TODO - max header size option
// TODO - max body size option

bool HttpClientSocket::_ReuseConnections = true;

/**
 * The state machine is built around the concept of a set of callback functions that produce and consume on demand, but
 * some APIs support direct passing in of buffers.  This class wraps a buffer that was directly passed in with a
 * callback adaptor that can be subsequently invoked by the state machine.
 *
 */
class HttpRequestBodyProducer : public HttpClientHandler {
 public:
  HttpRequestBodyProducer(unsigned const char *buffer, ESB::UInt64 size)
      : _buffer(buffer), _size(size), _bytesProduced(0) {}
  virtual ~HttpRequestBodyProducer() {}

  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error receiveResponseHeaders(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                      ESB::UInt64 *bytesAvailable) {
    if (!bytesAvailable) {
      return ESB_NULL_POINTER;
    }

    if (!_size) {
      // end body, transition to flushing state
      *bytesAvailable = 0;
      return ESB_SUCCESS;
    }

    *bytesAvailable = _size - _bytesProduced;
    if (0 == *bytesAvailable) {
      return ESB_BREAK;
    } else {
      return ESB_SUCCESS;
    }
  }

  virtual ESB::Error produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                        unsigned char *body, ESB::UInt64 bytesRequested) {
    assert(bytesRequested <= _size - _bytesProduced);
    if (bytesRequested > _size - _bytesProduced) {
      return ESB_INVALID_ARGUMENT;
    }

    memcpy(body, _buffer + _bytesProduced, bytesRequested);
    _bytesProduced += bytesRequested;
    return ESB_SUCCESS;
  }

  virtual ESB::Error consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                         const unsigned char *body, ESB::UInt64 bytesOffered,
                                         ESB::UInt64 *bytesConsumed) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error endRequest(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream, State state) {
    assert(!"function should not be called");
  }

  virtual void dumpClientCounters(ESB::Logger &logger, ESB::Logger::Severity severity) const {
    assert(!"function should not be called");
  }

  inline ESB::UInt64 bytesProduced() const { return _bytesProduced; };

 private:
  HttpRequestBodyProducer(const HttpRequestBodyProducer &disabled);
  void operator=(const HttpRequestBodyProducer &disabled);

  unsigned const char *_buffer;
  ESB::UInt64 _size;
  ESB::UInt64 _bytesProduced;
};

/**
 * The state machine is built around the concept of a set of callback functions that produce and consume on demand, but
 * some APIs support direct passing in of buffers.  This class wraps a buffer that was directly passed in with a
 * callback adaptor that can be subsequently invoked by the state machine.
 *
 */
class HttpResponseBodyConsumer : public HttpClientHandler {
 public:
  HttpResponseBodyConsumer(unsigned const char *buffer, ESB::UInt64 size)
      : _buffer(buffer), _size(size), _bytesConsumed(0), _bytesOffered(0) {}
  virtual ~HttpResponseBodyConsumer() {}

  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error receiveResponseHeaders(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                      ESB::UInt64 *bytesAvailable) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                        unsigned char *body, ESB::UInt64 bytesRequested) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                         const unsigned char *body, ESB::UInt64 bytesOffered,
                                         ESB::UInt64 *bytesConsumed) {
    if (!body || !bytesConsumed) {
      return ESB_NULL_POINTER;
    }

    _bytesOffered = bytesOffered;
    ESB::UInt64 bytesToCopy = MIN(_size - _bytesConsumed, bytesOffered);

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

  virtual ESB::Error endRequest(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream, State state) {
    assert(!"function should not be called");
  }

  virtual void dumpClientCounters(ESB::Logger &logger, ESB::Logger::Severity severity) const {
    assert(!"function should not be called");
  }

  inline ESB::UInt64 bytesConsumed() const { return _bytesConsumed; };

  inline ESB::UInt64 bytesOffered() const { return _bytesOffered; }

 private:
  HttpResponseBodyConsumer(const HttpResponseBodyConsumer &disabled);
  void operator=(const HttpResponseBodyConsumer &disabled);

  unsigned const char *_buffer;
  ESB::UInt64 _size;
  ESB::UInt64 _bytesConsumed;
  ESB::UInt64 _bytesOffered;
};

HttpClientSocket::HttpClientSocket(bool reused, HttpClientTransaction *transaction, ESB::ConnectedSocket *socket,
                                   HttpClientHandler &handler, HttpMultiplexerExtended &multiplexer,
                                   HttpConnectionMetrics &connectionMetrics, ESB::CleanupHandler &cleanupHandler)
    : _state(CONNECTING),
      _requestsPerConnection(0),
      _bodyBytesWritten(0),
      _bytesAvailable(0),
      _multiplexer(multiplexer),
      _handler(handler),
  _connectionMetrics(connectionMetrics),
      _transaction(transaction),
      _cleanupHandler(cleanupHandler),
      _recvBuffer(NULL),
      _sendBuffer(NULL),
      _socket(socket) {
  if (reused) {
    assert(connected());
    stateTransition(TRANSACTION_BEGIN);
    addFlag(FIRST_USE_AFTER_REUSE);
  }
}

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

bool HttpClientSocket::wantAccept() { return false; }

bool HttpClientSocket::wantConnect() { return (_state & CONNECTING) != 0; }

bool HttpClientSocket::wantRead() {
  if (_state & (RECV_PAUSED | ABORTED | INACTIVE)) {
    return false;
  }

  if (_socket->wantRead()) {
    return true;
  }

  if (_socket->wantWrite()) {
    return false;
  }

  return _state & RECV_STATE_MASK;
}

bool HttpClientSocket::wantWrite() {
  if (_state & (SEND_PAUSED | ABORTED | INACTIVE)) {
    return false;
  }

  if (_socket->wantWrite()) {
    return true;
  }

  if (_socket->wantRead()) {
    return false;
  }

  return _state & (TRANSACTION_BEGIN | SEND_STATE_MASK);
}

ESB::Error HttpClientSocket::handleAccept() {
  assert(!(INACTIVE & _state));
  ESB_LOG_ERROR("[%d] Cannot handle accept events", _socket->socketDescriptor());
  return ESB_INVALID_STATE;  // remove from multiplexer
}

ESB::Error HttpClientSocket::handleConnect() {
  assert(!(ABORTED & _state));
  assert(_socket->connected());

  _connectionMetrics.totalConnections().inc();
  ESB_LOG_INFO("[%s] connected to peer", _socket->name());

  stateTransition(TRANSACTION_BEGIN);
  return handleWritable();
}

ESB::Error HttpClientSocket::currentChunkBytesAvailable(ESB::UInt64 *bytesAvailable) {
  if (0 < _bytesAvailable) {
    *bytesAvailable = _bytesAvailable;
    return ESB_SUCCESS;
  }

  if (_state & LAST_CHUNK_RECEIVED || !_transaction->response().hasBody()) {
    *bytesAvailable = 0;
    return ESB_SUCCESS;
  }

  ESB::UInt64 bufferOffset = 0U;  // TODO can probably kill bufferOffset entirely
  switch (ESB::Error error = _transaction->getParser()->parseBody(_recvBuffer, &bufferOffset, bytesAvailable)) {
    case ESB_SUCCESS:
      ESB_LOG_DEBUG("[%s] %lu response bytes available in current chunk", _socket->name(), *bytesAvailable);
      _bytesAvailable = *bytesAvailable;
      if (0 == _bytesAvailable) {
        ESB_LOG_DEBUG("[%s] parsed last chunk", _socket->name());
        addFlag(LAST_CHUNK_RECEIVED);
      }
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] insufficient bytes available in current chunk", _socket->name());
      return ESB_AGAIN;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] error parsing response body", _socket->name());
      return error;  // remove from multiplexer
  }
}

ESB::Error HttpClientSocket::responseBodyAvailable(ESB::UInt64 *bytesAvailable) {
  if (!bytesAvailable) {
    return ESB_NULL_POINTER;
  }

  HttpResponseBodyConsumer adaptor(NULL, 0);

  ESB::Error error = advanceStateMachine(adaptor, UPDATE_MULTIPLEXER | ADVANCE_RECV);
  *bytesAvailable = adaptor.bytesOffered();
  return error;
}

ESB::Error HttpClientSocket::readResponseBody(unsigned char *chunk, ESB::UInt64 bytesRequested,
                                              ESB::UInt64 *bytesRead) {
  assert(chunk);
  if (!chunk) {
    return ESB_NULL_POINTER;
  }

  HttpResponseBodyConsumer adaptor(chunk, bytesRequested);
  ESB::Error error = advanceStateMachine(adaptor, UPDATE_MULTIPLEXER | ADVANCE_RECV);
  *bytesRead = adaptor.bytesConsumed();

  return error;
}

ESB::Error HttpClientSocket::handleReadable() {
  assert(wantRead());
  assert(_socket->connected());
  assert(_transaction);

  if (!wantRead() || !_socket->connected() || !_transaction) {
    return ESB_INVALID_STATE;
  }

  return advanceStateMachine(_handler, INITIAL_FILL_RECV_BUFFER | ADVANCE_RECV | ADVANCE_SEND);
}

ESB::Error HttpClientSocket::sendRequestBody(unsigned const char *chunk, ESB::UInt64 bytesOffered,
                                             ESB::UInt64 *bytesConsumed) {
  if (!chunk || !bytesConsumed) {
    return ESB_NULL_POINTER;
  }

  if (!(_state & FORMATTING_BODY)) {
    return ESB_INVALID_STATE;
  }

  HttpRequestBodyProducer adaptor(chunk, bytesOffered);

  ESB::Error error = advanceStateMachine(adaptor, UPDATE_MULTIPLEXER | ADVANCE_SEND);
  *bytesConsumed = adaptor.bytesProduced();
  return error;
}

ESB::Error HttpClientSocket::handleWritable() {
  assert(wantWrite());
  assert(_socket->connected());
  assert(_transaction);

  if (!wantWrite() || !_socket->connected() || !_transaction) {
    return ESB_INVALID_STATE;
  }

  // TODO measure impact of adding INITIAL_DRAIN_SEND_BUFFER here
  return advanceStateMachine(_handler, ADVANCE_RECV | ADVANCE_SEND);
}

void HttpClientSocket::handleError(ESB::Error error) {
#ifdef ESB_CI_BUILD
  // make it easier to debug CI build failures
  ESB_LOG_WARNING_ERRNO(error, "[%s] socket error", _socket->name());
#else
  ESB_LOG_INFO_ERRNO(error, "[%s] socket error", _socket->name());
#endif
  assert(!(INACTIVE & _state));
}

void HttpClientSocket::handleRemoteClose() {
  // TODO - this may just mean the client closed its half of the socket but is still expecting a response.
#ifdef ESB_CI_BUILD
  ESB_LOG_WARNING("[%s] remote server closed socket", _socket->name());
#else
  ESB_LOG_INFO("[%s] remote server closed socket", _socket->name());
#endif
  assert(!(INACTIVE & _state));
}

void HttpClientSocket::handleIdle() {
#ifdef ESB_CI_BUILD
  // make it easier to debug CI build failures
  ESB_LOG_WARNING("[%s] idle (state=%s, flags=%s)", _socket->name(), describeState(), describeFlags());
#else
  ESB_LOG_INFO("[%s] idle (state=%s, flags=%s)", _socket->name(), describeState(), describeFlags());
#endif
  assert(!(INACTIVE & _state));
}

void HttpClientSocket::handleRemove() {
  assert(!(INACTIVE & _state));
  ESB_LOG_INFO("[%s] client socket has been removed", _socket->name());

  if (_sendBuffer) {
    _multiplexer.releaseBuffer(_sendBuffer);
    _sendBuffer = NULL;
  }
  if (_recvBuffer) {
    _multiplexer.releaseBuffer(_recvBuffer);
    _recvBuffer = NULL;
  }

  if (_state & FIRST_USE_AFTER_REUSE && !(_state & ABORTED)) {
    ESB_LOG_DEBUG("[%s] closing stale connection and retrying transaction", _socket->name());
    _socket->close();
    assert(_transaction);
    ESB::Error error = _multiplexer.executeClientTransaction(_transaction);
    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] Cannot retry transaction", _socket->name());
      _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_BEGIN);
      _multiplexer.destroyClientTransaction(_transaction);
    }
    _transaction = NULL;
    stateTransition(INACTIVE);
    return;
  }

  bool reuseConnection = false;
  int state = _state & STATE_MASK;

  switch (state) {
    case TRANSACTION_BEGIN:
      assert(_transaction);
      _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_BEGIN);
      break;
    case CONNECTING:
      assert(_transaction);
      _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONNECT);
      break;
    case FORMATTING_HEADERS:
      assert(_transaction);
      _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS);
      break;
    case FORMATTING_BODY:
    case FLUSHING_BODY:
      assert(_transaction);
      _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY);
      break;
    case PARSING_HEADERS:
      assert(_transaction);
      _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS);
      break;
    case PARSING_BODY:
      assert(_transaction);
      _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY);
      break;
    case TRANSACTION_END:
      assert(_transaction);
      ++_requestsPerConnection;
      if (GetReuseConnections() && !(_state & ABORTED)) {
        const HttpHeader *header = _transaction->response().findHeader("Connection");
        reuseConnection = !(header && header->fieldValue() && !strcasecmp("close", (const char *)header->fieldValue()));
      }
      _handler.endTransaction(_multiplexer, *this, HttpClientHandler::ES_HTTP_CLIENT_HANDLER_END);
      break;
    default:
      assert(!"socket has invalid state");
      ESB_LOG_WARNING_ERRNO(ESB_INVALID_STATE, "[%s] socket has invalid state '%d'", _socket->name(), state);
  }

  if (_transaction) {
    _multiplexer.destroyClientTransaction(_transaction);
    _transaction = NULL;
  }

  // HttpClientSocketFactory::release() is invoked by the cleanup handler and returns open sockets to the pool.

  if (reuseConnection) {
    ESB_LOG_DEBUG("[%s] adding connection to connection pool", _socket->name());
  } else {
    ESB_LOG_DEBUG("[%s] connection will not be reused", _socket->name());
    _connectionMetrics.averageTransactionsPerConnection().add(_requestsPerConnection);
    _requestsPerConnection = 0;
    _socket->close();
  }

  stateTransition(INACTIVE);
}

SOCKET HttpClientSocket::socketDescriptor() const { return _socket->socketDescriptor(); }

ESB::CleanupHandler *HttpClientSocket::cleanupHandler() { return &_cleanupHandler; }

ESB::Error HttpClientSocket::advanceStateMachine(HttpClientHandler &handler, int flags) {
  bool fillRecvBuffer = flags & INITIAL_FILL_RECV_BUFFER;
  bool drainSendBuffer = flags & INITIAL_DRAIN_SEND_BUFFER;

  while (!_multiplexer.shutdown()) {
    bool inRecvState = _state & RECV_STATE_MASK;
    bool inSendState = _state & SEND_STATE_MASK;

    if (inRecvState && !(flags & ADVANCE_RECV)) {
      return ESB_SUCCESS;
    }

    if (inSendState && !(flags & ADVANCE_SEND)) {
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
          return ESB_CLOSED;
        default:
          handleError(error);
          return error;
      }
    }

    ESB::Error error;
    int state = _state & STATE_MASK;

    switch (state) {
      case TRANSACTION_BEGIN:
        error = stateBeginTransaction();
        break;
      case FORMATTING_HEADERS:
        error = stateSendRequestHeaders();
        break;
      case FORMATTING_BODY:
        error = stateSendRequestBody(handler);
        break;
      case FLUSHING_BODY:
        switch (error = stateFlushRequestBody()) {
          case ESB_AGAIN:
            return ESB_AGAIN;  // don't try to write to a socket with full send buffers twice in a row.
          case ESB_SUCCESS:
            fillRecvBuffer = true;
            break;
          default:
            break;
        }
        break;
      case PARSING_HEADERS:
        error = stateReceiveResponseHeaders();
        break;
      case PARSING_BODY:
        error = stateReceiveResponseBody(handler);
        break;
      case TRANSACTION_END:
        return ESB_SUCCESS;
      default:
        ESB_LOG_WARNING_ERRNO(ESB_INVALID_STATE, "[%s] invalid transition to state %d", _socket->name(), state);
        assert(!"invalid state");
        return ESB_INVALID_STATE;
    }

    switch (error) {
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
          return error;
      }
    }
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown", _socket->name());
  return ESB_SHUTDOWN;
}

ESB::Error HttpClientSocket::stateBeginTransaction() {
  // TODO make connection reuse more configurable
  if (!HttpClientSocket::GetReuseConnections() && !_transaction->request().findHeader("Connection")) {
    ESB::Error error = _transaction->request().addHeader("Connection", "close", _transaction->allocator());

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s] cannot add connection: close header", _socket->name());
      return ESB_AGAIN == error ? ESB_OTHER_ERROR : error;
    }
  }

  // TODO add user agent, etc headers

  ESB::Error error = _handler.beginTransaction(_multiplexer, *this);
  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborted transaction immediately after connecting", _socket->name());
    return ESB_AGAIN == error ? ESB_OTHER_ERROR : error;
  }

  stateTransition(FORMATTING_HEADERS);

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::stateSendRequestHeaders() {
  if (!_sendBuffer) {
    _sendBuffer = _multiplexer.acquireBuffer();
    if (!_sendBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer", _socket->name());
      return ESB_OUT_OF_MEMORY;
    }
  }

  ESB::Error error = _transaction->getFormatter()->formatHeaders(_sendBuffer, _transaction->request());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] partially formatted request headers", _socket->name());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error formatting request header", _socket->name());
    return error;
  }

  stateTransition(FORMATTING_BODY);

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::stateSendRequestBody(HttpClientHandler &handler) {
  assert(_transaction);
  assert(_sendBuffer);

  while (!_multiplexer.shutdown()) {
    ESB::UInt64 chunkSize = 0;
    ESB::UInt64 offeredSize = 0;

    ESB::Error error = handler.offerRequestBody(_multiplexer, *this, &offeredSize);
    switch (error) {
      case ESB_AGAIN:
      case ESB_PAUSE:
        return ESB_PAUSE;
      case ESB_BREAK:
        return ESB_BREAK;
      case ESB_SUCCESS:
        ESB_LOG_DEBUG("[%s] handler offers request chunk of %lu bytes", _socket->name(), offeredSize);
        break;
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler error offering request chunk", _socket->name());
        return error;
    }

    if (0 == offeredSize) {
      ESB_LOG_DEBUG("[%s] sending last chunk", _socket->name());
      error = _transaction->getFormatter()->endBody(_sendBuffer);
      switch (error) {
        case ESB_SUCCESS:
          stateTransition(FLUSHING_BODY);
          return ESB_SUCCESS;
        case ESB_AGAIN:
          ESB_LOG_DEBUG("[%s] insufficient space in send buffer to format end body", _socket->name());
          return ESB_AGAIN;
        default:
          ESB_LOG_INFO_ERRNO(error, "[%s] error formatting last request chunk", _socket->name());
          return error;
      }
    }

    if (ESB_SUCCESS != (error = formatStartChunk(offeredSize, &chunkSize))) {
      return error;
    }

    // ask the handler to produce chunkSize bytes of body data

    switch (error = handler.produceRequestBody(_multiplexer, *this,
                                               _sendBuffer->buffer() + _sendBuffer->writePosition(), chunkSize)) {
      case ESB_SUCCESS:
      case ESB_AGAIN:
      case ESB_PAUSE:
        break;
      default:
        ESB_LOG_INFO_ERRNO(error, "[%s] cannot format request chunk of size %lu", _socket->name(), chunkSize);
        return error;
    }

    _sendBuffer->setWritePosition(_sendBuffer->writePosition() + chunkSize);
    _bodyBytesWritten += chunkSize;
    ESB_LOG_DEBUG("[%s] formatted request chunk of size %lu", _socket->name(), chunkSize);

    // beginBlock reserves space for this operation, it should never fail
    if (ESB_SUCCESS != (error = formatEndChunk())) {
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot end request chunk", _socket->name());
      return error;
    }
  }

  return ESB_SHUTDOWN;
}

ESB::Error HttpClientSocket::stateFlushRequestBody() {
  ESB::Error error = flushSendBuffer();

  if (unlikely(ESB_SUCCESS != error)) {
    return error;
  }

  stateTransition(PARSING_HEADERS);
  error = _handler.endRequest(_multiplexer, *this);
  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborted transaction on request end", _socket->name());
    return error;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::stateReceiveResponseHeaders() {
  ESB::Error error = _transaction->getParser()->parseHeaders(_recvBuffer, _transaction->response());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] need more response header data from stream", _socket->name());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot response parse headers", _socket->name());
    return error;  // remove from multiplexer
  }

  // parse complete

  if (ESB_DEBUG_LOGGABLE) {
    ESB_LOG_DEBUG("[%s] status line: HTTP/%d.%d %d %s", _socket->name(), _transaction->response().httpVersion() / 100,
                  _transaction->response().httpVersion() % 100 / 10, _transaction->response().statusCode(),
                  ESB_SAFE_STR(_transaction->response().reasonPhrase()));
    HttpHeader *header = (HttpHeader *)_transaction->response().headers().first();
    for (; header; header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("[%s] response header: %s: %s", _socket->name(), ESB_SAFE_STR(header->fieldName()),
                    ESB_SAFE_STR(header->fieldValue()));
    }
  }

  stateTransition(PARSING_BODY);

  switch (error = _handler.receiveResponseHeaders(_multiplexer, *this)) {
    case ESB_SUCCESS:
      return ESB_SUCCESS;
    case ESB_AGAIN:
    case ESB_PAUSE:
      return ESB_PAUSE;
    default:
      ESB_LOG_DEBUG_ERRNO(error, "[%s] Client request header handler aborting connection", _socket->name());
      return error;  // remove from multiplexer
  }
}

ESB::Error HttpClientSocket::stateReceiveResponseBody(HttpClientHandler &handler) {
  assert(_transaction);
  assert(_state & PARSING_BODY);

  //
  // Until the body is read or either the parser or handler return ESB_AGAIN,
  // ask the parser how much body data is ready to be read, pass the available
  // body data to the handler, and pass back the body data actually consumed to
  // the parser.
  //

  while (!_multiplexer.shutdown()) {
    ESB::UInt64 bytesOffered = 0U;
    ESB::UInt64 bytesConsumed = 0U;
    ESB::Error error = ESB_SUCCESS;

    if (ESB_SUCCESS != (error = currentChunkBytesAvailable(&bytesOffered))) {
      return error;
    }

    // if last chunk
    if (0 == bytesOffered) {
      ESB_LOG_DEBUG("[%s] offering last chunk", _socket->name());
      stateTransition(TRANSACTION_END);
      unsigned char byte = 0;
      switch (error = handler.consumeResponseBody(_multiplexer, *this, &byte, 0U, &bytesConsumed)) {
        case ESB_BREAK:
          return ESB_BREAK;
        case ESB_SUCCESS:
          return ESB_SUCCESS;
        case ESB_PAUSE:
        case ESB_AGAIN:
          return ESB_PAUSE;
        default:
          ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborting connection after last response body chunk",
                              _socket->name());
          return error;
      }
    }

    ESB_LOG_DEBUG("[%s] offering response chunk of size %lu", _socket->name(), bytesOffered);
    error = handler.consumeResponseBody(_multiplexer, *this, _recvBuffer->buffer() + _recvBuffer->readPosition(),
                                        bytesOffered, &bytesConsumed);

    if (0 < bytesConsumed) {
      ESB::Error error2 = _transaction->getParser()->consumeBody(_recvBuffer, bytesConsumed);
      if (ESB_SUCCESS != error2) {
        ESB_LOG_DEBUG_ERRNO(error2, "[%s] cannot consume %lu response chunk bytes", _socket->name(), bytesOffered);
        return error2;  // remove from multiplexer
      }
      assert(_bytesAvailable >= bytesConsumed);
      _bytesAvailable -= bytesConsumed;

      ESB_LOG_DEBUG("[%s] consumed %lu out of %lu response chunk bytes, %lu bytes still available", _socket->name(),
                    bytesConsumed, bytesOffered, _bytesAvailable);
    }

    switch (error) {
      case ESB_BREAK:
        return ESB_BREAK;
      case ESB_SUCCESS:
        break;
      case ESB_AGAIN:
      case ESB_PAUSE:
        return ESB_PAUSE;
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborting connection before last response body chunk", _socket->name());
        return error;  // remove from multiplexer
    }
  }

  return ESB_SHUTDOWN;
}

ESB::Error HttpClientSocket::fillReceiveBuffer() {
  if (!_transaction) {
    return ESB_INVALID_STATE;
  }

  if (!_recvBuffer) {
    _recvBuffer = _multiplexer.acquireBuffer();
    if (!_recvBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer", _socket->name());
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
    ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot refill client recv buffer", _socket->name());
    return error;
  }

  if (0 == result) {
    ESB_LOG_DEBUG("[%s] connection closed during client recv buffer refill", _socket->name());
    return ESB_CLOSED;
  }

  ESB_LOG_DEBUG("[%s] read %ld bytes into client recv buffer", _socket->name(), result);
  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::flushSendBuffer() {
  ESB_LOG_DEBUG("[%s] flushing request output buffer", _socket->name());

  if (!_sendBuffer->isReadable()) {
    ESB_LOG_INFO("[%s] request formatter jammed", _socket->name());
    return ESB_OVERFLOW;  // remove from multiplexer
  }

  bool flushed = false;
  while (!_multiplexer.shutdown() && _sendBuffer->isReadable()) {
    ESB::SSize bytesSent = _socket->send(_sendBuffer);

    if (0 > bytesSent) {
      ESB::Error error = ESB::LastError();
      if (ESB_AGAIN == error) {
        ESB_LOG_DEBUG_ERRNO(error, "[%s] output socket buffer is full", _socket->name());
        return flushed ? ESB_SUCCESS : ESB_AGAIN;  // keep in multiplexer
      }

      assert(ESB_SUCCESS != error);
      ESB_LOG_INFO_ERRNO(error, "[%s] error flushing response output buffer", _socket->name());
      return error;  // remove from multiplexer
    }

    flushed = true;
    clearFlag(FIRST_USE_AFTER_REUSE);
    ESB_LOG_DEBUG("[%s] flushed %ld bytes from request output buffer", _socket->name(), bytesSent);
  }

  return _multiplexer.shutdown() ? ESB_SHUTDOWN : ESB_SUCCESS;
}

const char *HttpClientSocket::logAddress() const { return _socket->name(); }

const ESB::Date &HttpClientSocket::transactionStartTime() const { return _transaction->startTime(); }

ESB::Error HttpClientSocket::abort(bool updateMultiplexer) {
#ifdef ESB_CI_BUILD
  ESB_LOG_WARNING("[%s] client connection aborted", _socket->name());
#else
  ESB_LOG_DEBUG("[%s] client connection aborted", _socket->name());
#endif

  assert(!(ABORTED & _state));
  if (_state & ABORTED) {
    return ESB_INVALID_STATE;
  }

  addFlag(ABORTED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().removeMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot abort client connection", _socket->name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::pauseRecv(bool updateMultiplexer) {
  assert(!(INACTIVE & _state));
  assert(!(ABORTED & _state));
  if (_state & (INACTIVE | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (_state & RECV_PAUSED) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] pausing client response receive", _socket->name());

  addFlag(RECV_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause client response receive", _socket->name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::resumeRecv(bool updateMultiplexer) {
  assert(!(INACTIVE & _state));
  assert(!(ABORTED & _state));
  if (_state & (INACTIVE | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (!(_state & RECV_PAUSED)) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] resuming client response receive", _socket->name());

  clearFlag(RECV_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume client response receive", _socket->name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::pauseSend(bool updateMultiplexer) {
  assert(!(INACTIVE & _state));
  assert(!(ABORTED & _state));
  if (_state & (INACTIVE | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (_state & SEND_PAUSED) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] pausing client request send", _socket->name());

  addFlag(SEND_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot pause client response send", _socket->name());
      return error == ESB_AGAIN ? ESB_OTHER_ERROR : error;
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpClientSocket::resumeSend(bool updateMultiplexer) {
  assert(!(INACTIVE & _state));
  assert(!(ABORTED & _state));
  if (_state & (INACTIVE | ABORTED)) {
    return ESB_INVALID_STATE;
  }

  if (!(_state & SEND_PAUSED)) {
    return ESB_SUCCESS;
  }

  ESB_LOG_DEBUG("[%s] resuming client request send", _socket->name());

  clearFlag(SEND_PAUSED);
  if (updateMultiplexer) {
    ESB::Error error = _multiplexer.multiplexer().updateMultiplexedSocket(this);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot resume client response send", _socket->name());
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

void *HttpClientSocket::context() { return _transaction ? _transaction->context() : NULL; }

const void *HttpClientSocket::context() const { return _transaction ? _transaction->context() : NULL; }

const ESB::SocketAddress &HttpClientSocket::peerAddress() const { return _socket->peerAddress(); }

ESB::Error HttpClientSocket::formatStartChunk(ESB::UInt64 chunkSize, ESB::UInt64 *maxChunkSize) {
  ESB::Error error = _transaction->getFormatter()->beginBlock(_sendBuffer, chunkSize, maxChunkSize);
  switch (error) {
    case ESB_SUCCESS:
      return ESB_SUCCESS;
    case ESB_AGAIN:
      ESB_LOG_DEBUG("[%s] insufficient send buffer space to begin chunk", _socket->name());
      return ESB_AGAIN;
    default:
      ESB_LOG_INFO_ERRNO(error, "[%s] error formatting request body", _socket->name());
      return error;
  }
}

ESB::Error HttpClientSocket::formatEndChunk() {
  ESB::Error error = _transaction->getFormatter()->endBlock(_sendBuffer);

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot format request chunk end block", _socket->name());
    return error;
  }

  return ESB_SUCCESS;
}

const char *HttpClientSocket::name() const { return _socket->name(); }

void HttpClientSocket::stateTransition(int state) {
  assert(state & STATE_MASK);
#ifndef NDEBUG
  int currentState = _state & STATE_MASK;
#endif
  _state &= ~STATE_MASK;

  switch (state & STATE_MASK) {
    case INACTIVE:
      assert(!(currentState & INACTIVE));
      _state |= INACTIVE;
      ESB_LOG_DEBUG("[%s] connection removed", _socket->name());
      break;
    case CONNECTING:
      assert(!"Cannot transition into state CONNECTING");
      break;
    case TRANSACTION_BEGIN:
      assert(currentState & (CONNECTING | INACTIVE));
      _state |= TRANSACTION_BEGIN;
      ESB_LOG_DEBUG("[%s] transaction begun", _socket->name());
      break;
    case FORMATTING_HEADERS:
      assert(currentState & TRANSACTION_BEGIN);
      _state |= FORMATTING_HEADERS;
      ESB_LOG_DEBUG("[%s] formatting headers", _socket->name());
      break;
    case FORMATTING_BODY:
      assert(currentState & FORMATTING_HEADERS);
      _state |= FORMATTING_BODY;
      ESB_LOG_DEBUG("[%s] formatting body", _socket->name());
      break;
    case FLUSHING_BODY:
      assert(currentState & FORMATTING_BODY);
      _state |= FLUSHING_BODY;
      ESB_LOG_DEBUG("[%s] flushing body", _socket->name());
      break;
    case PARSING_HEADERS:
      assert(currentState & FLUSHING_BODY);
      _state |= PARSING_HEADERS;
      ESB_LOG_DEBUG("[%s] parsing headers", _socket->name());
      break;
    case PARSING_BODY:
      assert(currentState & PARSING_HEADERS);
      _state |= PARSING_BODY;
      ESB_LOG_DEBUG("[%s] parsing body", _socket->name());
      break;
    case TRANSACTION_END:
      assert(currentState & (PARSING_HEADERS | PARSING_BODY));
      _state |= TRANSACTION_END;
      ESB_LOG_DEBUG("[%s] transaction complete", _socket->name());
      break;
    default:
      ESB_LOG_ERROR("[%s] cannot transition to unknown states: %d", _socket->name(), state & STATE_MASK);
      assert(!"Cannot transition into unknown state");
  }
}

bool HttpClientSocket::secure() const { return _socket->secure(); }

bool HttpClientSocket::permanent() { return false; }

const char *HttpClientSocket::describeState() const {
  switch (_state & STATE_MASK) {
    case INACTIVE:
      return "inactive";
    case CONNECTING:
      return "connecting";
    case TRANSACTION_BEGIN:
      return "begun";
    case FORMATTING_HEADERS:
      return "send-headers";
    case FORMATTING_BODY:
      return "send-body";
    case FLUSHING_BODY:
      return "flush-body";
    case PARSING_HEADERS:
      return "recv-headers";
    case PARSING_BODY:
      return "recv-body";
    case TRANSACTION_END:
      return "end";
    default:
      return "unknown";
  }
}

const char *HttpClientSocket::describeFlags() const {
  if (_state & RECV_PAUSED) {
    return (_state & SEND_PAUSED) ? "send-recv-paused" : "recv-paused";
  }

  if (_state & SEND_PAUSED) {
    return "send-paused";
  }

  return _state & ABORTED ? "aborted" : "active";
}

void HttpClientSocket::markDead() { _state |= DEAD; }

bool HttpClientSocket::dead() const { return _state & DEAD; }

}  // namespace ES
