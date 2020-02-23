/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_ECHO_CLIENT_HANDLER_H
#include <AWSHttpEchoClientHandler.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

#ifndef AWS_HTTP_ECHO_CLIENT_CONTEXT_H
#include <AWSHttpEchoClientContext.h>
#endif

#ifndef AWS_HTTP_ECHO_CLIENT_REQUEST_BUILDER_H
#include <AWSHttpEchoClientRequestBuilder.h>
#endif

AWSHttpEchoClientHandler::AWSHttpEchoClientHandler(
    const char *absPath, const char *method, const char *contentType,
    const unsigned char *body, int bodySize, int totalTransactions,
    AWSHttpConnectionPool *pool, ESFLogger *logger)
    : _absPath(absPath),
      _method(method),
      _contentType(contentType),
      _body(body),
      _bodySize(bodySize),
      _totalTransactions(totalTransactions),
      _pool(pool),
      _logger(logger ? logger : ESFNullLogger::GetInstance()),
      _completedTransactions() {}

AWSHttpEchoClientHandler::~AWSHttpEchoClientHandler() {}

int AWSHttpEchoClientHandler::reserveRequestChunk(
    AWSHttpTransaction *transaction) {
  ESF_ASSERT(transaction);

  AWSHttpEchoClientContext *context =
      (AWSHttpEchoClientContext *)transaction->getApplicationContext();

  ESF_ASSERT(context);

  return _bodySize - context->getBytesSent();
}

void AWSHttpEchoClientHandler::fillRequestChunk(AWSHttpTransaction *transaction,
                                                unsigned char *chunk,
                                                unsigned int chunkSize) {
  ESF_ASSERT(transaction);
  ESF_ASSERT(chunk);
  ESF_ASSERT(0 < chunkSize);

  AWSHttpEchoClientContext *context =
      (AWSHttpEchoClientContext *)transaction->getApplicationContext();

  ESF_ASSERT(context);

  unsigned int totalBytesRemaining = _bodySize - context->getBytesSent();
  unsigned int bytesToSend =
      chunkSize > totalBytesRemaining ? totalBytesRemaining : chunkSize;

  memcpy(chunk, _body + context->getBytesSent(), bytesToSend);

  context->setBytesSent(context->getBytesSent() + bytesToSend);
}

AWSHttpClientHandler::Result AWSHttpEchoClientHandler::receiveResponseHeaders(
    AWSHttpTransaction *transaction) {
  ESF_ASSERT(transaction);

  AWSHttpResponse *response = transaction->getResponse();

  ESF_ASSERT(response);

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[handler] headers parsed");
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[handler] StatusCode: %d", response->getStatusCode());
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[handler] ReasonPhrase: %s", response->getReasonPhrase());
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[handler] Version: HTTP/%d.%d\n",
                 response->getHttpVersion() / 100,
                 response->getHttpVersion() % 100 / 10);

    for (AWSHttpHeader *header =
             (AWSHttpHeader *)response->getHeaders()->getFirst();
         header; header = (AWSHttpHeader *)header->getNext()) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__, "[handler] %s: %s\n",
                   (const char *)header->getFieldName(),
                   0 == header->getFieldValue()
                       ? "null"
                       : (const char *)header->getFieldValue());
    }
  }

  return AWSHttpClientHandler::AWS_HTTP_CLIENT_HANDLER_CONTINUE;
}

AWSHttpClientHandler::Result AWSHttpEchoClientHandler::receiveResponseBody(
    AWSHttpTransaction *transaction, unsigned const char *chunk,
    unsigned int chunkSize) {
  ESF_ASSERT(transaction);
  ESF_ASSERT(chunk);

  if (0U == chunkSize) {
    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[handler] Response body finished");
    }

    return AWS_HTTP_CLIENT_HANDLER_CONTINUE;
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    char buffer[4096];
    unsigned int size =
        (sizeof(buffer) - 1) > chunkSize ? chunkSize : (sizeof(buffer) - 1);

    memcpy(buffer, chunk, size);
    buffer[size] = 0;

    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[handler] Received body chunk: %s", buffer);
  }

  return AWS_HTTP_CLIENT_HANDLER_CONTINUE;
}

void AWSHttpEchoClientHandler::end(AWSHttpTransaction *transaction,
                                   State state) {
  if (_totalTransactions == _completedTransactions.inc()) {
    if (_logger->isLoggable(ESFLogger::Notice)) {
      _logger->log(ESFLogger::Notice, __FILE__, __LINE__,
                   "[handler] All transactions completed");
    }
  }

  ESF_ASSERT(transaction);

  AWSHttpEchoClientContext *context =
      (AWSHttpEchoClientContext *)transaction->getApplicationContext();

  ESF_ASSERT(context);

  ESFAllocator *allocator = transaction->getAllocator();

  ESF_ASSERT(allocator);

  switch (state) {
    case AWS_HTTP_CLIENT_HANDLER_BEGIN:

      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[handler] Transaction failed at begin state");
      }

      break;

    case AWS_HTTP_CLIENT_HANDLER_RESOLVE:

      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[handler] Transaction failed at resolve state");
      }

      break;

    case AWS_HTTP_CLIENT_HANDLER_CONNECT:

      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[handler] Transaction failed at connect state");
      }

      break;

    case AWS_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS:

      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(
            ESFLogger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at send request headers state");
      }

      break;

    case AWS_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY:

      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[handler] Transaction failed at send request body state");
      }

      break;

    case AWS_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS:

      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(
            ESFLogger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at receive response headers state");
      }

      break;

    case AWS_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY:

      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(
            ESFLogger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at receive response body state");
      }

      break;

    case AWS_HTTP_CLIENT_HANDLER_END:

      if (_logger->isLoggable(ESFLogger::Debug)) {
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[handler] Transaction finished");
      }

      break;

    default:

      if (_logger->isLoggable(ESFLogger::Critical)) {
        _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                     "[handler] Transaction failed at unknown state");
      }
  }

  if (0U == context->getRemainingIterations()) {
    context->~AWSHttpEchoClientContext();
    allocator->deallocate(context);
    transaction->setApplicationContext(0);

    return;
  }

  AWSHttpClientTransaction *newTransaction =
      _pool->createClientTransaction(this);

  if (0 == newTransaction) {
    if (_logger->isLoggable(ESFLogger::Error)) {
      _logger->log(ESFLogger::Error, __FILE__, __LINE__,
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

  ESFError error = AWSHttpEchoClientRequestBuilder(
      dottedIP, transaction->getPeerAddress()->getPort(), _absPath, _method,
      _contentType, newTransaction);

  if (ESF_SUCCESS != error) {
    _pool->destroyClientTransaction(newTransaction);

    if (_logger->isLoggable(ESFLogger::Critical)) {
      char buffer[100];

      ESFDescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                   "[handler] cannot build request: %s");
    }

    return;
  }

  error = _pool->executeClientTransaction(newTransaction);

  if (ESF_SUCCESS != error) {
    _pool->destroyClientTransaction(newTransaction);

    if (_logger->isLoggable(ESFLogger::Error)) {
      char buffer[100];

      ESFDescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                   "[handler] Cannot execute transaction: %s", buffer);
    }

    return;
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[handler] Resubmitted transaction.  %u iterations remaining",
                 context->getRemainingIterations());
  }
}
