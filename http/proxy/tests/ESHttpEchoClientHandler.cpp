#ifndef ES_HTTP_ECHO_CLIENT_HANDLER_H
#include <ESHttpEchoClientHandler.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

#ifndef ES_HTTP_ECHO_CLIENT_CONTEXT_H
#include <ESHttpEchoClientContext.h>
#endif

#ifndef ES_HTTP_ECHO_CLIENT_REQUEST_BUILDER_H
#include <ESHttpEchoClientRequestBuilder.h>
#endif

namespace ES {

HttpEchoClientHandler::HttpEchoClientHandler(
    const char *absPath, const char *method, const char *contentType,
    const unsigned char *body, int bodySize, int totalTransactions,
    HttpConnectionPool *pool, ESB::Logger *logger)
    : _absPath(absPath),
      _method(method),
      _contentType(contentType),
      _body(body),
      _bodySize(bodySize),
      _totalTransactions(totalTransactions),
      _pool(pool),
      _logger(logger ? logger : ESB::NullLogger::GetInstance()),
      _completedTransactions() {}

HttpEchoClientHandler::~HttpEchoClientHandler() {}

int HttpEchoClientHandler::reserveRequestChunk(
    HttpTransaction *transaction) {
  assert(transaction);

  HttpEchoClientContext *context =
      (HttpEchoClientContext *)transaction->getApplicationContext();

  assert(context);

  return _bodySize - context->getBytesSent();
}

void HttpEchoClientHandler::fillRequestChunk(HttpTransaction *transaction,
                                                unsigned char *chunk,
                                                unsigned int chunkSize) {
  assert(transaction);
  assert(chunk);
  assert(0 < chunkSize);

  HttpEchoClientContext *context =
      (HttpEchoClientContext *)transaction->getApplicationContext();

  assert(context);

  unsigned int totalBytesRemaining = _bodySize - context->getBytesSent();
  unsigned int bytesToSend =
      chunkSize > totalBytesRemaining ? totalBytesRemaining : chunkSize;

  memcpy(chunk, _body + context->getBytesSent(), bytesToSend);

  context->setBytesSent(context->getBytesSent() + bytesToSend);
}

HttpClientHandler::Result HttpEchoClientHandler::receiveResponseHeaders(
    HttpTransaction *transaction) {
  assert(transaction);

  HttpResponse *response = transaction->getResponse();

  assert(response);

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[handler] headers parsed");
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[handler] StatusCode: %d", response->getStatusCode());
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[handler] ReasonPhrase: %s", response->getReasonPhrase());
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[handler] Version: HTTP/%d.%d\n",
                 response->getHttpVersion() / 100,
                 response->getHttpVersion() % 100 / 10);

    for (HttpHeader *header =
             (HttpHeader *)response->getHeaders()->getFirst();
         header; header = (HttpHeader *)header->getNext()) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__, "[handler] %s: %s\n",
                   (const char *)header->getFieldName(),
                   0 == header->getFieldValue()
                       ? "null"
                       : (const char *)header->getFieldValue());
    }
  }

  return HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONTINUE;
}

HttpClientHandler::Result HttpEchoClientHandler::receiveResponseBody(
    HttpTransaction *transaction, unsigned const char *chunk,
    unsigned int chunkSize) {
  assert(transaction);
  assert(chunk);

  if (0U == chunkSize) {
    if (_logger->isLoggable(ESB::Logger::Debug)) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[handler] Response body finished");
    }

    return ES_HTTP_CLIENT_HANDLER_CONTINUE;
  }

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    char buffer[4096];
    unsigned int size =
        (sizeof(buffer) - 1) > chunkSize ? chunkSize : (sizeof(buffer) - 1);

    memcpy(buffer, chunk, size);
    buffer[size] = 0;

    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[handler] Received body chunk: %s", buffer);
  }

  return ES_HTTP_CLIENT_HANDLER_CONTINUE;
}

void HttpEchoClientHandler::end(HttpTransaction *transaction,
                                   State state) {
  if (_totalTransactions == _completedTransactions.inc()) {
    if (_logger->isLoggable(ESB::Logger::Notice)) {
      _logger->log(ESB::Logger::Notice, __FILE__, __LINE__,
                   "[handler] All transactions completed");
    }
  }

  assert(transaction);

  HttpEchoClientContext *context =
      (HttpEchoClientContext *)transaction->getApplicationContext();

  assert(context);

  ESB::Allocator *allocator = transaction->getAllocator();

  assert(allocator);

  switch (state) {
    case ES_HTTP_CLIENT_HANDLER_BEGIN:

      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                     "[handler] Transaction failed at begin state");
      }

      break;

    case ES_HTTP_CLIENT_HANDLER_RESOLVE:

      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                     "[handler] Transaction failed at resolve state");
      }

      break;

    case ES_HTTP_CLIENT_HANDLER_CONNECT:

      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                     "[handler] Transaction failed at connect state");
      }

      break;

    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS:

      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(
            ESB::Logger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at send request headers state");
      }

      break;

    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY:

      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                     "[handler] Transaction failed at send request body state");
      }

      break;

    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS:

      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(
            ESB::Logger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at receive response headers state");
      }

      break;

    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY:

      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(
            ESB::Logger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at receive response body state");
      }

      break;

    case ES_HTTP_CLIENT_HANDLER_END:

      if (_logger->isLoggable(ESB::Logger::Debug)) {
        _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                     "[handler] Transaction finished");
      }

      break;

    default:

      if (_logger->isLoggable(ESB::Logger::Critical)) {
        _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                     "[handler] Transaction failed at unknown state");
      }
  }

  if (0U == context->getRemainingIterations()) {
    context->~HttpEchoClientContext();
    allocator->deallocate(context);
    transaction->setApplicationContext(0);

    return;
  }

  HttpClientTransaction *newTransaction =
      _pool->createClientTransaction(this);

  if (0 == newTransaction) {
    if (_logger->isLoggable(ESB::Logger::Err)) {
      _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                   "[handler] Cannot create new transaction: bad alloc");
    }

    return;
  }

  context->setRemainingIterations(context->getRemainingIterations() - 1);
  context->setBytesSent(0U);

  newTransaction->setApplicationContext(context);
  transaction->setApplicationContext(0);

  char dottedIP[16];

  transaction->getPeerAddress()->getIPAddress(dottedIP, sizeof(dottedIP));

  ESB::Error error = HttpEchoClientRequestBuilder(
      dottedIP, transaction->getPeerAddress()->getPort(), _absPath, _method,
      _contentType, newTransaction);

  if (ESB_SUCCESS != error) {
    _pool->destroyClientTransaction(newTransaction);

    if (_logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[handler] cannot build request: %s");
    }

    return;
  }

  error = _pool->executeClientTransaction(newTransaction);

  if (ESB_SUCCESS != error) {
    _pool->destroyClientTransaction(newTransaction);

    if (_logger->isLoggable(ESB::Logger::Err)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                   "[handler] Cannot execute transaction: %s", buffer);
    }

    return;
  }

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[handler] Resubmitted transaction.  %u iterations remaining",
                 context->getRemainingIterations());
  }
}

}
