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

#ifndef ES_HTTP_ALLOCATOR_SIZE
#define ES_HTTP_ALLOCATOR_SIZE 3072
#endif

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

HttpServerSocket::HttpServerSocket(HttpServerHandler *handler,
                                   ESB::CleanupHandler *cleanupHandler,
                                   HttpServerCounters *counters,
                                   ESB::BufferPool &bufferPool)
    : _state(TRANSACTION_BEGIN),
      _bodyBytesWritten(0),
      _requestsPerConnection(0),
      _cleanupHandler(cleanupHandler),
      _handler(handler),
      _counters(counters),
      _bufferPool(bufferPool),
      _buffer(NULL),
      _transaction(),
      _socket() {}

HttpServerSocket::~HttpServerSocket() {
  if (_buffer) {
    _bufferPool.releaseBuffer(_buffer);
    _buffer = NULL;
  }
}

ESB::Error HttpServerSocket::reset(HttpServerHandler *handler,
                                   ESB::TCPSocket::State &state) {
  if (!handler) {
    return ESB_NULL_POINTER;
  }

  _state = TRANSACTION_BEGIN;
  _bodyBytesWritten = 0;
  _handler = handler;
  _transaction.reset();
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
  ESB_LOG_ERROR("socket:%d Cannot handle accept events",
                _socket.socketDescriptor());
  return true;  // keep in multiplexer
}

bool HttpServerSocket::handleConnect(ESB::SocketMultiplexer &multiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_ERROR("socket:%d Cannot handle connect events",
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

  if (!_buffer) {
    _buffer = _bufferPool.acquireBuffer();
    if (!_buffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "Cannot create server buffer");
      return false;  // remove from multiplexer
    }
  }

  if (_state & TRANSACTION_BEGIN) {
    // If we send a response before fully reading the request, we can't reuse
    // the socket
    _state |= CLOSE_AFTER_RESPONSE_SENT;
    _transaction.setPeerAddress(_socket.peerAddress());

    switch (_handler->beginServerTransaction(multiplexer, &_transaction)) {
      case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
        ESB_LOG_DEBUG("socket:%d server handler aborted connection",
                      _socket.socketDescriptor());
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
    if (!_buffer->isWritable()) {
      ESB_LOG_DEBUG("socket:%d compacting input buffer",
                    _socket.socketDescriptor());
      if (!_buffer->compact()) {
        ESB_LOG_INFO("socket:%d parser jammed", _socket.socketDescriptor());
        return sendBadRequestResponse(multiplexer);
      }
    }

    assert(_buffer->isWritable());
    result = _socket.receive(_buffer);

    if (!multiplexer.isRunning()) {
      break;
    }

    if (0 > result) {
      error = ESB::LastError();

      if (ESB_AGAIN == error) {
        ESB_LOG_DEBUG("socket:%d not ready for read",
                      _socket.socketDescriptor());
        return true;  // keep in multiplexer
      }

      if (ESB_INTR == error) {
        ESB_LOG_DEBUG("socket:%d interrupted", _socket.socketDescriptor());
        continue;  // try _socket.receive again
      }

      return handleError(error, multiplexer);
    }

    if (0 == result) {
      return handleRemoteClose(multiplexer);
    }

    ESB_LOG_DEBUG("socket:%d Read %ld bytes", _socket.socketDescriptor(),
                  result);

    if (_state & TRANSACTION_BEGIN) {
      // If we send a response before fully reading the request, we can't reuse
      // the socket
      _state |= CLOSE_AFTER_RESPONSE_SENT;
      _transaction.setPeerAddress(_socket.peerAddress());

      switch (_handler->beginServerTransaction(multiplexer, &_transaction)) {
        case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
          ESB_LOG_DEBUG("socket:%d server handler aborted connection",
                        _socket.socketDescriptor());
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

      if (!_transaction.request().hasBody()) {
        _state &= ~CLOSE_AFTER_RESPONSE_SENT;
      }

      // TODO - check Expect header and maybe send a 100 Continue

      switch (_handler->receiveRequestHeaders(multiplexer, &_transaction)) {
        case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
          ESB_LOG_DEBUG(
              "socket:%d Server request header handler aborting connection",
              _socket.socketDescriptor());
          return false;  // remove from multiplexer
        case HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE:
          return sendResponse(multiplexer);
        default:
          break;
      }

      if (!_transaction.request().hasBody()) {
        unsigned char byte = 0;

        switch (_handler->receiveRequestBody(multiplexer, &_transaction, &byte,
                                             0)) {
          case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
            ESB_LOG_DEBUG("socket:%d server body handler aborted connection",
                          _socket.socketDescriptor());
            return false;  // remove from multiplexer
          default:
            return sendResponse(multiplexer);
        }
      }

      _state &= ~PARSING_HEADERS;
      _state |= PARSING_BODY;
    }

    if (PARSING_BODY & _state) {
      assert(_transaction.request().hasBody());
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
    assert(_transaction.request().hasBody());
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

  ESB_LOG_DEBUG("socket:%d multiplexer shutdown with socket in parse state",
                _socket.socketDescriptor());
  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleWritable(ESB::SocketMultiplexer &multiplexer) {
  assert(_state & (FORMATTING_HEADERS | FORMATTING_BODY));
  assert(!(HAS_BEEN_REMOVED & _state));

  if (!_buffer) {
    _buffer = _bufferPool.acquireBuffer();
    if (!_buffer) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "Cannot create server buffer");
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

      if (_transaction.response().hasBody()) {
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
    _handler->endServerTransaction(
        multiplexer, &_transaction,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_END);

    if (CLOSE_AFTER_RESPONSE_SENT & _state) {
      return false;  // remove from multiplexer
    }

    if (CloseAfterErrorResponse &&
        300 <= _transaction.response().statusCode()) {
      return false;  // remove from multiplexer
    }

    // TODO - close connection if max requests sent on connection

    if (_transaction.request().reuseConnection()) {
      _transaction.reset();
      _state = TRANSACTION_BEGIN;
      _bodyBytesWritten = 0;

      // TODO - when buffers are split into input and output buffers, release
      // output buffers unconditionally but only release input buffers if
      // the have no data in them belonging to the next transaction

      if (_buffer) {
        _bufferPool.releaseBuffer(_buffer);
        _buffer = NULL;
      }

      return true;  // keep in multiplexer; but always yield after a http
                    // transaction
    } else {
      return false;  // remove from multiplexer
    }
  }

  ESB_LOG_DEBUG("socket:%d multiplexer shutdown with socket in format state",
                _socket.socketDescriptor());
  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleError(ESB::Error error,
                                   ESB::SocketMultiplexer &multiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));

  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.peerAddress().presentationAddress(dottedAddress,
                                              sizeof(dottedAddress));
    ESB_LOG_INFO_ERRNO(error, "socket:%d error from client %s:%u",
                       _socket.socketDescriptor(), dottedAddress,
                       _socket.peerAddress().port());
  }

  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleRemoteClose(ESB::SocketMultiplexer &multiplexer) {
  // TODO - this may just mean the client closed its half of the socket but is
  // still expecting a response.

  assert(!(HAS_BEEN_REMOVED & _state));

  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.peerAddress().presentationAddress(dottedAddress,
                                              sizeof(dottedAddress));
    ESB_LOG_INFO("socket:%d client %s:%d closed socket",
                 _socket.socketDescriptor(), dottedAddress,
                 _socket.peerAddress().port());
  }

  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleIdle(ESB::SocketMultiplexer &multiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));

  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.peerAddress().presentationAddress(dottedAddress,
                                              sizeof(dottedAddress));
    ESB_LOG_INFO("socket:%d client %s:%d is idle", _socket.socketDescriptor(),
                 dottedAddress, _socket.peerAddress().port());
  }

  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleRemove(ESB::SocketMultiplexer &multiplexer) {
  assert(!(HAS_BEEN_REMOVED & _state));

  if (ESB_DEBUG_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.peerAddress().presentationAddress(dottedAddress,
                                              sizeof(dottedAddress));
    ESB_LOG_DEBUG("socket:%d closing socket for client %s:%d",
                  _socket.socketDescriptor(), dottedAddress,
                  _socket.peerAddress().port());
  }

  _socket.close();

  if (_buffer) {
    _bufferPool.releaseBuffer(_buffer);
    _buffer = NULL;
  }

  if (_state & PARSING_HEADERS) {
    _handler->endServerTransaction(
        multiplexer, &_transaction,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS);
  } else if (_state & PARSING_BODY) {
    _handler->endServerTransaction(
        multiplexer, &_transaction,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY);
  } else if (_state & (FORMATTING_HEADERS | FLUSHING_HEADERS)) {
    _handler->endServerTransaction(
        multiplexer, &_transaction,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS);
  } else if (_state & (FORMATTING_BODY | FLUSHING_BODY)) {
    _handler->endServerTransaction(
        multiplexer, &_transaction,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY);
  }

  _transaction.reset();
  _state = HAS_BEEN_REMOVED;
  _counters->getAverageTransactionsPerConnection()->add(_requestsPerConnection);
  _requestsPerConnection = 0;

  return true;  // call cleanup handler on us after this returns
}

SOCKET HttpServerSocket::socketDescriptor() const {
  return _socket.socketDescriptor();
}

ESB::CleanupHandler *HttpServerSocket::cleanupHandler() {
  return _cleanupHandler;
}

const char *HttpServerSocket::name() const { return "HttpServerSocket"; }

ESB::Error HttpServerSocket::parseRequestHeaders(
    ESB::SocketMultiplexer &multiplexer) {
  ESB::Error error =
      _transaction.getParser()->parseHeaders(_buffer, _transaction.request());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("socket:%d need more header data from stream",
                  _socket.socketDescriptor());
    if (!_buffer->compact()) {
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
    ESB_LOG_DEBUG("socket:%d Method: %s", _socket.socketDescriptor(),
                  _transaction.request().method());

    switch (_transaction.request().requestUri().type()) {
      case HttpRequestUri::ES_URI_ASTERISK:
        ESB_LOG_DEBUG("socket:%d Asterisk Request-URI",
                      _socket.socketDescriptor());
        break;
      case HttpRequestUri::ES_URI_HTTP:
      case HttpRequestUri::ES_URI_HTTPS:
        ESB_LOG_DEBUG("socket:%d Scheme: %s", _socket.socketDescriptor(),
                      HttpRequestUri::ES_URI_HTTP ==
                              _transaction.request().requestUri().type()
                          ? "http"
                          : "https");
        ESB_LOG_DEBUG("socket:%d Host: %s", _socket.socketDescriptor(),
                      ESB_SAFE_STR(_transaction.request().requestUri().host()));
        ESB_LOG_DEBUG("socket:%d Port: %d", _socket.socketDescriptor(),
                      _transaction.request().requestUri().port());
        ESB_LOG_DEBUG(
            "socket:%d AbsPath: %s", _socket.socketDescriptor(),
            ESB_SAFE_STR(_transaction.request().requestUri().absPath()));
        ESB_LOG_DEBUG(
            "socket:%d Query: %s", _socket.socketDescriptor(),
            ESB_SAFE_STR(_transaction.request().requestUri().query()));
        ESB_LOG_DEBUG(
            "socket:%d Fragment: %s", _socket.socketDescriptor(),
            ESB_SAFE_STR(_transaction.request().requestUri().fragment()));
        break;
      case HttpRequestUri::ES_URI_OTHER:
        ESB_LOG_DEBUG(
            "socket:%d Other: %s", _socket.socketDescriptor(),
            ESB_SAFE_STR(_transaction.request().requestUri().other()));
        break;
    }

    ESB_LOG_DEBUG("socket:%d Version: HTTP/%d.%d", _socket.socketDescriptor(),
                  _transaction.request().httpVersion() / 100,
                  _transaction.request().httpVersion() % 100 / 10);

    HttpHeader *header = (HttpHeader *)_transaction.request().headers().first();
    for (; header; header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("socket:%d %s: %s", _socket.socketDescriptor(),
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
    ESB::Error error = _transaction.getParser()->parseBody(
        _buffer, &startingPosition, &chunkSize);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("socket:%d need more body data from stream",
                    _socket.socketDescriptor());
      if (!_buffer->isWritable()) {
        ESB_LOG_DEBUG("socket:%d compacting input buffer",
                      _socket.socketDescriptor());
        if (!_buffer->compact()) {
          ESB_LOG_INFO("socket:%d cannot parse body: parser jammed",
                       _socket.socketDescriptor());
          return ESB_OVERFLOW;  // remove from multiplexer
        }
      }
      return ESB_AGAIN;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "socket:%d error parsing body: %d",
                         _socket.socketDescriptor(), error);
      return error;  // remove from multiplexer
    }

    if (0 == chunkSize) {
      ESB_LOG_DEBUG("socket:%d parsed body", _socket.socketDescriptor());

      unsigned char byte = 0;
      HttpServerHandler::Result result =
          _handler->receiveRequestBody(multiplexer, &_transaction, &byte, 0);
      _state &= ~PARSING_BODY;

      if (HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE == result) {
        ESB_LOG_DEBUG(
            "socket:%d server handler aborting connection post last body chunk",
            _socket.socketDescriptor());
        _state |= TRANSACTION_END;
      } else {
        _state |= SKIPPING_TRAILER;
      }

      return ESB_SUCCESS;
    }

    if (ESB_DEBUG_LOGGABLE) {
      char buffer[4096];
      memcpy(buffer, _buffer->buffer() + startingPosition,
             chunkSize > (int)sizeof(buffer) ? sizeof(buffer) : chunkSize);
      buffer[chunkSize > (int)sizeof(buffer) ? sizeof(buffer) : chunkSize] = 0;
      ESB_LOG_DEBUG("socket:%d read chunk: %s", _socket.socketDescriptor(),
                    buffer);
    }

    HttpServerHandler::Result result = _handler->receiveRequestBody(
        multiplexer, &_transaction, _buffer->buffer() + startingPosition,
        chunkSize);

    switch (result) {
      case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
        ESB_LOG_DEBUG(
            "socket:%d server handler aborting connection before "
            "last body chunk",
            _socket.socketDescriptor());
        _state &= ~PARSING_BODY;
        _state |= TRANSACTION_END;
        return ESB_SUCCESS;
      case HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE:
        ESB_LOG_DEBUG(
            "socket:%d server handler sending response before "
            "last body chunk",
            _socket.socketDescriptor());
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

  ESB::Error error = _transaction.getParser()->skipTrailer(_buffer);

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("socket:%d need more data from stream to skip trailer",
                  _socket.socketDescriptor());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "socket:%d error skipping trailer: %d\n",
                       _socket.socketDescriptor(), error);
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("socket:%d trailer skipped", _socket.socketDescriptor());
  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::formatResponseHeaders(
    ESB::SocketMultiplexer &multiplexer) {
  ESB::Error error = _transaction.getFormatter()->formatHeaders(
      _buffer, _transaction.response());

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
    requestedSize = _handler->reserveResponseChunk(multiplexer, &_transaction);

    if (0 > requestedSize) {
      ESB_LOG_DEBUG(
          "socket:%d server handler aborted connection while "
          "sending response body",
          _socket.socketDescriptor());
      return ESB_INTR;  // remove from multiplexer
    }

    if (0 == requestedSize) {
      break;  // format last chunk
    }

    error = _transaction.getFormatter()->beginBlock(_buffer, requestedSize,
                                                    &availableSize);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("socket:%d partially formatted response body",
                    _socket.socketDescriptor());
      return ESB_AGAIN;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "socket:%d error formatting response body: %d",
                         _socket.socketDescriptor(), error);
      return error;  // remove from multiplexer
    }

    if (requestedSize < availableSize) {
      availableSize = requestedSize;
    }

    // write the body data

    _handler->fillResponseChunk(multiplexer, &_transaction,
                                _buffer->buffer() + _buffer->writePosition(),
                                availableSize);
    _buffer->setWritePosition(_buffer->writePosition() + availableSize);
    _bodyBytesWritten += availableSize;

    ESB_LOG_DEBUG("socket:%d formatted chunk of size %d",
                  _socket.socketDescriptor(), availableSize);

    // beginBlock reserves space for this operation
    error = _transaction.getFormatter()->endBlock(_buffer);

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "socket:%d cannot format end block",
                         _socket.socketDescriptor());
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

  error = _transaction.getFormatter()->endBody(_buffer);

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("socket:%d insufficient space in socket buffer to end body",
                  _socket.socketDescriptor());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "socket:%d error formatting last chunk",
                       _socket.socketDescriptor());
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("socket:%d finished formatting body",
                _socket.socketDescriptor());
  _state &= ~FORMATTING_BODY;
  _state |= FLUSHING_BODY;

  return ESB_SUCCESS;  // keep in multiplexer
}

ESB::Error HttpServerSocket::flushBuffer(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_DEBUG("socket:%d flushing output buffer", _socket.socketDescriptor());

  if (!_buffer->isReadable()) {
    ESB_LOG_INFO("socket:%d formatter jammed", _socket.socketDescriptor());
    return ESB_OVERFLOW;  // remove from multiplexer
  }

  while (multiplexer.isRunning() && _buffer->isReadable()) {
    ESB::SSize bytesSent = _socket.send(_buffer);

    if (0 > bytesSent) {
      if (ESB_AGAIN == bytesSent) {
        ESB_LOG_DEBUG("socket:%d would block flushing output buffer",
                      _socket.socketDescriptor());
        return ESB_AGAIN;  // keep in multiplexer
      }

      ESB_LOG_INFO_ERRNO(bytesSent, "socket:%d error flushing output buffer",
                         _socket.socketDescriptor());
      return bytesSent;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("socket:%d flushed %ld bytes from output buffer",
                  _socket.socketDescriptor(), bytesSent);
  }

  return _buffer->isReadable() ? ESB_SHUTDOWN : ESB_SUCCESS;
}

bool HttpServerSocket::sendResponse(ESB::SocketMultiplexer &multiplexer) {
  if (0 == _transaction.response().statusCode()) {
    ESB_LOG_INFO(
        "socket:%d server handler failed to build response, "
        "sending 500 Internal Server Error",
        _socket.socketDescriptor());
    return sendInternalServerErrorResponse(multiplexer);
  }

  // TODO strip Transfer-Encoding, Content-Length, & Connection headers from the
  // response object
  // TODO add date header and any other  headers like that

  ESB::Error error = _transaction.response().addHeader(
      "Transfer-Encoding", "chunked", _transaction.allocator());

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "socket:%d cannot build response",
                       _socket.socketDescriptor());
    return sendInternalServerErrorResponse(multiplexer);
  }

  if (110 <= _transaction.request().httpVersion() &&
      !_transaction.request().reuseConnection()) {
    error = _transaction.response().addHeader("Connection", "close",
                                              _transaction.allocator());

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "socket:%d cannot build success response",
                         _socket.socketDescriptor());
      return sendInternalServerErrorResponse(multiplexer);
    }
  }

  ESB_LOG_DEBUG("socket:%d sending response: %d %s", _socket.socketDescriptor(),
                _transaction.response().statusCode(),
                _transaction.response().reasonPhrase());
  _state &= ~(TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY);
  _state |= FORMATTING_HEADERS;
  return YieldAfterParsingRequest ? true : handleWritable(multiplexer);
}

bool HttpServerSocket::sendBadRequestResponse(
    ESB::SocketMultiplexer &multiplexer) {
  _transaction.response().setStatusCode(400);
  _transaction.response().setReasonPhrase("Bad Request");
  _transaction.response().setHasBody(false);
  return sendResponse(multiplexer);
}

bool HttpServerSocket::sendInternalServerErrorResponse(
    ESB::SocketMultiplexer &multiplexer) {
  // TODO reserve a static read-only internal server error response for out of
  // memory conditions.

  _transaction.response().setStatusCode(500);
  _transaction.response().setReasonPhrase("Internal Server Error");
  _transaction.response().setHasBody(false);

  ESB::Error error = _transaction.response().addHeader(
      "Content-Length", "0", _transaction.allocator());

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "socket:%d cannot create 500 response",
                       _socket.socketDescriptor());
  }

  if (110 <= _transaction.request().httpVersion() &&
      (!_transaction.request().reuseConnection() || CloseAfterErrorResponse)) {
    error = _transaction.response().addHeader("Connection", "close",
                                              _transaction.allocator());

    if (ESB_SUCCESS != error) {
      ESB_LOG_INFO_ERRNO(error, "socket:%d cannot create 500 response",
                         _socket.socketDescriptor());
    }
  }

  _state &= ~(TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY);
  _state |= FORMATTING_HEADERS;

  ESB_LOG_DEBUG("socket:%d sending response: %d %s", _socket.socketDescriptor(),
                _transaction.response().statusCode(),
                _transaction.response().reasonPhrase());
  return handleWritable(multiplexer);
}

}  // namespace ES
