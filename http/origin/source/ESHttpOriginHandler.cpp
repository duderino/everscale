#ifndef ES_HTTP_ORIGIN_HANDLER_H
#include <ESHttpOriginHandler.h>
#endif

#ifndef ES_HTTP_ORIGIN_CONTEXT_H
#include <ESHttpOriginContext.h>
#endif

#define BODY                                                                \
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?><SOAP-ENV:Envelope "           \
  "xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" "           \
  "xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" "           \
  "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "                \
  "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "                         \
  "xmlns:ns2=\"http://schemas.xmlsoap.org/ws/2002/07/secext\" "             \
  "xmlns:ns3=\"urn:yahoo:ysm:aws\" "                                        \
  "xmlns:ns1=\"urn:yahoo:ysm:aws:echo\"><SOAP-ENV:Header><ns2:Security><"   \
  "UsernameToken><Username>foo</Username><Password>bar</Password></"        \
  "UsernameToken></ns2:Security><ns3:licensekey>baz</ns3:licensekey></"     \
  "SOAP-ENV:Header><SOAP-ENV:Body><ns1:EchoResponseElement><Message>box10." \
  "burbank.corp.yahoo.com:8029:0xb7fddbb0</Message></"                      \
  "ns1:EchoResponseElement></SOAP-ENV:Body></SOAP-ENV:Envelope>"

namespace ES {

static unsigned int BodySize = (sizeof(BODY) - 1);

HttpOriginHandler::HttpOriginHandler() {}

HttpOriginHandler::~HttpOriginHandler() {}

ESB::Error HttpOriginHandler::acceptConnection(HttpMultiplexer &stack, ESB::SocketAddress *address) {
  return ESB_SUCCESS;
}

ESB::Error HttpOriginHandler::beginTransaction(HttpMultiplexer &stack, HttpServerStream &stream) {
  ESB::Allocator &allocator = stream.allocator();
  HttpOriginContext *context = new (allocator) HttpOriginContext();

  if (!context) {
    ESB_LOG_WARNING_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot allocate new transaction", stream.logAddress());
    return ESB_OUT_OF_MEMORY;
  }

  assert(!stream.context());
  stream.setContext(context);
  return ESB_SUCCESS;
}

ESB::Error HttpOriginHandler::receiveRequestHeaders(HttpMultiplexer &stack, HttpServerStream &stream) {
  return ESB_SUCCESS;
}

ESB::Error HttpOriginHandler::consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                                 unsigned const char *chunk, ESB::UInt32 chunkSize,
                                                 ESB::UInt32 *bytesConsumed) {
  assert(chunk);
  assert(bytesConsumed);

  *bytesConsumed = chunkSize;
  HttpResponse &response = stream.response();

  if (0U == chunkSize) {
    response.setStatusCode(200);
    response.setReasonPhrase("OK");
    return ESB_SEND_RESPONSE;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpOriginHandler::offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                                ESB::UInt32 *bytesAvailable) {
  HttpOriginContext *context = (HttpOriginContext *)stream.context();
  assert(context);
  *bytesAvailable = BodySize - context->getBytesSent();
  return ESB_SUCCESS;
}

ESB::Error HttpOriginHandler::produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                                  unsigned char *chunk, ESB::UInt32 bytesRequested) {
  assert(chunk);
  assert(0 < bytesRequested);
  HttpOriginContext *context = (HttpOriginContext *)stream.context();
  assert(context);
  assert(bytesRequested <= BodySize - context->getBytesSent());

  ESB::UInt32 totalBytesRemaining = BodySize - context->getBytesSent();
  ESB::UInt32 bytesToSend = bytesRequested > totalBytesRemaining ? totalBytesRemaining : bytesRequested;

  memcpy(chunk, ((unsigned char *)BODY) + context->getBytesSent(), bytesToSend);
  context->addBytesSent(bytesToSend);
  return ESB_SUCCESS;
}

void HttpOriginHandler::endTransaction(HttpMultiplexer &stack, HttpServerStream &stream, State state) {
  HttpOriginContext *context = (HttpOriginContext *)stream.context();
  assert(context);
  ESB::Allocator &allocator = stream.allocator();

  context->~HttpOriginContext();
  allocator.deallocate(context);
  stream.setContext(NULL);

  switch (state) {
    case ES_HTTP_SERVER_HANDLER_BEGIN:
      ESB_LOG_INFO("Transaction failed at begin state");
      break;
    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS:
      ESB_LOG_INFO("Transaction failed at request header parse state");
      break;
    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY:
      ESB_LOG_INFO("Transaction failed at request body parse state");
      break;
    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS:
      ESB_LOG_INFO("Transaction failed at response header send state");
      break;
    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY:
      ESB_LOG_INFO("Transaction failed at response header send state");
      break;
    case ES_HTTP_SERVER_HANDLER_END:
      break;
    default:
      ESB_LOG_WARNING("Transaction failed at unknown state");
  }
}

}  // namespace ES
