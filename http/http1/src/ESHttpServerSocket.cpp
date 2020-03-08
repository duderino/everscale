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
                                   HttpServerCounters *counters)
    : _state(TRANSACTION_BEGIN),
      _bodyBytesWritten(0),
      _requestsPerConnection(0),
      _cleanupHandler(cleanupHandler),
      _handler(handler),
      _counters(counters),
      _transaction(),
      _socket() {}

HttpServerSocket::~HttpServerSocket() {}

ESB::Error HttpServerSocket::reset(HttpServerHandler *handler,
                                   ESB::TCPSocket::AcceptData *acceptData) {
  if (!acceptData || !handler) {
    return ESB_NULL_POINTER;
  }

  _state = TRANSACTION_BEGIN;
  _bodyBytesWritten = 0;
  _handler = handler;
  _transaction.reset();
  _socket.reset(acceptData);

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

bool HttpServerSocket::handleAcceptEvent(ESB::SharedInt *isRunning) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_ERROR("socket:%d Cannot handle accept events",
                 _socket.getSocketDescriptor());
  return true;  // keep in multiplexer
}

bool HttpServerSocket::handleConnectEvent(ESB::SharedInt *isRunning) {
  assert(!(HAS_BEEN_REMOVED & _state));
  ESB_LOG_ERROR("socket:%d Cannot handle connect events",
                 _socket.getSocketDescriptor());
  return true;  // keep in multiplexer
}

bool HttpServerSocket::handleReadableEvent(ESB::SharedInt *isRunning) {
  // returning true will keep the socket in the multiplexer
  // returning false will remove the socket from the multiplexer and ultimately
  // close it.

  assert(_state & (TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY |
                   SKIPPING_TRAILER));
  assert(!(HAS_BEEN_REMOVED & _state));

  if (_state & TRANSACTION_BEGIN) {
    // If we send a response before fully reading the request, we can't reuse
    // the socket
    _state |= CLOSE_AFTER_RESPONSE_SENT;
    _transaction.setPeerAddress(&_socket.getPeerAddress());

    switch (_handler->begin(&_transaction)) {
      case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
        ESB_LOG_DEBUG("socket:%d server handler aborted connection",
                       _socket.getSocketDescriptor());
        return false;  // remove from multiplexer - immediately close
      case HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE:
        return sendResponse(isRunning);
      default:
        break;
    }

    _state &= ~TRANSACTION_BEGIN;
    _state |= PARSING_HEADERS;
  }

  ESB::SSize result = 0;
  ESB::Error error = ESB_SUCCESS;

  while (isRunning->get()) {
    if (!_transaction.getIOBuffer()->isWritable()) {
      ESB_LOG_DEBUG("socket:%d compacting input buffer",_socket.getSocketDescriptor());
      if (!_transaction.getIOBuffer()->compact()) {
        ESB_LOG_INFO("socket:%d parser jammed", _socket.getSocketDescriptor());
        return sendBadRequestResponse(isRunning);
      }
    }

    assert(_transaction.getIOBuffer()->isWritable());
    result = _socket.receive(_transaction.getIOBuffer());

    if (!isRunning->get()) {
      break;
    }

    if (0 > result) {
      error = ESB::GetLastError();

      if (ESB_AGAIN == error) {
        ESB_LOG_DEBUG("socket:%d not ready for read", _socket.getSocketDescriptor());
        return true;  // keep in multiplexer
      }

      if (ESB_INTR == error) {
        ESB_LOG_DEBUG("socket:%d interrupted",_socket.getSocketDescriptor());
        continue;  // try _socket.receive again
      }

      return handleErrorEvent(error, isRunning);
    }

    if (0 == result) {
      return handleEndOfFileEvent(isRunning);
    }

    ESB_LOG_DEBUG("socket:%d Read %ld bytes", _socket.getSocketDescriptor(),
                   result);

    if (_state & TRANSACTION_BEGIN) {
      // If we send a response before fully reading the request, we can't reuse
      // the socket
      _state |= CLOSE_AFTER_RESPONSE_SENT;
      _transaction.setPeerAddress(&_socket.getPeerAddress());

      switch (_handler->begin(&_transaction)) {
        case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
          ESB_LOG_DEBUG("socket:%d server handler aborted connection",
                         _socket.getSocketDescriptor());
          return false;  // remove from multiplexer - immediately close
        case HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE:
          return sendResponse(isRunning);
        default:
          break;
      }

      _state &= ~TRANSACTION_BEGIN;
      _state |= PARSING_HEADERS;
    }

    if (PARSING_HEADERS & _state) {
      error = parseRequestHeaders(isRunning);

      if (ESB_AGAIN == error) {
        continue;  // read more data and repeat parse
      }

      if (ESB_SUCCESS != error) {
        if (HttpIsHttpError(error)) {
          return sendBadRequestResponse(isRunning);
        } else {
          return sendInternalServerErrorResponse(isRunning);
        }
      }

      if (!_transaction.getRequest()->hasBody()) {
        _state &= ~CLOSE_AFTER_RESPONSE_SENT;
      }

      // TODO - check Expect header and maybe send a 100 Continue

      switch (_handler->receiveRequestHeaders(&_transaction)) {
        case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
          ESB_LOG_DEBUG("socket:%d Server request header handler aborting connection",
                _socket.getSocketDescriptor());
          return false;  // remove from multiplexer
        case HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE:
          return sendResponse(isRunning);
        default:
          break;
      }

      if (!_transaction.getRequest()->hasBody()) {
        unsigned char byte = 0;

        switch (_handler->receiveRequestBody(&_transaction, &byte, 0)) {
          case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
            ESB_LOG_DEBUG("socket:%d server body handler aborted connection",
                  _socket.getSocketDescriptor());
            return false;  // remove from multiplexer
          default:
            return sendResponse(isRunning);
        }
      }

      _state &= ~PARSING_HEADERS;
      _state |= PARSING_BODY;
    }

    if (PARSING_BODY & _state) {
      assert(_transaction.getRequest()->hasBody());
      error = parseRequestBody(isRunning);

      if (ESB_AGAIN == error) {
        continue;  // read more data and repeat parse
      }

      if (ESB_SHUTDOWN == error) {
        continue;
      }

      if (ESB_SUCCESS != error) {
        if (HttpIsHttpError(error)) {
          return sendBadRequestResponse(isRunning);
        } else {
          return sendInternalServerErrorResponse(isRunning);
        }
      }

      if (_state & TRANSACTION_END) {
        // server handler decided to abort
        return false;  // remove from multiplexer
      }

      if (_state & FORMATTING_HEADERS) {
        // server handler decided to send response before finishing body
        assert(_state & CLOSE_AFTER_RESPONSE_SENT);
        return sendResponse(isRunning);
      }

      assert(_state & SKIPPING_TRAILER);
    }

    assert(SKIPPING_TRAILER & _state);
    assert(_transaction.getRequest()->hasBody());
    error = skipTrailer(isRunning);

    if (ESB_AGAIN == error) {
      continue;  // read more data and repeat parse
    }

    if (ESB_SUCCESS != error) {
      return false;  // remove from multiplexer
    }

    _state &= ~CLOSE_AFTER_RESPONSE_SENT;

    // server handler should have populated the response object in
    // parseRequestBody()

    return sendResponse(isRunning);
  }

  ESB_LOG_DEBUG("socket:%d multiplexer shutdown with socket in parse state",
        _socket.getSocketDescriptor());
  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleWritableEvent(ESB::SharedInt *isRunning) {
  assert(_state & (FORMATTING_HEADERS | FORMATTING_BODY));
  assert(!(HAS_BEEN_REMOVED & _state));

  ESB::Error error = ESB_SUCCESS;

  while (isRunning->get()) {
    if (FORMATTING_HEADERS & _state) {
      error = formatResponseHeaders(isRunning);

      if (ESB_AGAIN == error) {
        error = flushBuffer(isRunning);

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
      error = flushBuffer(isRunning);

      if (ESB_AGAIN == error) {
        return true;  // keep in multiplexer, wait for socket to become writable
      }

      if (ESB_SUCCESS != error) {
        return false;  // remove from multiplexer
      }

      _state &= ~FLUSHING_HEADERS;

      if (_transaction.getResponse()->hasBody()) {
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
      error = formatResponseBody(isRunning);

      if (ESB_SHUTDOWN == error) {
        continue;
      }

      if (ESB_AGAIN == error) {
        error = flushBuffer(isRunning);

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
      error = flushBuffer(isRunning);

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
    _handler->end(&_transaction, HttpServerHandler::ES_HTTP_SERVER_HANDLER_END);

    if (CLOSE_AFTER_RESPONSE_SENT & _state) {
      return false;  // remove from multiplexer
    }

    if (CloseAfterErrorResponse &&
        300 <= _transaction.getResponse()->getStatusCode()) {
      return false;  // remove from multiplexer
    }

    // TODO - close connection if max requests sent on connection

    if (_transaction.getRequest()->getReuseConnection()) {
      _transaction.reset();
      _state = TRANSACTION_BEGIN;
      _bodyBytesWritten = 0;

      return true;  // keep in multiplexer; but always yield after a http
                    // transaction
    } else {
      return false;  // remove from multiplexer
    }
  }

  ESB_LOG_DEBUG("socket:%d multiplexer shutdown with socket in format state",
        _socket.getSocketDescriptor());
  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleErrorEvent(ESB::Error error,
                                        ESB::SharedInt *isRunning) {
  assert(!(HAS_BEEN_REMOVED & _state));

  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB_LOG_ERRNO_INFO(error, "socket:%d error from client %s:%u",
                 _socket.getSocketDescriptor(), dottedAddress, _socket.getPeerAddress().getPort());
  }

  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleEndOfFileEvent(ESB::SharedInt *isRunning) {
  // TODO - this may just mean the client closed its half of the socket but is
  // still expecting a response.

  assert(!(HAS_BEEN_REMOVED & _state));

  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB_LOG_INFO("socket:%d client %s:%d closed socket",
                 _socket.getSocketDescriptor(), dottedAddress, _socket.getPeerAddress().getPort());
  }

  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleIdleEvent(ESB::SharedInt *isRunning) {
  assert(!(HAS_BEEN_REMOVED & _state));

  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB_LOG_INFO("socket:%d client %s:%d is idle",
                 _socket.getSocketDescriptor(), dottedAddress, _socket.getPeerAddress().getPort());
  }

  return false;  // remove from multiplexer
}

bool HttpServerSocket::handleRemoveEvent(ESB::SharedInt *isRunning) {
  assert(!(HAS_BEEN_REMOVED & _state));

  if (ESB_DEBUG_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB_LOG_DEBUG("socket:%d closing socket for client %s:%d",
                 _socket.getSocketDescriptor(), dottedAddress, _socket.getPeerAddress().getPort());
  }

  _socket.close();

  if (_state & PARSING_HEADERS) {
    _handler->end(&_transaction,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS);
  } else if (_state & PARSING_BODY) {
    _handler->end(&_transaction,
                  HttpServerHandler::ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY);
  } else if (_state & (FORMATTING_HEADERS | FLUSHING_HEADERS)) {
    _handler->end(&_transaction,
        HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS);
  } else if (_state & (FORMATTING_BODY | FLUSHING_BODY)) {
    _handler->end(&_transaction,
                  HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY);
  }

  _transaction.reset();
  _state = HAS_BEEN_REMOVED;
  _counters->getAverageTransactionsPerConnection()->addValue(_requestsPerConnection);
  _requestsPerConnection = 0;

  return true;  // call cleanup handler on us after this returns
}

SOCKET HttpServerSocket::getSocketDescriptor() const {
  return _socket.getSocketDescriptor();
}

ESB::CleanupHandler *HttpServerSocket::getCleanupHandler() {
  return _cleanupHandler;
}

const char *HttpServerSocket::getName() const { return "HttpServerSocket"; }

bool HttpServerSocket::run(ESB::SharedInt *isRunning) {
  return false;  // todo - log warning
}

ESB::Error HttpServerSocket::parseRequestHeaders(ESB::SharedInt *isRunning) {
  ESB::Error error = _transaction.getParser()->parseHeaders(
      _transaction.getIOBuffer(), _transaction.getRequest());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("socket:%d need more header data from stream",
                   _socket.getSocketDescriptor());
    if (!_transaction.getIOBuffer()->compact()) {
      ESB_LOG_INFO("socket:%d cannot parse headers: parser jammed", _socket.getSocketDescriptor());
      return ESB_OVERFLOW;  // remove from multiplexer
    }
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERRNO_INFO(error, "socket:%d cannot parse headers",
                   _socket.getSocketDescriptor());
    return error;  // remove from multiplexer
  }

  // parse complete

  if (ESB_DEBUG_LOGGABLE) {
  ESB_LOG_DEBUG("socket:%d headers parsed", _socket.getSocketDescriptor());
  ESB_LOG_DEBUG("socket:%d Method: %s", _socket.getSocketDescriptor(),
                 _transaction.getRequest()->getMethod());

    switch (_transaction.getRequest()->getRequestUri()->getType()) {
      case HttpRequestUri::ES_URI_ASTERISK:
        ESB_LOG_DEBUG("socket:%d Asterisk Request-URI",
                     _socket.getSocketDescriptor());
        break;
      case HttpRequestUri::ES_URI_HTTP:
      case HttpRequestUri::ES_URI_HTTPS:
        ESB_LOG_DEBUG("socket:%d Scheme: %s",_socket.getSocketDescriptor(),
            HttpRequestUri::ES_URI_HTTP ==
                    _transaction.getRequest()->getRequestUri()->getType()
                ? "http"
                : "https");
        ESB_LOG_DEBUG("socket:%d Host: %s", _socket.getSocketDescriptor(),
                     0 == _transaction.getRequest()->getRequestUri()->getHost()
                         ? "none"
                         : (const char *)_transaction.getRequest()
                               ->getRequestUri()
                               ->getHost());
        ESB_LOG_DEBUG("socket:%d Port: %d", _socket.getSocketDescriptor(),
                     _transaction.getRequest()->getRequestUri()->getPort());
        ESB_LOG_DEBUG("socket:%d AbsPath: %s", _socket.getSocketDescriptor(),
                     _transaction.getRequest()->getRequestUri()->getAbsPath());
        ESB_LOG_DEBUG("socket:%d Query: %s", _socket.getSocketDescriptor(),
                     0 == _transaction.getRequest()->getRequestUri()->getQuery()
                         ? "none"
                         : (const char *)_transaction.getRequest()
                               ->getRequestUri()
                               ->getQuery());
        ESB_LOG_DEBUG("socket:%d Fragment: %s",
            _socket.getSocketDescriptor(),
            0 == _transaction.getRequest()->getRequestUri()->getFragment()
                ? "none"
                : (const char *)_transaction.getRequest()
                      ->getRequestUri()
                      ->getFragment());

        break;
      case HttpRequestUri::ES_URI_OTHER:
        ESB_LOG_DEBUG("socket:%d Other: %s", _socket.getSocketDescriptor(),
                     _transaction.getRequest()->getRequestUri()->getOther());
        break;
    }

    ESB_LOG_DEBUG("socket:%d Version: HTTP/%d.%d\n",
                 _socket.getSocketDescriptor(),
                 _transaction.getRequest()->getHttpVersion() / 100,
                 _transaction.getRequest()->getHttpVersion() % 100 / 10);

    HttpHeader *header = (HttpHeader *)_transaction.getRequest()->getHeaders()->getFirst();
    for (; header; header = (HttpHeader *)header->getNext()) {
      ESB_LOG_DEBUG("socket:%d %s: %s\n",
          _socket.getSocketDescriptor(), (const char *)header->getFieldName(),
          0 == header->getFieldValue() ? "null"
                                       : (const char *)header->getFieldValue());
    }
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::parseRequestBody(ESB::SharedInt *isRunning) {
  int startingPosition = 0;
  int chunkSize = 0;

  while (isRunning->get()) {
    ESB::Error error = _transaction.getParser()->parseBody(
        _transaction.getIOBuffer(), &startingPosition, &chunkSize);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("socket:%d need more body data from stream",
                     _socket.getSocketDescriptor());
      if (!_transaction.getIOBuffer()->isWritable()) {
        ESB_LOG_DEBUG("socket:%d compacting input buffer",
                       _socket.getSocketDescriptor());
        if (!_transaction.getIOBuffer()->compact()) {
          ESB_LOG_INFO("socket:%d cannot parse body: parser jammed",
                         _socket.getSocketDescriptor());
          return ESB_OVERFLOW;  // remove from multiplexer
        }
      }
      return ESB_AGAIN;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERRNO_INFO(error, "socket:%d error parsing body: %d",
                     _socket.getSocketDescriptor(), error);
      return error;  // remove from multiplexer
    }

    if (0 == chunkSize) {
      ESB_LOG_DEBUG("socket:%d parsed body", _socket.getSocketDescriptor());

      unsigned char byte = 0;
      HttpServerHandler::Result result =
          _handler->receiveRequestBody(&_transaction, &byte, 0);
      _state &= ~PARSING_BODY;

      if (HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE == result) {
        ESB_LOG_DEBUG("socket:%d server handler aborting connection after "
                       "last body chunk",
                       _socket.getSocketDescriptor());
        _state |= TRANSACTION_END;
      } else {
        _state |= SKIPPING_TRAILER;
      }

      return ESB_SUCCESS;
    }

    if (ESB_DEBUG_LOGGABLE) {
      char buffer[4096];
      memcpy(buffer, _transaction.getIOBuffer()->getBuffer() + startingPosition,
             chunkSize > (int)sizeof(buffer) ? sizeof(buffer) : chunkSize);
      buffer[chunkSize > (int)sizeof(buffer) ? sizeof(buffer) : chunkSize] = 0;
      ESB_LOG_DEBUG("socket:%d read chunk: %s", _socket.getSocketDescriptor(),
                   buffer);
    }

    HttpServerHandler::Result result = _handler->receiveRequestBody(
        &_transaction,
        _transaction.getIOBuffer()->getBuffer() + startingPosition, chunkSize);

    switch (result) {
      case HttpServerHandler::ES_HTTP_SERVER_HANDLER_CLOSE:
        ESB_LOG_DEBUG("socket:%d server handler aborting connection before "
                       "last body chunk",
                       _socket.getSocketDescriptor());
        _state &= ~PARSING_BODY;
        _state |= TRANSACTION_END;
        return ESB_SUCCESS;
      case HttpServerHandler::ES_HTTP_SERVER_HANDLER_SEND_RESPONSE:
        ESB_LOG_DEBUG("socket:%d server handler sending response before "
                       "last body chunk",
                       _socket.getSocketDescriptor());
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

ESB::Error HttpServerSocket::skipTrailer(ESB::SharedInt *isRunning) {
  assert(_state & SKIPPING_TRAILER);

  ESB::Error error =
      _transaction.getParser()->skipTrailer(_transaction.getIOBuffer());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("socket:%d need more data from stream to skip trailer",
                   _socket.getSocketDescriptor());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERRNO_INFO(error, "socket:%d error skipping trailer: %d\n",
                   _socket.getSocketDescriptor(), error);
    return error;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("socket:%d trailer skipped", _socket.getSocketDescriptor());
  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::formatResponseHeaders(ESB::SharedInt *isRunning) {
  ESB::Error error = _transaction.getFormatter()->formatHeaders(
      _transaction.getIOBuffer(), _transaction.getResponse());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("socket:%d partially formatted response headers",
                   _socket.getSocketDescriptor());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERRNO_INFO(error, "socket:%d error formatting response header",
                   _socket.getSocketDescriptor());
    return error;
  }

  ESB_LOG_DEBUG("socket:%d formatted response headers", _socket.getSocketDescriptor());

  _state &= ~FORMATTING_HEADERS;
  if (FlushResponseHeaders) {
    _state |= FLUSHING_HEADERS;
  } else {
    _state |= FORMATTING_BODY;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerSocket::formatResponseBody(ESB::SharedInt *isRunning) {
  ESB::Error error;
  int availableSize = 0;
  int requestedSize = 0;

  while (isRunning->get()) {
    availableSize = 0;
    requestedSize = _handler->reserveResponseChunk(&_transaction);

    if (0 > requestedSize) {
      ESB_LOG_DEBUG("socket:%d server handler aborted connection while "
                     "sending response body", _socket.getSocketDescriptor());
      return ESB_INTR;  // remove from multiplexer
    }

    if (0 == requestedSize) {
      break;  // format last chunk
    }

    error = _transaction.getFormatter()->beginBlock(_transaction.getIOBuffer(), requestedSize, &availableSize);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("socket:%d partially formatted response body",
                     _socket.getSocketDescriptor());
      return ESB_AGAIN;  // keep in multiplexer
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERRNO_INFO(error, "socket:%d error formatting response body: %d",
                     _socket.getSocketDescriptor(), error);
      return error;  // remove from multiplexer
    }

    if (requestedSize < availableSize) {
      availableSize = requestedSize;
    }

    // write the body data

    _handler->fillResponseChunk(
        &_transaction,
        _transaction.getIOBuffer()->getBuffer() +
            _transaction.getIOBuffer()->getWritePosition(),
        availableSize);
    _transaction.getIOBuffer()->setWritePosition(
        _transaction.getIOBuffer()->getWritePosition() + availableSize);
    _bodyBytesWritten += availableSize;

    ESB_LOG_DEBUG("socket:%d formatted chunk of size %d",
                   _socket.getSocketDescriptor(), availableSize);

    // beginBlock reserves space for this operation
    error = _transaction.getFormatter()->endBlock(_transaction.getIOBuffer());

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERRNO_INFO(error, "socket:%d cannot format end block",
                     _socket.getSocketDescriptor());
      return error;  // remove from multiplexer
    }

    if (YieldAfterFormattingChunk) {
      return ESB_SUCCESS;  // keep in multiplexer but yield to another socket
    }
  }

  if (!isRunning->get()) {
    return ESB_SHUTDOWN;
  }

  // format last chunk

  error = _transaction.getFormatter()->endBody(_transaction.getIOBuffer());

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("socket:%d insufficient space in socket buffer to end body",
          _socket.getSocketDescriptor());
    return ESB_AGAIN;  // keep in multiplexer
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERRNO_INFO(error, "socket:%d error formatting last chunk",
                   _socket.getSocketDescriptor());
    return error;  // remove from multiplexer
  }

  ESB_LOG_ERROR("socket:%d finshed formatting body", _socket.getSocketDescriptor());
  _state &= ~FORMATTING_BODY;
  _state |= FLUSHING_BODY;

  return ESB_SUCCESS;  // keep in multiplexer
}

ESB::Error HttpServerSocket::flushBuffer(ESB::SharedInt *isRunning) {
  ESB_LOG_DEBUG("socket:%d flushing output buffer", _socket.getSocketDescriptor());

  if (!_transaction.getIOBuffer()->isReadable()) {
    ESB_LOG_INFO("socket:%d formatter jammed", _socket.getSocketDescriptor());
    return ESB_OVERFLOW;  // remove from multiplexer
  }

  while (isRunning->get() && _transaction.getIOBuffer()->isReadable()) {
    ESB::SSize bytesSent = _socket.send(_transaction.getIOBuffer());

    if (0 > bytesSent) {
      if (ESB_AGAIN == bytesSent) {
        ESB_LOG_DEBUG("socket:%d would block flushing output buffer",
                       _socket.getSocketDescriptor());
        return ESB_AGAIN;  // keep in multiplexer
      }

      ESB_LOG_ERRNO_INFO(bytesSent, "socket:%d error flushing output buffer",
                     _socket.getSocketDescriptor());
      return bytesSent;  // remove from multiplexer
    }

    ESB_LOG_DEBUG("socket:%d flushed %ld bytes from output buffer",
                   _socket.getSocketDescriptor(), bytesSent);
  }

  return _transaction.getIOBuffer()->isReadable() ? ESB_SHUTDOWN : ESB_SUCCESS;
}

bool HttpServerSocket::sendResponse(ESB::SharedInt *isRunning) {
  if (0 == _transaction.getResponse()->getStatusCode()) {
    ESB_LOG_INFO("socket:%d server handler failed to build response, "
                   "sending 500 Internal Server Error",
                   _socket.getSocketDescriptor());
    return sendInternalServerErrorResponse(isRunning);
  }

  // TODO strip Transfer-Encoding, Content-Length, & Connection headers from the
  // response object
  // TODO add date header and any other  headers like that

  ESB::Error error = _transaction.getResponse()->addHeader(
      (const unsigned char *)"Transfer-Encoding",
      (const unsigned char *)"chunked", _transaction.getAllocator());

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERRNO_INFO(error, "socket:%d cannot build response",
                   _socket.getSocketDescriptor());
    return sendInternalServerErrorResponse(isRunning);
  }

  if (110 <= _transaction.getRequest()->getHttpVersion() &&
      !_transaction.getRequest()->getReuseConnection()) {
    error = _transaction.getResponse()->addHeader(
        (const unsigned char *)"Connection", (const unsigned char *)"close",
        _transaction.getAllocator());

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERRNO_INFO(error, "socket:%d cannot build success response",
                     _socket.getSocketDescriptor());
      return sendInternalServerErrorResponse(isRunning);
    }
  }

  ESB_LOG_DEBUG("socket:%d sending response: %d %s",
                 _socket.getSocketDescriptor(),
                 _transaction.getResponse()->getStatusCode(),
                 _transaction.getResponse()->getReasonPhrase());
  _state &= ~(TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY);
  _state |= FORMATTING_HEADERS;
  return YieldAfterParsingRequest ? true
                                  : handleWritableEvent(isRunning);
}

bool HttpServerSocket::sendBadRequestResponse(ESB::SharedInt *isRunning) {
  _transaction.getResponse()->setStatusCode(400);
  _transaction.getResponse()->setReasonPhrase(
      (const unsigned char *)"Bad Request");
  _transaction.getResponse()->setHasBody(false);
  return sendResponse(isRunning);
}

bool HttpServerSocket::sendInternalServerErrorResponse(ESB::SharedInt *isRunning) {
  // TODO reserve a static read-only internal server error response for out of
  // memory conditions.

  _transaction.getResponse()->setStatusCode(500);
  _transaction.getResponse()->setReasonPhrase(
      (const unsigned char *)"Internal Server Error");
  _transaction.getResponse()->setHasBody(false);

  ESB::Error error = _transaction.getResponse()->addHeader(
      (const unsigned char *)"Content-Length", (const unsigned char *)"0",
      _transaction.getAllocator());

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERRNO_INFO(error, "socket:%d cannot create 500 response",
          _socket.getSocketDescriptor());
  }

  if (110 <= _transaction.getRequest()->getHttpVersion() &&
      (!_transaction.getRequest()->getReuseConnection() ||
       CloseAfterErrorResponse)) {
    error = _transaction.getResponse()->addHeader(
        (const unsigned char *)"Connection", (const unsigned char *)"close",
        _transaction.getAllocator());

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERRNO_INFO(error, "socket:%d cannot create 500 response",
            _socket.getSocketDescriptor());
    }
  }

  _state &= ~(TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY);
  _state |= FORMATTING_HEADERS;

  ESB_LOG_DEBUG("socket:%d sending response: %d %s",
                 _socket.getSocketDescriptor(),
                 _transaction.getResponse()->getStatusCode(),
                 _transaction.getResponse()->getReasonPhrase());
  return handleWritableEvent(isRunning);
}

}  // namespace ES
