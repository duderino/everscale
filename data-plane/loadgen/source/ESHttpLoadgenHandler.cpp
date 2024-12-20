#ifndef ES_HTTP_LOADGEN_HANDLER_H
#include <ESHttpLoadgenHandler.h>
#endif

#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

namespace ES {

HttpLoadgenHandler::HttpLoadgenHandler(const HttpTestParams &params, ESB::Allocator &allocator)
    : _params(params), _completedTransactions(), _clientCounters(5 * 60, 1, allocator) {}

HttpLoadgenHandler::~HttpLoadgenHandler() {}

ESB::Error HttpLoadgenHandler::beginTransaction(HttpMultiplexer &multiplexer, HttpClientStream &stream) {
  switch (_params.disruptTransaction()) {
    case HttpTestParams::STALL_CLIENT_SEND_HEADERS:
      stream.pauseSend(false);
      stream.pauseRecv(false);
      return ESB_PAUSE;
    case HttpTestParams::CLOSE_CLIENT_SEND_HEADERS:
      return ESB_CLOSED;
    case HttpTestParams::HAPPY_PATH:
    default:
      return ESB_SUCCESS;
  }
}

ESB::Error HttpLoadgenHandler::offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                                ESB::UInt64 *bytesAvailable) {
  switch (_params.disruptTransaction()) {
    case HttpTestParams::STALL_CLIENT_SEND_BODY:
      stream.pauseSend(false);
      stream.pauseRecv(false);
      return ESB_PAUSE;
    case HttpTestParams::CLOSE_CLIENT_SEND_BODY:
      return ESB_CLOSED;
    case HttpTestParams::HAPPY_PATH:
    default:
      break;
  }

  assert(bytesAvailable);
  HttpLoadgenContext *context = (HttpLoadgenContext *)stream.context();
  assert(context);
  assert(context->bytesSent() <= _params.requestSize());
  *bytesAvailable = _params.requestSize() - context->bytesSent();
  return ESB_SUCCESS;
}

ESB::Error HttpLoadgenHandler::produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                                  unsigned char *chunk, ESB::UInt64 bytesRequested) {
  assert(chunk);
  assert(0 < bytesRequested);
  HttpLoadgenContext *context = (HttpLoadgenContext *)stream.context();
  assert(context);

  const ESB::UInt64 bytesSent = context->bytesSent();
  assert(bytesSent <= _params.requestSize());
  assert(bytesRequested <= _params.requestSize() - bytesSent);

#ifndef NDEBUG
  // Verified in HttpOriginHandler::consumeRequestBody
  for (ESB::UInt64 i = 0; i < bytesRequested; ++i) {
    chunk[i] = 'a' + (bytesSent + i) % 26;
  }
#endif

  context->setBytesSent(bytesSent + bytesRequested);

  ESB_LOG_DEBUG("[%s] sent %lu or %lu/%lu request body bytes", stream.logAddress(), bytesRequested,
                context->bytesSent(), _params.requestSize());

  return ESB_SUCCESS;
}

ESB::Error HttpLoadgenHandler::receiveResponseHeaders(HttpMultiplexer &multiplexer, HttpClientStream &stream) {
  switch (_params.disruptTransaction()) {
    case HttpTestParams::STALL_CLIENT_RECV_HEADERS:
      stream.pauseSend(false);
      stream.pauseRecv(false);
      return ESB_PAUSE;
    case HttpTestParams::CLOSE_CLIENT_RECV_HEADERS:
      return ESB_CLOSED;
    case HttpTestParams::HAPPY_PATH:
    default:
      return ESB_SUCCESS;
  }
}

ESB::Error HttpLoadgenHandler::consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                                   const unsigned char *chunk, ESB::UInt64 chunkSize,
                                                   ESB::UInt64 *bytesConsumed) {
  switch (_params.disruptTransaction()) {
    case HttpTestParams::STALL_CLIENT_RECV_BODY:
      stream.pauseSend(false);
      stream.pauseRecv(false);
      return ESB_PAUSE;
    case HttpTestParams::CLOSE_CLIENT_RECV_BODY:
      return ESB_CLOSED;
    case HttpTestParams::HAPPY_PATH:
    default:
      break;
  }

  assert(chunk);
  assert(bytesConsumed);
  HttpLoadgenContext *context = (HttpLoadgenContext *)stream.context();
  assert(context);
  assert(chunkSize + context->bytesReceived() <= _params.responseSize());

#ifndef NDEBUG
  {
    ESB::UInt64 bytesReceived = context->bytesReceived();
    for (ESB::UInt64 i = 0; i < chunkSize; ++i) {
      char actual = chunk[i];
      char expected = 'A' + (bytesReceived + i) % 26;
      assert(actual == expected);
    }
  }
#endif

  *bytesConsumed = chunkSize;
  context->setBytesRecieved(context->bytesReceived() + chunkSize);

  ESB_LOG_DEBUG("[%s] received %lu or %lu/%lu response body bytes", stream.logAddress(), chunkSize,
                context->bytesReceived(), _params.responseSize());

  return ESB_SUCCESS;
}

void HttpLoadgenHandler::endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &stream, State state) {
  HttpLoadgenContext *context = (HttpLoadgenContext *) stream.context();
  assert(context);
  const ESB::Date &start = stream.transactionStartTime();
  const ESB::Date stop = ESB::Time::Instance().now();

  switch (state) {
    case ES_HTTP_CLIENT_HANDLER_BEGIN:
      _clientCounters.requestBeginError().record(start, stop);
      _clientCounters.failedTransactions().record(start, stop);
      ESB_LOG_INFO("[%s] transaction failed at begin state", stream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_RESOLVE:
      _clientCounters.requestResolveError().record(start, stop);
      _clientCounters.failedTransactions().record(start, stop);
      ESB_LOG_INFO("[%s] transaction failed at resolve state", stream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_CONNECT:
      _clientCounters.requestConnectError().record(start, stop);
      _clientCounters.failedTransactions().record(start, stop);
      ESB_LOG_INFO("[%s] transaction failed at connect state", stream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS:
      _clientCounters.requestHeaderSendError().record(start, stop);
      _clientCounters.failedTransactions().record(start, stop);
      ESB_LOG_INFO("[%s] transaction failed at send request headers state", stream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY:
      _clientCounters.requestBodySendError().record(start, stop);
      _clientCounters.failedTransactions().record(start, stop);
      ESB_LOG_INFO("[%s] transaction failed at send request body state", stream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS:
      _clientCounters.responseHeaderReceiveError().record(start, stop);
      _clientCounters.failedTransactions().record(start, stop);
      ESB_LOG_INFO("[%s] transaction failed at receive response headers state", stream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY:
      _clientCounters.responseBodyReceiveError().record(start, stop);
      _clientCounters.failedTransactions().record(start, stop);
      ESB_LOG_INFO("[%s] transaction failed at receive response body state", stream.logAddress());
      break;
    case ES_HTTP_CLIENT_HANDLER_END:
      _clientCounters.successfulTransactions().record(start, stop);
      _clientCounters.incrementStatusCounter(stream.response().statusCode(), start, stop);
      if (_params.responseSize() != context->bytesReceived()) {
        ESB_LOG_ERROR("[%s] expected %lu response body bytes but received %lu bytes (delta %lu)", stream.logAddress(),
                      _params.responseSize(), context->bytesReceived(),
                      _params.responseSize() - context->bytesReceived());
      }
      assert(context->bytesReceived() == _params.responseSize());
      break;
    default:
      assert(!"Transaction failed at unknown state");
      _clientCounters.failedTransactions().record(start, stop);
      ESB_LOG_ERROR("[%s] Transaction failed at unknown state %d", stream.logAddress(), state);
  }

  HttpLoadgenContext::IncCompletedIterations();
  // returns the value pre-decrement
  int remainingIterations = HttpLoadgenContext::DecRemainingIterations();

  if (0 > remainingIterations) {
    ESB::CleanupHandler &cleanupHandler = context->cleanupHandler();
    cleanupHandler.destroy(context);
    stream.setContext(NULL);
    return;
  }

  HttpClientTransaction *newTransaction = multiplexer.createClientTransaction();

  if (!newTransaction) {
    ESB_LOG_WARNING_ERRNO(ESB_OUT_OF_MEMORY, "Cannot create new transaction");
    return;
  }

  ESB::Error error = newTransaction->request().copy(&stream.request(), newTransaction->allocator());
  if (ESB_SUCCESS != error) {
    multiplexer.destroyClientTransaction(newTransaction);
    ESB_LOG_WARNING_ERRNO(error, "Cannot build request");
    return;
  }

  newTransaction->setPeerAddress(stream.peerAddress());
  context->setBytesSent(0U);
  context->setBytesRecieved(0U);
  newTransaction->setContext(context);
  stream.setContext(NULL);

  // TODO executeClientTransaction is adding newTransaction to a list on failure
  error = multiplexer.executeClientTransaction(newTransaction);

  if (ESB_SUCCESS != error) {
    ESB_LOG_WARNING_ERRNO(error, "Cannot execute transaction");
    multiplexer.destroyClientTransaction(newTransaction);
    return;
  }

  ESB_LOG_DEBUG("Resubmitted transaction.  %u iterations remaining", remainingIterations);
}

void HttpLoadgenHandler::dumpClientCounters(ESB::Logger &logger, ESB::Logger::Severity severity) const {
  _clientCounters.log(logger, severity);
}

ESB::Error HttpLoadgenHandler::endRequest(HttpMultiplexer &multiplexer, HttpClientStream &stream) {
  return ESB_SUCCESS;
}

}  // namespace ES
