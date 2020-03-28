#ifndef ES_HTTP_ECHO_CLIENT_HANDLER_H
#include <ESHttpEchoClientHandler.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
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
    HttpConnectionPool *pool)
    : _absPath(absPath),
      _method(method),
      _contentType(contentType),
      _body(body),
      _bodySize(bodySize),
      _totalTransactions(totalTransactions),
      _pool(pool),
      _completedTransactions() {}

HttpEchoClientHandler::~HttpEchoClientHandler() {}

int HttpEchoClientHandler::reserveRequestChunk(
    ESB::SocketMultiplexer &multiplexer, HttpTransaction *transaction) {
  assert(transaction);

  HttpEchoClientContext *context =
      (HttpEchoClientContext *)transaction->context();

  assert(context);

  return _bodySize - context->getBytesSent();
}

void HttpEchoClientHandler::fillRequestChunk(
    ESB::SocketMultiplexer &multiplexer, HttpTransaction *transaction,
    unsigned char *chunk, unsigned int chunkSize) {
  assert(transaction);
  assert(chunk);
  assert(0 < chunkSize);

  HttpEchoClientContext *context =
      (HttpEchoClientContext *)transaction->context();

  assert(context);

  unsigned int totalBytesRemaining = _bodySize - context->getBytesSent();
  unsigned int bytesToSend =
      chunkSize > totalBytesRemaining ? totalBytesRemaining : chunkSize;

  memcpy(chunk, _body + context->getBytesSent(), bytesToSend);

  context->setBytesSent(context->getBytesSent() + bytesToSend);
}

HttpClientHandler::Result HttpEchoClientHandler::receiveResponseHeaders(
    ESB::SocketMultiplexer &multiplexer, HttpTransaction *transaction) {
  assert(transaction);
  HttpResponse &response = transaction->response();

  if (ESB_DEBUG_LOGGABLE) {
    ESB_LOG_DEBUG("Response headers parsed");
    ESB_LOG_DEBUG("StatusCode: %d", response.statusCode());
    ESB_LOG_DEBUG("ReasonPhrase: %s", ESB_SAFE_STR(response.reasonPhrase()));
    ESB_LOG_DEBUG("Version: HTTP/%d.%d", response.httpVersion() / 100,
                  response.httpVersion() % 100 / 10);

    for (HttpHeader *header = (HttpHeader *)response.headers().first(); header;
         header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("%s: %s", ESB_SAFE_STR(header->fieldName()),
                    ESB_SAFE_STR(header->fieldValue()));
    }
  }

  return HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONTINUE;
}

HttpClientHandler::Result HttpEchoClientHandler::receiveResponseBody(
    ESB::SocketMultiplexer &multiplexer, HttpTransaction *transaction,
    unsigned const char *chunk, unsigned int chunkSize) {
  assert(transaction);
  assert(chunk);

  if (0U == chunkSize) {
    ESB_LOG_DEBUG("Response body finished");
    return ES_HTTP_CLIENT_HANDLER_CONTINUE;
  }

  if (ESB_DEBUG_LOGGABLE) {
    char buffer[4096];
    unsigned int size =
        (sizeof(buffer) - 1) > chunkSize ? chunkSize : (sizeof(buffer) - 1);
    memcpy(buffer, chunk, size);
    buffer[size] = 0;
    ESB_LOG_DEBUG("Received body chunk: %s", buffer);
  }

  return ES_HTTP_CLIENT_HANDLER_CONTINUE;
}

void HttpEchoClientHandler::endClientTransaction(
    ESB::SocketMultiplexer &multiplexer, HttpTransaction *transaction,
    State state) {
  if (_totalTransactions == _completedTransactions.inc()) {
    ESB_LOG_NOTICE("All transactions completed");
  }

  assert(transaction);
  HttpEchoClientContext *context =
      (HttpEchoClientContext *)transaction->context();
  assert(context);
  ESB::Allocator &allocator = transaction->allocator();

  switch (state) {
    case ES_HTTP_CLIENT_HANDLER_BEGIN:
      ESB_LOG_INFO("Transaction failed at begin state");
      break;
    case ES_HTTP_CLIENT_HANDLER_RESOLVE:
      ESB_LOG_INFO("Transaction failed at resolve state");
      break;
    case ES_HTTP_CLIENT_HANDLER_CONNECT:
      ESB_LOG_INFO("Transaction failed at connect state");
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS:
      ESB_LOG_INFO("Transaction failed at send request headers state");
      break;
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY:
      ESB_LOG_INFO("Transaction failed at send request body state");
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS:
      ESB_LOG_INFO("Transaction failed at receive response headers state");
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY:
      ESB_LOG_INFO("Transaction failed at receive response body state");
      break;
    case ES_HTTP_CLIENT_HANDLER_END:
      ESB_LOG_DEBUG("Transaction finished");
      break;
    default:
      ESB_LOG_WARNING("Transaction failed at unknown state");
  }

  if (0U == context->getRemainingIterations()) {
    context->~HttpEchoClientContext();
    allocator.deallocate(context);
    transaction->setContext(0);
    return;
  }

  HttpClientTransaction *newTransaction = _pool->createClientTransaction(this);

  if (!newTransaction) {
    ESB_LOG_WARNING("Cannot create new transaction: bad alloc");
    return;
  }

  context->setRemainingIterations(context->getRemainingIterations() - 1);
  context->setBytesSent(0U);

  newTransaction->setContext(context);
  transaction->setContext(0);

  char dottedIP[ESB_IPV6_PRESENTATION_SIZE];
  transaction->peerAddress().presentationAddress(dottedIP, sizeof(dottedIP));

  ESB::Error error = HttpEchoClientRequestBuilder(
      dottedIP, transaction->peerAddress().port(), _absPath, _method,
      _contentType, newTransaction);

  if (ESB_SUCCESS != error) {
    _pool->destroyClientTransaction(newTransaction);
    ESB_LOG_WARNING_ERRNO(error, "cannot build request");
    return;
  }

  error = _pool->executeClientTransaction(newTransaction);

  if (ESB_SUCCESS != error) {
    _pool->destroyClientTransaction(newTransaction);
    ESB_LOG_WARNING_ERRNO(error, "cannot execute transaction");
    return;
  }

  ESB_LOG_DEBUG("Resubmitted transaction.  %u iterations remaining",
                context->getRemainingIterations());
}

}  // namespace ES
