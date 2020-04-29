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
#include "ESHttpEchoClientHandler.h"

#endif

namespace ES {

HttpEchoClientHandler::HttpEchoClientHandler(const char *absPath,
                                             const char *method,
                                             const char *contentType,
                                             const unsigned char *body,
                                             int bodySize)
    : _absPath(absPath),
      _method(method),
      _contentType(contentType),
      _body(body),
      _bodySize(bodySize),
      _completedTransactions() {}

HttpEchoClientHandler::~HttpEchoClientHandler() {}

ESB::UInt32 HttpEchoClientHandler::reserveRequestChunk(HttpClientStack &stack,
                                                       HttpStream &stream) {
  HttpEchoClientContext *context = (HttpEchoClientContext *)stream.context();
  assert(context);
  return _bodySize - context->bytesSent();
}

void HttpEchoClientHandler::fillRequestChunk(HttpClientStack &stack,
                                             HttpStream &stream,
                                             unsigned char *chunk,
                                             ESB::UInt32 chunkSize) {
  assert(chunk);
  assert(0 < chunkSize);

  HttpEchoClientContext *context = (HttpEchoClientContext *)stream.context();
  assert(context);

  ESB::UInt32 totalBytesRemaining = _bodySize - context->bytesSent();
  ESB::UInt32 bytesToSend =
      chunkSize > totalBytesRemaining ? totalBytesRemaining : chunkSize;

  memcpy(chunk, _body + context->bytesSent(), bytesToSend);

  context->setBytesSent(context->bytesSent() + bytesToSend);
}

HttpClientHandler::Result HttpEchoClientHandler::receiveResponseHeaders(
    HttpClientStack &stack, HttpStream &stream) {
  return HttpClientHandler::ES_HTTP_CLIENT_HANDLER_CONTINUE;
}

ESB::UInt32 HttpEchoClientHandler::reserveResponseChunk(HttpClientStack &stack,
                                                        HttpStream &stream) {
  return ESB_UINT32_MAX;
}

HttpClientHandler::Result HttpEchoClientHandler::receiveResponseChunk(
    HttpClientStack &stack, HttpStream &stream, unsigned const char *chunk,
    ESB::UInt32 chunkSize) {
  assert(chunk);

  if (0U == chunkSize) {
    return ES_HTTP_CLIENT_HANDLER_CONTINUE;
  }

  return ES_HTTP_CLIENT_HANDLER_CONTINUE;
}

void HttpEchoClientHandler::endClientTransaction(HttpClientStack &stack,
                                                 HttpStream &stream,
                                                 State state) {
  HttpEchoClientContext *context = (HttpEchoClientContext *)stream.context();
  assert(context);

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
      break;
    default:
      ESB_LOG_WARNING("Transaction failed at unknown state");
  }

  HttpEchoClientContext::IncCompletedIterations();
  // returns the value pre-decrement
  int remainingIterations = HttpEchoClientContext::DecRemainingIterations();

  if (0 > remainingIterations) {
    ESB::CleanupHandler &cleanupHandler = context->cleanupHandler();
    cleanupHandler.destroy(context);
    stream.setContext(NULL);
    return;
  }

  HttpClientTransaction *newTransaction = stack.createClientTransaction();

  if (!newTransaction) {
    ESB_LOG_WARNING_ERRNO(ESB_OUT_OF_MEMORY, "Cannot create new transaction");
    return;
  }

  context->setBytesSent(0U);

  newTransaction->setContext(context);
  stream.setContext(NULL);

  char dottedIP[ESB_IPV6_PRESENTATION_SIZE];
  stream.peerAddress().presentationAddress(dottedIP, sizeof(dottedIP));

  ESB::Error error = HttpEchoClientRequestBuilder(
      dottedIP, stream.peerAddress().port(), _absPath, _method, _contentType,
      newTransaction);

  if (ESB_SUCCESS != error) {
    stack.destroyTransaction(newTransaction);
    ESB_LOG_WARNING_ERRNO(error, "Cannot build request");
    return;
  }

  error = stack.executeTransaction(newTransaction);

  if (ESB_SUCCESS != error) {
    stack.destroyTransaction(newTransaction);
    ESB_LOG_WARNING_ERRNO(error, "Cannot execute transaction");
    return;
  }

  ESB_LOG_DEBUG("Resubmitted transaction.  %u iterations remaining",
                remainingIterations);
}
void HttpEchoClientHandler::receivePaused(HttpClientStack &stack,
                                          HttpStream &stream) {
  assert(0 == "HttpEchoClientHandler should not be paused");
}

}  // namespace ES
