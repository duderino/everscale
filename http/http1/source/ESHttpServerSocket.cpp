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
#define CLOSE_AFTER_RESPONSE_SENT (1 << 10)
#define RECV_PAUSED (1 << 11)
#define SEND_PAUSED (1 << 12)

// TODO - max requests per connection option (1 disables keepalives)
// TODO - max header size option
// TODO - max body size option

static bool CloseAfterErrorResponse = true;

HttpServerSocket::HttpServerSocket(HttpServerHandler &handler,
                                   HttpMultiplexerExtended &multiplexer,
                                   HttpServerCounters &counters,
                                   ESB::CleanupHandler &cleanupHandler)
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
  if (_state & RECV_PAUSED) {
    return false;
  }

  return (_state & (TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY |
                    SKIPPING_TRAILER)) != 0;
}

bool HttpServerSocket::wantWrite() {
  if (_state & SEND_PAUSED) {
    return false;
  }

  return (_state & (FORMATTING_HEADERS | FLUSHING_HEADERS | FORMATTING_BODY |
                    FLUSHING_BODY)) != 0;
}

bool HttpServerSocket::isIdle() {
  // TODO - implement idle timeout

  return false;
}

ESB::Error HttpServerSocket::handleAccept() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_ERROR("[%d] Cannot handle accept events", _socket.socketDescriptor());
  return ESB_SUCCESS;  // keep in multiplexer
}

bool HttpServerSocket::handleConnect() {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_ERROR("[%d] Cannot handle connect events",
                _socket.socketDescriptor());
  return true;  // keep in multiplexer
}

bool HttpServerSocket::handleReadable() {
  // returning true will keep the socket in the multiplexer
  // returning false will remove the socket from the multiplexer and ultimately
  // close it.

  assert(_state & (TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY |
                   SKIPPING_TRAILER));
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(!(RECV_PAUSED & _state));

  if (!_recvBuffer) {
    assert(_state & TRANSACTION_BEGIN);
    _recvBuffer = _multiplexer.acquireBuffer();
    if (!_recvBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer",
                          _socket.logAddress());
      return false;  // remove from multiplexer
    }
  }

  if (!_transaction) {
    assert(_state & TRANSACTION_BEGIN);
    _transaction = _multiplexer.createServerTransaction();
    if (!_transaction) {
      _multiplexer.releaseBuffer(_recvBuffer);
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create server trans",
                          _socket.logAddress());
      return false;  // remove from multiplexer
    }
  }

  ESB::Error error = ESB_SUCCESS;

  if (_state & TRANSACTION_BEGIN) {
    // If we send a response before fully reading the request, we can't reuse
    // the socket
    _state |= CLOSE_AFTER_RESPONSE_SENT;
    _transaction->setPeerAddress(_socket.peerAddress());

    switch (error = _handler.beginTransaction(_multiplexer, *this)) {
      case ESB_SUCCESS:
        break;
      case ESB_SEND_RESPONSE:
        return sendResponse();
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler aborted connection",
                            _socket.logAddress());
        return false;  // remove from multiplexer - immediately close
    }

    _state &= ~TRANSACTION_BEGIN;
    _state |= PARSING_HEADERS;
  }

  ESB::SSize result = 0;

  while (!_multiplexer.shutdown()) {
    // If there is no data in the recv buffer, read some more from the socket
    if (!_recvBuffer->isReadable()) {
      // If there is no space left in the recv buffer, make room if possible
      if (!_recvBuffer->isWritable()) {
        ESB_LOG_DEBUG("[%s] compacting input buffer", _socket.logAddress());
        if (!_recvBuffer->compact()) {
          ESB_LOG_INFO("[%s] parser jammed", _socket.logAddress());
          return sendBadRequestResponse();
        }
      }

      // And read from the socket
      assert(_recvBuffer->isWritable());
      result = _socket.receive(_recvBuffer);

      if (_multiplexer.shutdown()) {
        break;
      }

      if (0 > result) {
        error = ESB::LastError();

        if (ESB_AGAIN == error) {
          ESB_LOG_DEBUG("[%s] not ready for read", _socket.logAddress());
          return true;  // keep in multiplexer
        }

        if (ESB_INTR == error) {
          ESB_LOG_DEBUG("[%s] interrupted", _socket.logAddress());
          continue;  // try _socket.receive again
        }

        return handleError(error);
      }

      if (0 == result) {
        return handleRemoteClose();
      }

      ESB_LOG_DEBUG("[%s] Read %ld bytes", _socket.logAddress(), result);
    }

    if (_state & TRANSACTION_BEGIN) {
      // If we send a response before fully reading the request, we can't reuse
      // the socket
      _state |= CLOSE_AFTER_RESPONSE_SENT;
      _transaction->setPeerAddress(_socket.peerAddress());

      switch (error = _handler.beginTransaction(_multiplexer, *this)) {
        case ESB_SUCCESS:
          break;
        case ESB_SEND_RESPONSE:
          return sendResponse();
        default:
          ESB_LOG_DEBUG_ERRNO(error, "[%s] server handler aborted connection",
                              _socket.logAddress());
          return false;  // remove from multiplexer - immediately close
      }

      _state &= ~TRANSACTION_BEGIN;
      _state |= PARSING_HEADERS;
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
        _state &= ~CLOSE_AFTER_RESPONSE_SENT;
      }

      // TODO - check Expect header and maybe send a 100 Continue

      switch (error = _handler.receiveRequestHeaders(_multiplexer, *this)) {
        case ESB_SUCCESS:
          break;
        case ESB_SEND_RESPONSE:
          return sendResponse();
        default:
          ESB_LOG_DEBUG_ERRNO(
              error, "[%s] Server request header handler aborting connection",
              _socket.logAddress());
          return false;  // remove from multiplexer
      }

      if (!_transaction->request().hasBody()) {
        unsigned char byte = 0;
        ESB::UInt32 bytesWritten = 0;

        switch (error = _handler.consumeRequestChunk(_multiplexer, *this, &byte,
                                                     0, &bytesWritten)) {
          case ESB_SUCCESS:
          case ESB_SEND_RESPONSE:
            return sendResponse();
          default:
            ESB_LOG_DEBUG_ERRNO(error,
                                "[%s] server body handler aborted connection",
                                _socket.logAddress());
            return false;  // remove from multiplexer
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
        continue;
      }

      if (ESB_PAUSE == error) {
        _state |= RECV_PAUSED;
        return true;  // keep in multiplexer, but remove from read interest set.
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
        return false;  // remove from multiplexer
      }

      if (_state & FORMATTING_HEADERS) {
        // server handler decided to send response before finishing body
        assert(_state & CLOSE_AFTER_RESPONSE_SENT);
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
      return false;  // remove from multiplexer
    }

    _state &= ~CLOSE_AFTER_RESPONSE_SENT;

    // server handler should have populated the response object in
    // parseRequestBody()

    return sendResponse();
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown with socket in parse state",
                _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleWritable() {
  assert(_state & (FORMATTING_HEADERS | FORMATTING_BODY));
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

  while (!_multiplexer.shutdown()) {
    if (FORMATTING_HEADERS & _state) {
      error = formatResponseHeaders();

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

    if (FLUSHING_BODY & _state) {
      error = flushBuffer();

      if (ESB_AGAIN == error) {
        return true;  // keep in multiplexer
      }

      if (ESB_SUCCESS != error) {
        return false;  // remove from multiplexer
      }

      _state &= ~FLUSHING_BODY;
      _state |= TRANSACTION_END;
    }

    assert(_state & TRANSACTION_END);

    ++_requestsPerConnection;
    _handler.endTransaction(_multiplexer, *this,
                            HttpServerHandler::ES_HTTP_SERVER_HANDLER_END);

    if (CLOSE_AFTER_RESPONSE_SENT & _state) {
      return false;  // remove from multiplexer
    }

    if (CloseAfterErrorResponse &&
        300 <= _transaction->response().statusCode()) {
      return false;  // remove from multiplexer
    }

    // TODO - close connection if max requests sent on connection

    if (_transaction->request().reuseConnection()) {
      _state = TRANSACTION_BEGIN;
      _bodyBytesWritten = 0;

      if (_recvBuffer) {
        if (_recvBuffer->isReadable()) {
          // sometimes there is data for the next transaction sitting in the
          // recv buffer.  If there is, handle the next transaction right now.
          _transaction->reset();
          return handleReadable();
        }

        _multiplexer.releaseBuffer(_recvBuffer);
        _recvBuffer = NULL;
      }

      if (_sendBuffer) {
        _multiplexer.releaseBuffer(_sendBuffer);
        _sendBuffer = NULL;
      }

      _multiplexer.destroyServerTransaction(_transaction);
      _transaction = NULL;
      return true;  // keep in multiplexer; but always yield after a http
                    // transaction
    } else {
      return false;  // remove from multiplexer
    }
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown with socket in format state",
                _socket.logAddress());
  return false;  // remove from multiplexer
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
  ESB_LOG_INFO("[%s] closing socket", _socket.logAddress());
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
    _handler.endTransaction(
        _multiplexer, *this,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS);
  } else if (_state & PARSING_BODY) {
    assert(_transaction);
    _handler.endTransaction(
        _multiplexer, *this,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY);
  } else if (_state & (FORMATTING_HEADERS | FLUSHING_HEADERS)) {
    assert(_transaction);
    _handler.endTransaction(
        _multiplexer, *this,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS);
  } else if (_state & (FORMATTING_BODY | FLUSHING_BODY)) {
    assert(_transaction);
    _handler.endTransaction(
        _multiplexer, *this,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY);
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

SOCKET HttpServerSocket::socketDescriptor() const {
  return _socket.socketDescriptor();
}

ESB::CleanupHandler *HttpServerSocket::cleanupHandler() {
  return &_cleanupHandler;
}

const char *HttpServerSocket::name() const { return logAddress(); }

ESB::Error HttpServerSocket::parseRequestHeaders() {
  assert(_transaction);

  ESB::Error error = _transaction->getParser()->parseHeaders(
      _recvBuffer, _transaction->request());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] need more request header data from stream",
                  _socket.logAddress());
    if (!_recvBuffer->compact()) {
      ESB_LOG_INFO("[%s] cannot parse request headers: parser jammed",
                   _socket.logAddress());
      return ESB_OVERFLOW;  // remove from multiplexer
    }
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot parse request headers",
                       _socket.logAddress());
    return error;  // remove from multiplexer
  }

  // parse complete

  if (ESB_DEBUG_LOGGABLE) {
    ESB_LOG_DEBUG("[%s] request headers parsed", _socket.logAddress());
    ESB_LOG_DEBUG("[%s] Method: %s", _socket.logAddress(),
                  _transaction->request().method());

    switch (_transaction->request().requestUri().type()) {
      case HttpRequestUri::ES_URI_ASTERISK:
        ESB_LOG_DEBUG("[%s] Asterisk Request-URI", _socket.logAddress());
        break;
      case HttpRequestUri::ES_URI_HTTP:
      case HttpRequestUri::ES_URI_HTTPS:
        ESB_LOG_DEBUG("[%s] scheme: %s", _socket.logAddress(),
                      _transaction->request().requestUri().typeString());
        ESB_LOG_DEBUG(
            "[%s] host: %s", _socket.logAddress(),
            ESB_SAFE_STR(_transaction->request().requestUri().host()));
        ESB_LOG_DEBUG("[%s] port: %d", _socket.logAddress(),
                      _transaction->request().requestUri().port());
        ESB_LOG_DEBUG(
            "[%s] path: %s", _socket.logAddress(),
            ESB_SAFE_STR(_transaction->request().requestUri().absPath()));
        ESB_LOG_DEBUG(
            "[%s] query: %s", _socket.logAddress(),
            ESB_SAFE_STR(_transaction->request().requestUri().query()));
        ESB_LOG_DEBUG(
            "[%s] fragment: %s", _socket.logAddress(),
            ESB_SAFE_STR(_transaction->request().requestUri().fragment()));
        break;
      case HttpRequestUri::ES_URI_OTHER:
        ESB_LOG_DEBUG(
            "[%s] Other: %s", _socket.logAddress(),
            ESB_SAFE_STR(_transaction->request().requestUri().other()));
        break;
    }

    ESB_LOG_DEBUG("[%s] Version: HTTP/%d.%d", _socket.logAddress(),
                  _transaction->request().httpVersion() / 100,
                  _transaction->request().httpVersion() % 100 / 10);

    HttpHeader *header =
        (HttpHeader *)_transaction->request().headers().first();
    for (; header; header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("[%s] %s: %s", _socket.logAddress(),
                    ESB_SAFE_STR(header->fieldName()),
                    ESB_SAFE_STR(header->fieldValue()));
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::parseRequestBody() {
  assert(_transaction);

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
        ESB_LOG_DEBUG("[%s] need more request body data from stream",
                      _socket.logAddress());
        if (!_recvBuffer->isWritable()) {
          ESB_LOG_DEBUG("[%s] compacting request input buffer",
                        _socket.logAddress());
          if (!_recvBuffer->compact()) {
            ESB_LOG_INFO("[%s] cannot parse request body: parser jammed",
                         _socket.logAddress());
            return ESB_OVERFLOW;  // remove from multiplexer
          }
        }
        return ESB_AGAIN;  // keep in multiplexer
      default:
        ESB_LOG_INFO_ERRNO(error, "[%s] error parsing request body",
                           _socket.logAddress());
        return error;  // remove from multiplexer
    }

    if (0 == bytesAvailable) {
      ESB_LOG_DEBUG("[%s] parsed request body", _socket.logAddress());
      unsigned char byte = 0;
      error = _handler.consumeRequestChunk(_multiplexer, *this, &byte, 0U,
                                           &bytesConsumed);
      _state &= ~PARSING_BODY;

      switch (error) {
        case ESB_SUCCESS:
        case ESB_SEND_RESPONSE:
          _state |= SKIPPING_TRAILER;
          return ESB_SUCCESS;
        default:
          ESB_LOG_DEBUG_ERRNO(
              error,
              "[%s] handler aborting connection after last request body chunk",
              _socket.logAddress());
          _state |= TRANSACTION_END;
          return ESB_AGAIN == error ? ESB_INTR : error;
      }
    }

    ESB_LOG_DEBUG("[%s] offering request chunk of size %u",
                  _socket.logAddress(), bytesAvailable);

    error = _handler.consumeRequestChunk(_multiplexer, *this,
                                         _recvBuffer->buffer() + bufferOffset,
                                         bytesAvailable, &bytesConsumed);
    switch (error) {
      case ESB_SUCCESS:
        if (0 == bytesConsumed) {
          ESB_LOG_DEBUG(
              "[%s] pausing request body receive until handler is ready",
              _socket.logAddress());
          return ESB_PAUSE;
        }
        break;
      case ESB_AGAIN:
        ESB_LOG_DEBUG(
            "[%s] pausing request body receive until handler is ready",
            _socket.logAddress());
        return ESB_PAUSE;
      case ESB_SEND_RESPONSE:
        ESB_LOG_DEBUG(
            "[%s] handler sending response before last request body chunk",
            _socket.logAddress());
        _state &= ~PARSING_BODY;
        _state |= FORMATTING_HEADERS;
        return ESB_SUCCESS;
      default:
        ESB_LOG_DEBUG_ERRNO(
            error,
            "[%s] handler aborting connection before last request body chunk",
            _socket.logAddress());
        _state &= ~PARSING_BODY;
        _state |= TRANSACTION_END;
        return ESB_SUCCESS;
    }

    error = _transaction->getParser()->consumeBody(_recvBuffer, bytesConsumed);
    if (ESB_SUCCESS != error) {
      ESB_LOG_DEBUG_ERRNO(error, "[%s] cannot consume %u request chunk bytes",
                          _socket.logAddress(), bytesAvailable);
      return error;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("[%s] consumed %u out of %u response chunk bytes",
                  _socket.logAddress(), bytesConsumed, bytesAvailable);
  }

  return ESB_SHUTDOWN;
}

ESB::Error HttpServerSocket::skipTrailer() {
  assert(_state & SKIPPING_TRAILER);
  assert(_transaction);

  ESB::Error error = _transaction->getParser()->skipTrailer(_recvBuffer);

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] need more data from stream to skip trailer",
                  _socket.logAddress());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error skipping trailer",
                       _socket.logAddress());
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[%s] request trailer skipped", _socket.logAddress());
  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::formatResponseHeaders() {
  assert(_transaction);

  ESB::Error error = _transaction->getFormatter()->formatHeaders(
      _sendBuffer, _transaction->response());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] partially formatted response headers",
                  _socket.logAddress());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error formatting response header",
                       _socket.logAddress());
    return error;
  }

  ESB_LOG_DEBUG("[%s] formatted response headers", _socket.logAddress());

  _state &= ~FORMATTING_HEADERS;
  _state |= FORMATTING_BODY;

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::formatResponseBody() {
  assert(_transaction);
  ESB::Error error = ESB_SUCCESS;
  ESB::UInt32 maxChunkSize = 0;
  ESB::UInt32 offeredSize = 0;

  while (!_multiplexer.shutdown()) {
    maxChunkSize = 0;
    error = _handler.offerResponseChunk(_multiplexer, *this, &offeredSize);

    switch (error) {
      case ESB_AGAIN:
        ESB_LOG_DEBUG(
            "[%s] handler cannot offer response chunk, pausing socket",
            _socket.logAddress());
        return ESB_PAUSE;  // keep in multiplexer
      case ESB_SUCCESS:
        ESB_LOG_DEBUG("[%s] handler offers response chunk of %u bytes",
                      _socket.logAddress(), offeredSize);
        break;
      default:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] handler error offering response chunk",
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
        ESB_LOG_DEBUG("[%s] partially formatted response body",
                      _socket.logAddress());
        return ESB_AGAIN;  // keep in multiplexer
      case ESB_SUCCESS:
        break;
      default:
        ESB_LOG_INFO_ERRNO(error, "[%s] error formatting response body",
                           _socket.logAddress());
        return error;  // remove from multiplexer
    }

    ESB::UInt32 chunkSize = ESB_MIN(offeredSize, maxChunkSize);

    // write the body data

    error = _handler.produceResponseChunk(
        _multiplexer, *this,
        _sendBuffer->buffer() + _sendBuffer->writePosition(), chunkSize);
    _sendBuffer->setWritePosition(_sendBuffer->writePosition() + chunkSize);
    _bodyBytesWritten += chunkSize;

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot format response chunk of size %u",
                         _socket.logAddress(), chunkSize);
      return error;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("[%s] formatted response chunk of size %u",
                  _socket.logAddress(), chunkSize);

    // beginBlock reserves space for this operation
    error = _transaction->getFormatter()->endBlock(_sendBuffer);

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot format response end block",
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
    ESB_LOG_DEBUG(
        "[%s] insufficient space in socket buffer to end response body",
        _socket.logAddress());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error formatting last response chunk",
                       _socket.logAddress());
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[%s] finished formatting response body", _socket.logAddress());
  _state &= ~FORMATTING_BODY;
  _state |= FLUSHING_BODY;

  return ESB_SUCCESS;  // keep in multiplexer
}

ESB::Error HttpServerSocket::flushBuffer() {
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
        ESB_LOG_DEBUG("[%s] would block flushing response output buffer",
                      _socket.logAddress());
        return ESB_AGAIN;  // keep in multiplexer
      }

      ESB_LOG_INFO_ERRNO(bytesSent,
                         "[%s] error flushing response output buffer",
                         _socket.logAddress());
      return bytesSent;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("[%s] flushed %ld bytes from response output buffer",
                  _socket.logAddress(), bytesSent);
  }

  return _sendBuffer->isReadable() ? ESB_SHUTDOWN : ESB_SUCCESS;
}

bool HttpServerSocket::sendResponse() {
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

  ESB::Error error = _transaction->response().addHeader(
      "Transfer-Encoding", "chunked", _transaction->allocator());

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s} cannot build response",
                       _socket.logAddress());
    return sendInternalServerErrorResponse();
  }

  if (110 <= _transaction->request().httpVersion() &&
      !_transaction->request().reuseConnection()) {
    error = _transaction->response().addHeader("Connection", "close",
                                               _transaction->allocator());

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot build success response",
                         _socket.logAddress());
      return sendInternalServerErrorResponse();
    }
  }

  ESB_LOG_DEBUG("[%s] sending response: %d %s", _socket.logAddress(),
                _transaction->response().statusCode(),
                _transaction->response().reasonPhrase());
  _state &= ~(TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY);
  _state |= FORMATTING_HEADERS;
  return handleWritable();
}

bool HttpServerSocket::sendBadRequestResponse() {
  _transaction->response().setStatusCode(400);
  _transaction->response().setReasonPhrase("Bad Request");
  _transaction->response().setHasBody(false);
  return sendResponse();
}

bool HttpServerSocket::sendInternalServerErrorResponse() {
  // TODO reserve a static read-only internal server error response for out of
  // memory conditions.

  _transaction->response().setStatusCode(500);
  _transaction->response().setReasonPhrase("Internal Server Error");
  _transaction->response().setHasBody(false);

  ESB::Error error = _transaction->response().addHeader(
      "Content-Length", "0", _transaction->allocator());

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot create 500 response",
                       _socket.logAddress());
  }

  if (110 <= _transaction->request().httpVersion() &&
      (!_transaction->request().reuseConnection() || CloseAfterErrorResponse)) {
    error = _transaction->response().addHeader("Connection", "close",
                                               _transaction->allocator());

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot create 500 response",
                         _socket.logAddress());
    }
  }

  _state &= ~(TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY);
  _state |= FORMATTING_HEADERS;

  ESB_LOG_DEBUG("[%s] sending response: %d %s", _socket.logAddress(),
                _transaction->response().statusCode(),
                _transaction->response().reasonPhrase());
  return handleWritable();
}

const char *HttpServerSocket::logAddress() const {
  return _socket.logAddress();
}

ESB::Error HttpServerSocket::abort() {
  assert(_state | RECV_PAUSED);
  assert(!(HAS_BEEN_REMOVED & _state));

  if (!(_state | RECV_PAUSED) || _state | HAS_BEEN_REMOVED) {
    return ESB_INVALID_STATE;
  }

  ESB_LOG_DEBUG("[%s] local close during paused request body read",
                _socket.logAddress());
  _state &= ~RECV_PAUSED;

  _multiplexer.multiplexer().removeMultiplexedSocket(this);

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

const ESB::SocketAddress &HttpServerSocket::peerAddress() const {
  return _socket.peerAddress();
}

}  // namespace ES
