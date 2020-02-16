/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_SERVER_SOCKET_H
#include <AWSHttpServerSocket.h>
#endif

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifndef AWS_HTTP_ERROR_H
#include <AWSHttpError.h>
#endif

#ifndef AWS_HTTP_ALLOCATOR_SIZE
#define AWS_HTTP_ALLOCATOR_SIZE 3072
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

AWSHttpServerSocket::AWSHttpServerSocket(AWSHttpServerHandler *handler,
                                         ESFCleanupHandler *cleanupHandler,
                                         ESFLogger *logger,
                                         AWSHttpServerCounters *counters)
    : _state(TRANSACTION_BEGIN),
      _bodyBytesWritten(0),
      _requestsPerConnection(0),
      _logger(logger ? logger : ESFNullLogger::GetInstance()),
      _cleanupHandler(cleanupHandler),
      _handler(handler),
      _counters(counters),
      _transaction(),
      _socket() {}

AWSHttpServerSocket::~AWSHttpServerSocket() {}

ESFError AWSHttpServerSocket::reset(AWSHttpServerHandler *handler,
                                    ESFTCPSocket::AcceptData *acceptData) {
  if (!acceptData || !handler) {
    return ESF_NULL_POINTER;
  }

  _state = TRANSACTION_BEGIN;
  _bodyBytesWritten = 0;
  _handler = handler;
  _transaction.reset();
  _socket.reset(acceptData);

  return ESF_SUCCESS;
}

bool AWSHttpServerSocket::wantAccept() { return false; }

bool AWSHttpServerSocket::wantConnect() { return false; }

bool AWSHttpServerSocket::wantRead() {
  return _state & (TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY |
                   SKIPPING_TRAILER)
             ? true
             : false;
}

bool AWSHttpServerSocket::wantWrite() {
  return _state & (FORMATTING_HEADERS | FLUSHING_HEADERS | FORMATTING_BODY |
                   FLUSHING_BODY)
             ? true
             : false;
}

bool AWSHttpServerSocket::isIdle() {
  // TODO - implement idle timeout

  return false;
}

bool AWSHttpServerSocket::handleAcceptEvent(ESFFlag *isRunning,
                                            ESFLogger *logger) {
  ESF_ASSERT(!(HAS_BEEN_REMOVED & _state));

  if (_logger->isLoggable(ESFLogger::Warning)) {
    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Cannot handle accept events",
                 _socket.getSocketDescriptor());
  }

  return true;  // keep in multiplexer
}

bool AWSHttpServerSocket::handleConnectEvent(ESFFlag *isRunning,
                                             ESFLogger *logger) {
  ESF_ASSERT(!(HAS_BEEN_REMOVED & _state));

  if (_logger->isLoggable(ESFLogger::Warning)) {
    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Cannot handle connect events",
                 _socket.getSocketDescriptor());
  }

  return true;  // keep in multiplexer
}

bool AWSHttpServerSocket::handleReadableEvent(ESFFlag *isRunning,
                                              ESFLogger *logger) {
  // returning true will keep the socket in the multiplexer
  // returning false will remove the socket from the multiplexer and ultimately
  // close it.

  ESF_ASSERT(_state & (TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY |
                       SKIPPING_TRAILER));
  ESF_ASSERT(!(HAS_BEEN_REMOVED & _state));

  if (_state & TRANSACTION_BEGIN) {
    // If we send a response before fully reading the request, we can't reuse
    // the socket
    _state |= CLOSE_AFTER_RESPONSE_SENT;
    _transaction.setPeerAddress(&_socket.getPeerAddress());

    switch (_handler->begin(&_transaction)) {
      case AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_CLOSE:

        if (_logger->isLoggable(ESFLogger::Debug)) {
          _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                       "[socket:%d] Server begin handler aborting connection",
                       _socket.getSocketDescriptor());
        }

        return false;  // remove from multiplexer - immediately close

      case AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_SEND_RESPONSE:

        return sendResponse(isRunning, logger);

      default:

        break;
    }

    _state &= ~TRANSACTION_BEGIN;
    _state |= PARSING_HEADERS;
  }

  ESFSSize result = 0;
  ESFError error = ESF_SUCCESS;

  while (isRunning->get()) {
    if (false == _transaction.getIOBuffer()->isWritable()) {
      if (_logger->isLoggable(ESFLogger::Debug)) {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] compacting input buffer for",
                     _socket.getSocketDescriptor());
      }

      if (false == _transaction.getIOBuffer()->compact()) {
        if (_logger->isLoggable(ESFLogger::Warning)) {
          _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                       "[socket:%d] parser jammed",
                       _socket.getSocketDescriptor());
        }

        return sendBadRequestResponse(isRunning, logger);
      }
    }

    ESF_ASSERT(_transaction.getIOBuffer()->isWritable());

    result = _socket.receive(_transaction.getIOBuffer());

    if (false == isRunning->get()) {
      break;
    }

    if (0 > result) {
      error = ESFGetLastError();

      if (ESF_AGAIN == error) {
        if (_logger->isLoggable(ESFLogger::Debug)) {
          _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                       "[socket:%d] not ready for read",
                       _socket.getSocketDescriptor());
        }

        return true;  // keep in multiplexer
      }

      if (ESF_INTR == error) {
        if (_logger->isLoggable(ESFLogger::Debug)) {
          _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                       "[socket:%d] interrupted",
                       _socket.getSocketDescriptor());
        }

        continue;  // try _socket.receive again
      }

      return handleErrorEvent(error, isRunning, logger);
    }

    if (0 == result) {
      return handleEndOfFileEvent(isRunning, logger);
    }

    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[socket:%d] Read %d bytes", _socket.getSocketDescriptor(),
                   result);
    }

    if (_state & TRANSACTION_BEGIN) {
      // If we send a response before fully reading the request, we can't reuse
      // the socket
      _state |= CLOSE_AFTER_RESPONSE_SENT;
      _transaction.setPeerAddress(&_socket.getPeerAddress());

      switch (_handler->begin(&_transaction)) {
        case AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_CLOSE:

          if (_logger->isLoggable(ESFLogger::Debug)) {
            _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                         "[socket:%d] Server begin handler aborting connection",
                         _socket.getSocketDescriptor());
          }

          return false;  // remove from multiplexer - immediately close

        case AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_SEND_RESPONSE:

          return sendResponse(isRunning, logger);

        default:

          break;
      }

      _state &= ~TRANSACTION_BEGIN;
      _state |= PARSING_HEADERS;
    }

    if (PARSING_HEADERS & _state) {
      error = parseRequestHeaders(isRunning, logger);

      if (ESF_AGAIN == error) {
        continue;  // read more data and repeat parse
      }

      if (ESF_SUCCESS != error) {
        if (AWSHttpIsHttpError(error)) {
          return sendBadRequestResponse(isRunning, logger);
        } else {
          return sendInternalServerErrorResponse(isRunning, logger);
        }
      }

      if (false == _transaction.getRequest()->hasBody()) {
        _state &= ~CLOSE_AFTER_RESPONSE_SENT;
      }

      // TODO - check Expect header and maybe send a 100 Continue

      switch (_handler->receiveRequestHeaders(&_transaction)) {
        case AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_CLOSE:

          if (_logger->isLoggable(ESFLogger::Debug)) {
            _logger->log(
                ESFLogger::Debug, __FILE__, __LINE__,
                "[socket:%d] Server request header handler aborting connection",
                _socket.getSocketDescriptor());
          }

          return false;  // remove from multiplexer

        case AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_SEND_RESPONSE:

          return sendResponse(isRunning, logger);

        default:

          break;
      }

      if (false == _transaction.getRequest()->hasBody()) {
        unsigned char byte = 0;

        switch (_handler->receiveRequestBody(&_transaction, &byte, 0)) {
          case AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_CLOSE:

            if (_logger->isLoggable(ESFLogger::Debug)) {
              _logger->log(
                  ESFLogger::Debug, __FILE__, __LINE__,
                  "[socket:%d] Server body handler aborting connection",
                  _socket.getSocketDescriptor());
            }

            return false;  // remove from multiplexer

          default:

            return sendResponse(isRunning, logger);
        }
      }

      _state &= ~PARSING_HEADERS;
      _state |= PARSING_BODY;
    }

    if (PARSING_BODY & _state) {
      ESF_ASSERT(_transaction.getRequest()->hasBody());

      error = parseRequestBody(isRunning, logger);

      if (ESF_AGAIN == error) {
        continue;  // read more data and repeat parse
      }

      if (ESF_SHUTDOWN == error) {
        continue;
      }

      if (ESF_SUCCESS != error) {
        if (AWSHttpIsHttpError(error)) {
          return sendBadRequestResponse(isRunning, logger);
        } else {
          return sendInternalServerErrorResponse(isRunning, logger);
        }
      }

      if (_state & TRANSACTION_END) {
        // server handler decided to abort

        return false;  // remove from multiplexer
      }

      if (_state & FORMATTING_HEADERS) {
        // server handler decided to send response before finishing body

        ESF_ASSERT(_state & CLOSE_AFTER_RESPONSE_SENT);

        return sendResponse(isRunning, logger);
      }

      ESF_ASSERT(_state & SKIPPING_TRAILER);
    }

    ESF_ASSERT(SKIPPING_TRAILER & _state);
    ESF_ASSERT(_transaction.getRequest()->hasBody());

    error = skipTrailer(isRunning, logger);

    if (ESF_AGAIN == error) {
      continue;  // read more data and repeat parse
    }

    if (ESF_SUCCESS != error) {
      return false;  // remove from multiplexer
    }

    _state &= ~CLOSE_AFTER_RESPONSE_SENT;

    // server handler should have populated the response object in
    // parseRequestBody()

    return sendResponse(isRunning, logger);
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(
        ESFLogger::Debug, __FILE__, __LINE__,
        "[socket:%d] multiplexer shutdown, killing socket in parse state",
        _socket.getSocketDescriptor());
  }

  return false;  // remove from multiplexer
}

bool AWSHttpServerSocket::handleWritableEvent(ESFFlag *isRunning,
                                              ESFLogger *logger) {
  ESF_ASSERT(_state & (FORMATTING_HEADERS | FORMATTING_BODY));
  ESF_ASSERT(!(HAS_BEEN_REMOVED & _state));

  ESFError error = ESF_SUCCESS;

  while (isRunning->get()) {
    if (FORMATTING_HEADERS & _state) {
      error = formatResponseHeaders(isRunning, logger);

      if (ESF_AGAIN == error) {
        error = flushBuffer(isRunning, logger);

        if (ESF_AGAIN == error) {
          return true;  // keep in multiplexer, wait for socket to become
                        // writable
        }

        if (ESF_SUCCESS != error) {
          return false;  // remove from multiplexer
        }

        continue;
      }

      if (ESF_SUCCESS != error) {
        return false;  // remove from multiplexer;
      }

      if (YieldAfterFormattingHeaders && false == FlushResponseHeaders) {
        return true;  // keep in multiplexer, but yield
      }
    }

    if (FLUSHING_HEADERS & _state) {
      error = flushBuffer(isRunning, logger);

      if (ESF_AGAIN == error) {
        return true;  // keep in multiplexer, wait for socket to become writable
      }

      if (ESF_SUCCESS != error) {
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
      error = formatResponseBody(isRunning, logger);

      if (ESF_SHUTDOWN == error) {
        continue;
      }

      if (ESF_AGAIN == error) {
        error = flushBuffer(isRunning, logger);

        if (ESF_AGAIN == error) {
          return true;  // keep in multiplexer, wait for socket to become
                        // writable
        }

        if (ESF_SUCCESS != error) {
          return false;  // remove from multiplexer
        }

        continue;
      }

      if (ESF_SUCCESS != error) {
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
      error = flushBuffer(isRunning, logger);

      if (ESF_AGAIN == error) {
        return true;  // keep in multiplexer
      }

      if (ESF_SUCCESS != error) {
        return false;  // remove from multiplexer
      }

      _state &= ~FLUSHING_BODY;
      _state |= TRANSACTION_END;
    }

    ESF_ASSERT(_state & TRANSACTION_END);

    ++_requestsPerConnection;
    _handler->end(&_transaction,
                  AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_END);

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

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(
        ESFLogger::Debug, __FILE__, __LINE__,
        "[socket:%d] multiplexer shutdown, killing socket in format state",
        _socket.getSocketDescriptor());
  }

  return false;  // remove from multiplexer
}

bool AWSHttpServerSocket::handleErrorEvent(ESFError errorCode,
                                           ESFFlag *isRunning,
                                           ESFLogger *logger) {
  ESF_ASSERT(!(HAS_BEEN_REMOVED & _state));

  if (_logger->isLoggable(ESFLogger::Warning)) {
    char buffer[100];
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESFDescribeError(errorCode, buffer, sizeof(buffer));

    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Error from client %s: %s",
                 _socket.getSocketDescriptor(), dottedAddress, buffer);
  }

  return false;  // remove from multiplexer
}

bool AWSHttpServerSocket::handleEndOfFileEvent(ESFFlag *isRunning,
                                               ESFLogger *logger) {
  // TODO - this may just mean the client closed its half of the socket but is
  // still expecting a response.

  ESF_ASSERT(!(HAS_BEEN_REMOVED & _state));

  if (_logger->isLoggable(ESFLogger::Debug)) {
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Client %s closed socket",
                 _socket.getSocketDescriptor(), dottedAddress);
  }

  return false;  // remove from multiplexer
}

bool AWSHttpServerSocket::handleIdleEvent(ESFFlag *isRunning,
                                          ESFLogger *logger) {
  ESF_ASSERT(!(HAS_BEEN_REMOVED & _state));

  if (_logger->isLoggable(ESFLogger::Debug)) {
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Client %s is idle", _socket.getSocketDescriptor(),
                 dottedAddress);
  }

  return false;  // remove from multiplexer
}

bool AWSHttpServerSocket::handleRemoveEvent(ESFFlag *isRunning,
                                            ESFLogger *logger) {
  ESF_ASSERT(!(HAS_BEEN_REMOVED & _state));

  if (_logger->isLoggable(ESFLogger::Debug)) {
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Closing socket for client %s",
                 _socket.getSocketDescriptor(), dottedAddress);
  }

  _socket.close();

  if (_state & PARSING_HEADERS) {
    _handler->end(
        &_transaction,
        AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS);
  } else if (_state & PARSING_BODY) {
    _handler->end(
        &_transaction,
        AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY);
  } else if (_state & (FORMATTING_HEADERS | FLUSHING_HEADERS)) {
    _handler->end(
        &_transaction,
        AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS);
  } else if (_state & (FORMATTING_BODY | FLUSHING_BODY)) {
    _handler->end(
        &_transaction,
        AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY);
  }

  _transaction.reset();
  _state = HAS_BEEN_REMOVED;
  _counters->getAverageTransactionsPerConnection()->addValue(
      _requestsPerConnection);
  _requestsPerConnection = 0;

  return true;  // call cleanup handler on us after this returns
}

SOCKET AWSHttpServerSocket::getSocketDescriptor() const {
  return _socket.getSocketDescriptor();
}

ESFCleanupHandler *AWSHttpServerSocket::getCleanupHandler() {
  return _cleanupHandler;
}

const char *AWSHttpServerSocket::getName() const {
  return "AWSHttpServerSocket";
}

bool AWSHttpServerSocket::run(ESFFlag *isRunning) {
  return false;  // todo - log warning
}

ESFError AWSHttpServerSocket::parseRequestHeaders(ESFFlag *isRunning,
                                                  ESFLogger *logger) {
  ESFError error = _transaction.getParser()->parseHeaders(
      _transaction.getIOBuffer(), _transaction.getRequest());

  if (ESF_AGAIN == error) {
    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[socket:%d] need more header data from stream",
                   _socket.getSocketDescriptor());
    }

    if (false == _transaction.getIOBuffer()->compact()) {
      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[socket:%d] cannot parse headers: parser jammed",
                     _socket.getSocketDescriptor());
      }

      return ESF_OVERFLOW;  // remove from multiplexer
    }

    return ESF_AGAIN;  // keep in multiplexer
  }

  if (ESF_SUCCESS != error) {
    if (_logger->isLoggable(ESFLogger::Warning)) {
      // TODO describe error, not just error code

      _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                   "[socket:%d] cannot parse headers: %d",
                   _socket.getSocketDescriptor(), error);
    }

    return error;  // remove from multiplexer
  }

  // parse complete

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[socket:%d] headers parsed", _socket.getSocketDescriptor());

    _logger->log(ESFLogger::Debug, __FILE__, __LINE__, "[socket:%d] Method: %s",
                 _socket.getSocketDescriptor(),
                 _transaction.getRequest()->getMethod());

    switch (_transaction.getRequest()->getRequestUri()->getType()) {
      case AWSHttpRequestUri::AWS_URI_ASTERISK:

        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] Asterisk Request-URI",
                     _socket.getSocketDescriptor());
        break;

      case AWSHttpRequestUri::AWS_URI_HTTP:
      case AWSHttpRequestUri::AWS_URI_HTTPS:

        _logger->log(
            ESFLogger::Debug, __FILE__, __LINE__, "[socket:%d] Scheme: %s",
            _socket.getSocketDescriptor(),
            AWSHttpRequestUri::AWS_URI_HTTP ==
                    _transaction.getRequest()->getRequestUri()->getType()
                ? "http"
                : "https");
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] Host: %s", _socket.getSocketDescriptor(),
                     0 == _transaction.getRequest()->getRequestUri()->getHost()
                         ? "none"
                         : (const char *)_transaction.getRequest()
                               ->getRequestUri()
                               ->getHost());
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] Port: %d", _socket.getSocketDescriptor(),
                     _transaction.getRequest()->getRequestUri()->getPort());
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] AbsPath: %s", _socket.getSocketDescriptor(),
                     _transaction.getRequest()->getRequestUri()->getAbsPath());
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] Query: %s", _socket.getSocketDescriptor(),
                     0 == _transaction.getRequest()->getRequestUri()->getQuery()
                         ? "none"
                         : (const char *)_transaction.getRequest()
                               ->getRequestUri()
                               ->getQuery());
        _logger->log(
            ESFLogger::Debug, __FILE__, __LINE__, "[socket:%d] Fragment: %s",
            _socket.getSocketDescriptor(),
            0 == _transaction.getRequest()->getRequestUri()->getFragment()
                ? "none"
                : (const char *)_transaction.getRequest()
                      ->getRequestUri()
                      ->getFragment());

        break;

      case AWSHttpRequestUri::AWS_URI_OTHER:

        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] Other: %s", _socket.getSocketDescriptor(),
                     _transaction.getRequest()->getRequestUri()->getOther());

        break;
    }

    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[socket:%d] Version: HTTP/%d.%d\n",
                 _socket.getSocketDescriptor(),
                 _transaction.getRequest()->getHttpVersion() / 100,
                 _transaction.getRequest()->getHttpVersion() % 100 / 10);

    for (AWSHttpHeader *header = (AWSHttpHeader *)_transaction.getRequest()
                                     ->getHeaders()
                                     ->getFirst();
         header; header = (AWSHttpHeader *)header->getNext()) {
      _logger->log(
          ESFLogger::Debug, __FILE__, __LINE__, "[socket:%d] %s: %s\n",
          _socket.getSocketDescriptor(), (const char *)header->getFieldName(),
          0 == header->getFieldValue() ? "null"
                                       : (const char *)header->getFieldValue());
    }
  }

  return ESF_SUCCESS;
}

ESFError AWSHttpServerSocket::parseRequestBody(ESFFlag *isRunning,
                                               ESFLogger *logger) {
  int startingPosition = 0;
  int chunkSize = 0;

  while (isRunning->get()) {
    ESFError error = _transaction.getParser()->parseBody(
        _transaction.getIOBuffer(), &startingPosition, &chunkSize);

    if (ESF_AGAIN == error) {
      if (_logger->isLoggable(ESFLogger::Debug)) {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] need more body data from stream",
                     _socket.getSocketDescriptor());
      }

      if (false == _transaction.getIOBuffer()->isWritable()) {
        if (_logger->isLoggable(ESFLogger::Debug)) {
          _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                       "[socket:%d] compacting input buffer",
                       _socket.getSocketDescriptor());
        }

        if (false == _transaction.getIOBuffer()->compact()) {
          if (_logger->isLoggable(ESFLogger::Warning)) {
            _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                         "[socket:%d] cannot parse body: parser jammed",
                         _socket.getSocketDescriptor());
          }

          return ESF_OVERFLOW;  // remove from multiplexer
        }
      }

      return ESF_AGAIN;  // keep in multiplexer
    }

    if (ESF_SUCCESS != error) {
      if (_logger->isLoggable(ESFLogger::Debug)) {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] error parsing body: %d",
                     _socket.getSocketDescriptor(), error);
      }

      return error;  // remove from multiplexer
    }

    if (0 == chunkSize) {
      if (_logger->isLoggable(ESFLogger::Debug)) {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] parsed body", _socket.getSocketDescriptor());
      }

      unsigned char byte = 0;

      AWSHttpServerHandler::Result result =
          _handler->receiveRequestBody(&_transaction, &byte, 0);

      _state &= ~PARSING_BODY;

      if (AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_CLOSE == result) {
        if (_logger->isLoggable(ESFLogger::Debug)) {
          _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                       "[socket:%d] server handler aborting connection after "
                       "last body chunk",
                       _socket.getSocketDescriptor());
        }

        _state |= TRANSACTION_END;
      } else {
        _state |= SKIPPING_TRAILER;
      }

      return ESF_SUCCESS;
    }

    if (_logger->isLoggable(ESFLogger::Debug)) {
      char buffer[4096];

      memcpy(buffer, _transaction.getIOBuffer()->getBuffer() + startingPosition,
             chunkSize > (int)sizeof(buffer) ? sizeof(buffer) : chunkSize);
      buffer[chunkSize > (int)sizeof(buffer) ? sizeof(buffer) : chunkSize] = 0;

      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[socket:%d] read chunk: %s", _socket.getSocketDescriptor(),
                   buffer);
    }

    AWSHttpServerHandler::Result result = _handler->receiveRequestBody(
        &_transaction,
        _transaction.getIOBuffer()->getBuffer() + startingPosition, chunkSize);

    switch (result) {
      case AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_CLOSE:

        if (_logger->isLoggable(ESFLogger::Debug)) {
          _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                       "[socket:%d] server handler aborting connection before "
                       "last body chunk",
                       _socket.getSocketDescriptor());
        }

        _state &= ~PARSING_BODY;
        _state |= TRANSACTION_END;

        return ESF_SUCCESS;

      case AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_SEND_RESPONSE:

        if (_logger->isLoggable(ESFLogger::Debug)) {
          _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                       "[socket:%d] server handler sending response before "
                       "last body chunk",
                       _socket.getSocketDescriptor());
        }

        _state &= ~PARSING_BODY;
        _state |= FORMATTING_HEADERS;

        return ESF_SUCCESS;

      case AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_CONTINUE:
      default:

        break;
    }
  }

  return ESF_SHUTDOWN;
}

ESFError AWSHttpServerSocket::skipTrailer(ESFFlag *isRunning,
                                          ESFLogger *logger) {
  ESF_ASSERT(_state & SKIPPING_TRAILER);

  ESFError error =
      _transaction.getParser()->skipTrailer(_transaction.getIOBuffer());

  if (ESF_AGAIN == error) {
    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[socket:%d] need more data from stream to skip trailer",
                   _socket.getSocketDescriptor());
    }

    return ESF_AGAIN;  // keep in multiplexer
  }

  if (ESF_SUCCESS != error) {
    if (_logger->isLoggable(ESFLogger::Warning)) {
      _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                   "[socket:%d] error skipping trailer: %d\n",
                   _socket.getSocketDescriptor(), error);
    }

    return error;  // remove from multiplexer
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[socket:%d] trailer skipped", _socket.getSocketDescriptor());
  }

  return ESF_SUCCESS;
}

ESFError AWSHttpServerSocket::formatResponseHeaders(ESFFlag *isRunning,
                                                    ESFLogger *logger) {
  ESFError error = _transaction.getFormatter()->formatHeaders(
      _transaction.getIOBuffer(), _transaction.getResponse());

  if (ESF_AGAIN == error) {
    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[socket:%d] partially formatted response headers",
                   _socket.getSocketDescriptor());
    }

    return ESF_AGAIN;  // keep in multiplexer
  }

  if (ESF_SUCCESS != error) {
    if (_logger->isLoggable(ESFLogger::Warning)) {
      _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                   "[socket:%d] error formatting response header: %d",
                   _socket.getSocketDescriptor(), error);
    }

    return error;
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[socket:%d] formatted response headers",
                 _socket.getSocketDescriptor(), error);
  }

  _state &= ~FORMATTING_HEADERS;

  if (FlushResponseHeaders) {
    _state |= FLUSHING_HEADERS;
  } else {
    _state |= FORMATTING_BODY;
  }

  return ESF_SUCCESS;
}

ESFError AWSHttpServerSocket::formatResponseBody(ESFFlag *isRunning,
                                                 ESFLogger *logger) {
  ESFError error;
  int availableSize = 0;
  int requestedSize = 0;

  while (isRunning->get()) {
    availableSize = 0;

    requestedSize = _handler->reserveResponseChunk(&_transaction);

    if (0 > requestedSize) {
      if (_logger->isLoggable(ESFLogger::Debug)) {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] server handler aborted connection will "
                     "sending response body",
                     _socket.getSocketDescriptor());
      }

      return ESF_INTR;  // remove from multiplexer
    }

    if (0 == requestedSize) {
      break;  // format last chunk
    }

    error = _transaction.getFormatter()->beginBlock(
        _transaction.getIOBuffer(), requestedSize, &availableSize);

    if (ESF_AGAIN == error) {
      if (_logger->isLoggable(ESFLogger::Debug)) {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[socket:%d] partially formatted response body",
                     _socket.getSocketDescriptor());
      }

      return ESF_AGAIN;  // keep in multiplexer
    }

    if (ESF_SUCCESS != error) {
      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[socket:%d] error formatting response body: %d",
                     _socket.getSocketDescriptor(), error);
      }

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

    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[socket:%d] formatted chunk of size %d",
                   _socket.getSocketDescriptor(), availableSize);
    }

    // beginBlock reserves space for this operation

    error = _transaction.getFormatter()->endBlock(_transaction.getIOBuffer());

    if (ESF_SUCCESS != error) {
      if (_logger->isLoggable(ESFLogger::Error)) {
        _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                     "[socket:%d] cannot format end block: %d",
                     _socket.getSocketDescriptor(), error);
      }

      return error;  // remove from multiplexer
    }

    if (YieldAfterFormattingChunk) {
      return ESF_SUCCESS;  // keep in multiplexer but yield to another
                           // connection
    }
  }

  if (false == isRunning->get()) {
    return ESF_SHUTDOWN;
  }

  // format last chunk

  error = _transaction.getFormatter()->endBody(_transaction.getIOBuffer());

  if (ESF_AGAIN == error) {
    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(
          ESFLogger::Debug, __FILE__, __LINE__,
          "[socket:%d] insufficient space in output buffer to format end body",
          _socket.getSocketDescriptor());
    }

    return ESF_AGAIN;  // keep in multiplexer
  }

  if (ESF_SUCCESS != error) {
    if (_logger->isLoggable(ESFLogger::Warning)) {
      _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                   "[socket:%d] error formatting last chunk: %d",
                   _socket.getSocketDescriptor(), error);
    }

    return error;  // remove from multiplexer
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[socket:%d] finshed formatting body",
                 _socket.getSocketDescriptor());
  }

  _state &= ~FORMATTING_BODY;
  _state |= FLUSHING_BODY;

  return ESF_SUCCESS;  // keep in multiplexer
}

ESFError AWSHttpServerSocket::flushBuffer(ESFFlag *isRunning,
                                          ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[socket:%d] flushing output buffer",
                 _socket.getSocketDescriptor());
  }

  if (false == _transaction.getIOBuffer()->isReadable()) {
    if (_logger->isLoggable(ESFLogger::Warning)) {
      _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                   "[socket:%d] formatter jammed",
                   _socket.getSocketDescriptor());
    }

    return ESF_OVERFLOW;  // remove from multiplexer
  }

  ESFSSize bytesSent;

  while (isRunning->get() && _transaction.getIOBuffer()->isReadable()) {
    bytesSent = _socket.send(_transaction.getIOBuffer());

    if (0 > bytesSent) {
      if (ESF_AGAIN == bytesSent) {
        if (_logger->isLoggable(ESFLogger::Debug)) {
          _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                       "[socket:%d] would block flushing output buffer",
                       _socket.getSocketDescriptor());
        }

        return ESF_AGAIN;  // keep in multiplexer
      }

      if (_logger->isLoggable(ESFLogger::Warning)) {
        char buffer[1024];

        ESFDescribeError(bytesSent, buffer, sizeof(buffer));

        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[socket:%d] error flushing output buffer: %s",
                     _socket.getSocketDescriptor(), buffer);
      }

      return bytesSent;  // remove from multiplexer
    }

    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[socket:%d] flushed %d bytes from output buffer",
                   _socket.getSocketDescriptor(), bytesSent);
    }
  }

  return _transaction.getIOBuffer()->isReadable() ? ESF_SHUTDOWN : ESF_SUCCESS;
}

bool AWSHttpServerSocket::sendResponse(ESFFlag *isRunning, ESFLogger *logger) {
  if (0 == _transaction.getResponse()->getStatusCode()) {
    if (_logger->isLoggable(ESFLogger::Error)) {
      _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                   "[socket:%d] server handler failed to build response, "
                   "sending 500 Internal Server Error",
                   _socket.getSocketDescriptor());
    }

    return sendInternalServerErrorResponse(isRunning, logger);
  }

  // TODO strip Transfer-Encoding, Content-Length, & Connection headers from the
  // response object
  // TODO add date header and any other stupid headers like that

  ESFError error = _transaction.getResponse()->addHeader(
      (const unsigned char *)"Transfer-Encoding",
      (const unsigned char *)"chunked", _transaction.getAllocator());

  if (ESF_SUCCESS != error) {
    if (_logger->isLoggable(ESFLogger::Error)) {
      char buffer[1024];

      ESFDescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                   "[socket:%d] cannot build response: %s",
                   _socket.getSocketDescriptor(), buffer);
    }

    return sendInternalServerErrorResponse(isRunning, logger);
  }

  if (110 <= _transaction.getRequest()->getHttpVersion() &&
      false == _transaction.getRequest()->getReuseConnection()) {
    error = _transaction.getResponse()->addHeader(
        (const unsigned char *)"Connection", (const unsigned char *)"close",
        _transaction.getAllocator());

    if (ESF_SUCCESS != error) {
      if (_logger->isLoggable(ESFLogger::Error)) {
        char buffer[1024];

        ESFDescribeError(error, buffer, sizeof(buffer));

        _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                     "[socket:%d] cannot build success response: %s",
                     _socket.getSocketDescriptor(), buffer);
      }

      return sendInternalServerErrorResponse(isRunning, logger);
    }
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[socket:%d] sending response: %d %s",
                 _socket.getSocketDescriptor(),
                 _transaction.getResponse()->getStatusCode(),
                 _transaction.getResponse()->getReasonPhrase());
  }

  _state &= ~(TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY);
  _state |= FORMATTING_HEADERS;

  return YieldAfterParsingRequest ? true
                                  : handleWritableEvent(isRunning, logger);
}

bool AWSHttpServerSocket::sendBadRequestResponse(ESFFlag *isRunning,
                                                 ESFLogger *logger) {
  _transaction.getResponse()->setStatusCode(400);
  _transaction.getResponse()->setReasonPhrase(
      (const unsigned char *)"Bad Request");
  _transaction.getResponse()->setHasBody(false);

  return sendResponse(isRunning, logger);
}

bool AWSHttpServerSocket::sendInternalServerErrorResponse(ESFFlag *isRunning,
                                                          ESFLogger *logger) {
  // TODO reserve a static read-only internal server error response for out of
  // memory conditions.

  _transaction.getResponse()->setStatusCode(500);
  _transaction.getResponse()->setReasonPhrase(
      (const unsigned char *)"Internal Server Error");
  _transaction.getResponse()->setHasBody(false);

  ESFError error = _transaction.getResponse()->addHeader(
      (const unsigned char *)"Content-Length", (const unsigned char *)"0",
      _transaction.getAllocator());

  if (ESF_SUCCESS != error) {
    if (_logger->isLoggable(ESFLogger::Error)) {
      char buffer[1024];

      ESFDescribeError(error, buffer, sizeof(buffer));

      _logger->log(
          ESFLogger::Error, __FILE__, __LINE__,
          "[socket:%d] cannot build internal server error response: %s",
          _socket.getSocketDescriptor(), buffer);
    }
  }

  if (110 <= _transaction.getRequest()->getHttpVersion() &&
      (false == _transaction.getRequest()->getReuseConnection() ||
       CloseAfterErrorResponse)) {
    ESFError error = _transaction.getResponse()->addHeader(
        (const unsigned char *)"Connection", (const unsigned char *)"close",
        _transaction.getAllocator());

    if (ESF_SUCCESS != error) {
      if (_logger->isLoggable(ESFLogger::Error)) {
        char buffer[1024];

        ESFDescribeError(error, buffer, sizeof(buffer));

        _logger->log(
            ESFLogger::Error, __FILE__, __LINE__,
            "[socket:%d] cannot build internal server error response: %s",
            _socket.getSocketDescriptor(), buffer);
      }
    }
  }

  _state &= ~(TRANSACTION_BEGIN | PARSING_HEADERS | PARSING_BODY);
  _state |= FORMATTING_HEADERS;

  if (_logger->isLoggable(ESFLogger::Warning)) {
    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                 "[socket:%d] sending response: %d %s",
                 _socket.getSocketDescriptor(),
                 _transaction.getResponse()->getStatusCode(),
                 _transaction.getResponse()->getReasonPhrase());
  }

  return handleWritableEvent(isRunning, logger);
}
