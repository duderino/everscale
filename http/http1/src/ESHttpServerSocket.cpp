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

// TODO - max requests per connection option (1 disables keepalives)
// TODO - max header size option
// TODO - max body size option

static bool YieldAfterParsingRequest = false;
static bool YieldAfterFormattingHeaders = false;
static bool YieldAfterFormattingChunk = false;
static bool FlushResponseHeaders = false;
static bool CloseAfterErrorResponse = true;

HttpServerSocket::HttpServerSocket(HttpServerHandler &handler,
                                   HttpServerStack &stack,
                                   HttpServerCounters &counters,
                                   ESB::CleanupHandler &cleanupHandler)
    : _state(TRANSACTION_BEGIN),
      _bodyBytesWritten(0),
      _requestsPerConnection(0),
      _stack(stack),
      _handler(handler),
      _transaction(NULL),
      _counters(counters),
      _cleanupHandler(cleanupHandler),
      _recvBuffer(NULL),
      _sendBuffer(NULL),
      _socket() {}

HttpServerSocket::~HttpServerSocket() {
  if (_recvBuffer) {
    _stack.releaseBuffer(_recvBuffer);
    _recvBuffer = NULL;
  }
  if (_sendBuffer) {
    _stack.releaseBuffer(_sendBuffer);
    _sendBuffer = NULL;
  }
  if (_transaction) {
    _stack.destroyTransaction(_transaction);
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
  return _state & (TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY |
                   SKIPPING_TRAILER)
             ? true
             : false;
}

bool HttpServerSocket::wantWrite() {
  return _state & (FORMATTING_HEADERS | FLUSHING_HEADERS | FORMATTING_BODY |
                   FLUSHING_BODY)
             ? true
             : false;
}

bool HttpServerSocket::isIdle() {
  // TODO - implement idle timeout

  return false;
}

bool HttpServerSocket::handleAccept(ESB::SocketMultiplexer &multiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_ERROR("[%d] Cannot handle accept events", _socket.socketDescriptor());
  return true;  // keep in multiplexer
}

bool HttpServerSocket::handleConnect(ESB::SocketMultiplexer &multiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_ERROR("[%d] Cannot handle connect events",
                _socket.socketDescriptor());
  return true;  // keep in multiplexer
}

bool HttpServerSocket::handleReadable(ESB::SocketMultiplexer &multiplexer) {
  // returning true will keep the socket in the multiplexer
  // returning false will remove the socket from the multiplexer and ultimately
  // close it.

  assert(_state & (TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY |
                   SKIPPING_TRAILER));
  assert(!(HAS_BEEN_REMOVED & _state));

  if (!_recvBuffer) {
    assert(_state & TRANSACTION_BEGIN);
    _recvBuffer = _stack.acquireBuffer();
    if (!_recvBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer",
                          _socket.logAddress());
      return false;  // remove from multiplexer
    }
  }

  if (!_transaction) {
    assert(_state & TRANSACTION_BEGIN);
    _transaction = _stack.createTransaction();
    if (!_transaction) {
      _stack.releaseBuffer(_recvBuffer);
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create server trans",
                          _socket.logAddress());
      return false;  // remove from multiplexer
    }
  }

  if (_state & TRANSACTION_BEGIN) {
    // If we send a response before fully reading the request, we can't reuse
    // the socket
    _state |= CLOSE_AFTER_RESPONSE_SENT;
    _transaction->setPeerAddress(_socket.peerAddress());

    switch (_handler.beginServerTransaction(_stack, _transaction)) {
      case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
        ESB_LOG_DEBUG("[%s] handler aborted connection", _socket.logAddress());
        return false;  // remove from multiplexer - immediately close
      case HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE:
        return sendResponse(multiplexer);
      default:
        break;
    }

    _state &= ~TRANSACTION_BEGIN;
    _state |= PARSING_HEADERS;
  }

  ESB::SSize result = 0;
  ESB::Error error = ESB_SUCCESS;

  while (multiplexer.isRunning()) {
    // If there is no data in the recv buffer, read some more from the socket
    if (!_recvBuffer->isReadable()) {
      // If there is no space left in the recv buffer, make room if possible
      if (!_recvBuffer->isWritable()) {
        ESB_LOG_DEBUG("[%s] compacting input buffer", _socket.logAddress());
        if (!_recvBuffer->compact()) {
          ESB_LOG_INFO("[%s] parser jammed", _socket.logAddress());
          return sendBadRequestResponse(multiplexer);
        }
      }

      // And read from the socket
      assert(_recvBuffer->isWritable());
      result = _socket.receive(_recvBuffer);

      if (!multiplexer.isRunning()) {
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

        return handleError(error, multiplexer);
      }

      if (0 == result) {
        return handleRemoteClose(multiplexer);
      }

      ESB_LOG_DEBUG("[%s] Read %ld bytes", _socket.logAddress(), result);
    }

    if (_state & TRANSACTION_BEGIN) {
      // If we send a response before fully reading the request, we can't reuse
      // the socket
      _state |= CLOSE_AFTER_RESPONSE_SENT;
      _transaction->setPeerAddress(_socket.peerAddress());

      switch (_handler.beginServerTransaction(_stack, _transaction)) {
        case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
          ESB_LOG_DEBUG("[%s] server handler aborted connection",
                        _socket.logAddress());
          return false;  // remove from multiplexer - immediately close
        case HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE:
          return sendResponse(multiplexer);
        default:
          break;
      }

      _state &= ~TRANSACTION_BEGIN;
      _state |= PARSING_HEADERS;
    }

    if (PARSING_HEADERS & _state) {
      error = parseRequestHeaders(multiplexer);

      if (ESB_AGAIN == error) {
        continue;  // read more data and repeat parse
      }

      if (ESB_SUCCESS != error) {
        if (IsHttpError(error)) {
          return sendBadRequestResponse(multiplexer);
        } else {
          return sendInternalServerErrorResponse(multiplexer);
        }
      }

      if (!_transaction->request().hasBody()) {
        _state &= ~CLOSE_AFTER_RESPONSE_SENT;
      }

      // TODO - check Expect header and maybe send a 100 Continue

      switch (_handler.receiveRequestHeaders(_stack, _transaction)) {
        case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
          ESB_LOG_DEBUG(
              "[%s] Server request header handler aborting connection",
              _socket.logAddress());
          return false;  // remove from multiplexer
        case HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE:
          return sendResponse(multiplexer);
        default:
          break;
      }

      if (!_transaction->request().hasBody()) {
        unsigned char byte = 0;

        switch (_handler.receiveRequestBody(_stack, _transaction, &byte, 0)) {
          case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
            ESB_LOG_DEBUG("[%s] server body handler aborted connection",
                          _socket.logAddress());
            return false;  // remove from multiplexer
          default:
            return sendResponse(multiplexer);
        }
      }

      _state &= ~PARSING_HEADERS;
      _state |= PARSING_BODY;
    }

    if (PARSING_BODY & _state) {
      assert(_transaction->request().hasBody());
      error = parseRequestBody(multiplexer);

      if (ESB_AGAIN == error) {
        continue;  // read more data and repeat parse
      }

      if (ESB_SHUTDOWN == error) {
        continue;
      }

      if (ESB_SUCCESS != error) {
        if (IsHttpError(error)) {
          return sendBadRequestResponse(multiplexer);
        } else {
          return sendInternalServerErrorResponse(multiplexer);
        }
      }

      if (_state & TRANSACTION_END) {
        // server handler decided to abort
        return false;  // remove from multiplexer
      }

      if (_state & FORMATTING_HEADERS) {
        // server handler decided to send response before finishing body
        assert(_state & CLOSE_AFTER_RESPONSE_SENT);
        return sendResponse(multiplexer);
      }

      assert(_state & SKIPPING_TRAILER);
    }

    assert(SKIPPING_TRAILER & _state);
    assert(_transaction->request().hasBody());
    error = skipTrailer(multiplexer);

    if (ESB_AGAIN == error) {
      continue;  // read more data and repeat parse
    }

    if (ESB_SUCCESS != error) {
      return false;  // remove from multiplexer
    }

    _state &= ~CLOSE_AFTER_RESPONSE_SENT;

    // server handler should have populated the response object in
    // parseRequestBody()

    return sendResponse(multiplexer);
  }

  ESB_LOG_DEBUG("[%s] multiplexer shutdown with socket in parse state",
                _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleWritable(ESB::SocketMultiplexer &multiplexer) {
  assert(_state & (FORMATTING_HEADERS | FORMATTING_BODY));
  assert(!(HAS_BEEN_REMOVED & _state));
  assert(_transaction);

  if (!_sendBuffer) {
    _sendBuffer = _stack.acquireBuffer();
    if (!_sendBuffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create buffer",
                          _socket.logAddress());
      return false;  // remove from multiplexer
    }
  }

  ESB::Error error = ESB_SUCCESS;

  while (multiplexer.isRunning()) {
    if (FORMATTING_HEADERS & _state) {
      error = formatResponseHeaders(multiplexer);

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

      if (YieldAfterFormattingHeaders && !FlushResponseHeaders) {
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

      if (_transaction->response().hasBody()) {
        _state |= FORMATTING_BODY;
        if (YieldAfterFormattingHeaders) {
          return true;  // keep in multiplexer; but move on to the next
                        // connection
        }
      } else {
        _state |= TRANSACTION_END;
      }
    }

    if (FORMATTING_BODY & _state) {
      error = formatResponseBody(multiplexer);

      if (ESB_SHUTDOWN == error) {
        continue;
      }

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

    if (FLUSHING_BODY & _state) {
      error = flushBuffer(multiplexer);

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
    _handler.endServerTransaction(
        _stack, _transaction, HttpServerHandler::ES_HTTP_SERVER_HANDLER_END);

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
          return handleReadable(multiplexer);
        }

        _stack.releaseBuffer(_recvBuffer);
        _recvBuffer = NULL;
      }

      if (_sendBuffer) {
        _stack.releaseBuffer(_sendBuffer);
        _sendBuffer = NULL;
      }

      _stack.destroyTransaction(_transaction);
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

bool HttpServerSocket::handleError(ESB::Error error,
                                   ESB::SocketMultiplexer &multiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO_ERRNO(error, "[%s] socket error", _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleRemoteClose(ESB::SocketMultiplexer &multiplexer) {
  // TODO - this may just mean the client closed its half of the socket but is
  // still expecting a response.
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] clien peer closed socket", _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleIdle(ESB::SocketMultiplexer &multiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_INFO("[%s] client is idle", _socket.logAddress());
  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleRemove(ESB::SocketMultiplexer &multiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_DEBUG("[%s] closing socket", _socket.logAddress());
  _socket.close();

  if (_sendBuffer) {
    _stack.releaseBuffer(_sendBuffer);
    _sendBuffer = NULL;
  }
  if (_recvBuffer) {
    _stack.releaseBuffer(_recvBuffer);
    _recvBuffer = NULL;
  }

  if (_state & PARSING_HEADERS) {
    assert(_transaction);
    _handler.endServerTransaction(
        _stack, _transaction,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS);
  } else if (_state & PARSING_BODY) {
    assert(_transaction);
    _handler.endServerTransaction(
        _stack, _transaction,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY);
  } else if (_state & (FORMATTING_HEADERS | FLUSHING_HEADERS)) {
    assert(_transaction);
    _handler.endServerTransaction(
        _stack, _transaction,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS);
  } else if (_state & (FORMATTING_BODY | FLUSHING_BODY)) {
    assert(_transaction);
    _handler.endServerTransaction(
        _stack, _transaction,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY);
  }

  if (_transaction) {
    _stack.destroyTransaction(_transaction);
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

const char *HttpServerSocket::name() const { return "HttpServerSocket"; }

ESB::Error HttpServerSocket::parseRequestHeaders(
    ESB::SocketMultiplexer &multiplexer) {
  ESB::Error error = _transaction->getParser()->parseHeaders(
      _recvBuffer, _transaction->request());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] need more header data from stream",
                  _socket.logAddress());
    if (!_recvBuffer->compact()) {
      ESB_LOG_INFO("[%s] cannot parse headers: parser jammed",
                   _socket.logAddress());
      return ESB_OVERFLOW;  // remove from multiplexer
    }
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] cannot parse headers",
                       _socket.logAddress());
    return error;  // remove from multiplexer
  }

  // parse complete

  if (ESB_DEBUG_LOGGABLE) {
    ESB_LOG_DEBUG("[%s] headers parsed", _socket.logAddress());
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

ESB::Error HttpServerSocket::parseRequestBody(
    ESB::SocketMultiplexer &multiplexer) {
  int startingPosition = 0;
  int chunkSize = 0;

  while (multiplexer.isRunning()) {
    ESB::Error error = _transaction->getParser()->parseBody(
        _recvBuffer, &startingPosition, &chunkSize);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("[%s] need more body data from stream",
                    _socket.logAddress());
      if (!_recvBuffer->isWritable()) {
        ESB_LOG_DEBUG("[%s] compacting input buffer", _socket.logAddress());
        if (!_recvBuffer->compact()) {
          ESB_LOG_INFO("[%s] cannot parse body: parser jammed",
                       _socket.logAddress());
          return ESB_OVERFLOW;  // remove from multiplexer
        }
      }
      return ESB_AGAIN;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] error parsing body: %d",
                         _socket.logAddress(), error);
      return error;  // remove from multiplexer
    }

    if (0 == chunkSize) {
      ESB_LOG_DEBUG("[%s] parsed body", _socket.logAddress());

      unsigned char byte = 0;
      HttpServerHandler::Result result =
          _handler.receiveRequestBody(_stack, _transaction, &byte, 0);
      _state &= ~PARSING_BODY;

      if (HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE == result) {
        ESB_LOG_DEBUG("[%s] handler aborting connection after last body chunk",
                      _socket.logAddress());
        _state |= TRANSACTION_END;
      } else {
        _state |= SKIPPING_TRAILER;
      }

      return ESB_SUCCESS;
    }

    if (ESB_DEBUG_LOGGABLE) {
      char buffer[4096];
      memcpy(buffer, _recvBuffer->buffer() + startingPosition,
             chunkSize > (int)sizeof(buffer) ? sizeof(buffer) : chunkSize);
      buffer[chunkSize > (int)sizeof(buffer) ? sizeof(buffer) : chunkSize] = 0;
      ESB_LOG_DEBUG("[%s] read chunk: %s", _socket.logAddress(), buffer);
    }

    HttpServerHandler::Result result = _handler.receiveRequestBody(
        _stack, _transaction, _recvBuffer->buffer() + startingPosition,
        chunkSize);

    switch (result) {
      case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
        ESB_LOG_DEBUG("[%s] handler aborting connection before last body chunk",
                      _socket.logAddress());
        _state &= ~PARSING_BODY;
        _state |= TRANSACTION_END;
        return ESB_SUCCESS;
      case HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE:
        ESB_LOG_DEBUG("[%s] handler sending response before last body chunk",
                      _socket.logAddress());
        _state &= ~PARSING_BODY;
        _state |= FORMATTING_HEADERS;
        return ESB_SUCCESS;
      case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CONTINUE:
      default:
        break;
    }
  }

  return ESB_SHUTDOWN;
}

ESB::Error HttpServerSocket::skipTrailer(ESB::SocketMultiplexer &multiplexer) {
  assert(_state & SKIPPING_TRAILER);

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

  ESB_LOG_DEBUG("[%s] trailer skipped", _socket.logAddress());
  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::formatResponseHeaders(
    ESB::SocketMultiplexer &multiplexer) {
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
  if (FlushResponseHeaders) {
    _state |= FLUSHING_HEADERS;
  } else {
    _state |= FORMATTING_BODY;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::formatResponseBody(
    ESB::SocketMultiplexer &multiplexer) {
  ESB::Error error;
  int availableSize = 0;
  int requestedSize = 0;

  while (multiplexer.isRunning()) {
    availableSize = 0;
    requestedSize = _handler.reserveResponseChunk(_stack, _transaction);

    if (0 > requestedSize) {
      ESB_LOG_DEBUG(
          "[%s] handler aborted connection while sending response body",
          _socket.logAddress());
      return ESB_INTR;  // remove from multiplexer
    }

    if (0 == requestedSize) {
      break;  // format last chunk
    }

    error = _transaction->getFormatter()->beginBlock(_sendBuffer, requestedSize,
                                                     &availableSize);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("[%s] partially formatted response body",
                    _socket.logAddress());
      return ESB_AGAIN;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] error formatting response body",
                         _socket.logAddress());
      return error;  // remove from multiplexer
    }

    if (requestedSize < availableSize) {
      availableSize = requestedSize;
    }

    // write the body data

    _handler.fillResponseChunk(
        _stack, _transaction,
        _sendBuffer->buffer() + _sendBuffer->writePosition(), availableSize);
    _sendBuffer->setWritePosition(_sendBuffer->writePosition() + availableSize);
    _bodyBytesWritten += availableSize;

    ESB_LOG_DEBUG("[%s] formatted chunk of size %d", _socket.logAddress(),
                  availableSize);

    // beginBlock reserves space for this operation
    error = _transaction->getFormatter()->endBlock(_sendBuffer);

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot format end block",
                         _socket.logAddress());
      return error;  // remove from multiplexer
    }

    if (YieldAfterFormattingChunk) {
      return ESB_SUCCESS;  // keep in multiplexer but yield to another socket
    }
  }

  if (!multiplexer.isRunning()) {
    return ESB_SHUTDOWN;
  }

  // format last chunk

  error = _transaction->getFormatter()->endBody(_sendBuffer);

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] insufficient space in socket buffer to end body",
                  _socket.logAddress());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error formatting last chunk",
                       _socket.logAddress());
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[%s] finished formatting body", _socket.logAddress());
  _state &= ~FORMATTING_BODY;
  _state |= FLUSHING_BODY;

  return ESB_SUCCESS;  // keep in multiplexer
}

ESB::Error HttpServerSocket::flushBuffer(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_DEBUG("[%s] flushing output buffer", _socket.logAddress());

  if (!_sendBuffer->isReadable()) {
    ESB_LOG_INFO("[%s] formatter jammed", _socket.logAddress());
    return ESB_OVERFLOW;  // remove from multiplexer
  }

  while (multiplexer.isRunning() && _sendBuffer->isReadable()) {
    ESB::SSize bytesSent = _socket.send(_sendBuffer);

    if (0 > bytesSent) {
      if (ESB_AGAIN == bytesSent) {
        ESB_LOG_DEBUG("[%s] would block flushing output buffer",
                      _socket.logAddress());
        return ESB_AGAIN;  // keep in multiplexer
      }

      ESB_LOG_INFO_ERRNO(bytesSent, "[%s] error flushing output buffer",
                         _socket.logAddress());
      return bytesSent;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("[%s] flushed %ld bytes from output buffer",
                  _socket.logAddress(), bytesSent);
  }

  return _sendBuffer->isReadable() ? ESB_SHUTDOWN : ESB_SUCCESS;
}

bool HttpServerSocket::sendResponse(ESB::SocketMultiplexer &multiplexer) {
  if (0 == _transaction->response().statusCode()) {
    ESB_LOG_INFO(
        "[%s] server handler failed to build response, sending 500 Internal "
        "Server Error",
        _socket.logAddress());
    return sendInternalServerErrorResponse(multiplexer);
  }

  // TODO strip Transfer-Encoding, Content-Length, & Connection headers from the
  // response object
  // TODO add date header and any other  headers like that

  ESB::Error error = _transaction->response().addHeader(
      "Transfer-Encoding", "chunked", _transaction->allocator());

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s} cannot build response",
                       _socket.logAddress());
    return sendInternalServerErrorResponse(multiplexer);
  }

  if (110 <= _transaction->request().httpVersion() &&
      !_transaction->request().reuseConnection()) {
    error = _transaction->response().addHeader("Connection", "close",
                                               _transaction->allocator());

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "[%s] cannot build success response",
                         _socket.logAddress());
      return sendInternalServerErrorResponse(multiplexer);
    }
  }

  ESB_LOG_DEBUG("[%s] sending response: %d %s", _socket.logAddress(),
                _transaction->response().statusCode(),
                _transaction->response().reasonPhrase());
  _state &= ~(TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY);
  _state |= FORMATTING_HEADERS;
  return YieldAfterParsingRequest ? true : handleWritable(multiplexer);
}

bool HttpServerSocket::sendBadRequestResponse(
    ESB::SocketMultiplexer &multiplexer) {
  _transaction->response().setStatusCode(400);
  _transaction->response().setReasonPhrase("Bad Request");
  _transaction->response().setHasBody(false);
  return sendResponse(multiplexer);
}

bool HttpServerSocket::sendInternalServerErrorResponse(
    ESB::SocketMultiplexer &multiplexer) {
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

  ESB_LOG_DEBUG("[%s[ sending response: %d %s", _socket.logAddress(),
                _transaction->response().statusCode(),
                _transaction->response().reasonPhrase());
  return handleWritable(multiplexer);
}

}  // namespace ES
