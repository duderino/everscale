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

HttpServerHandler::Result HttpOriginHandler::acceptConnection(
    HttpServerStack &stack, ESB::SocketAddress *address) {
  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpOriginHandler::beginServerTransaction(
    HttpServerStack &stack, HttpStream &stream) {
  ESB::Allocator &allocator = stream.allocator();
  HttpOriginContext *context = new (allocator) HttpOriginContext();

  if (!context) {
    ESB_LOG_WARNING("[%s] Cannot allocate new transaction",
                    stream.logAddress());
    return ES_HTTP_SERVER_HANDLER_CLOSE;
  }

  assert(!stream.context());
  stream.setContext(context);
  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpOriginHandler::receiveRequestHeaders(
    HttpServerStack &stack, HttpStream &stream) {
  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpOriginHandler::receiveRequestChunk(
    HttpServerStack &stack, HttpStream &stream, unsigned const char *chunk,
    ESB::UInt32 chunkSize) {
  assert(chunk);

  HttpResponse &response = stream.response();

  if (0U == chunkSize) {
    response.setStatusCode(200);
    response.setReasonPhrase("OK");
    return ES_HTTP_SERVER_HANDLER_SEND_RESPONSE;
  }

  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

ESB::UInt32 HttpOriginHandler::reserveResponseChunk(HttpServerStack &stack,
                                                    HttpStream &stream) {
  HttpOriginContext *context = (HttpOriginContext *)stream.context();
  assert(context);
  return BodySize - context->getBytesSent();
}

void HttpOriginHandler::fillResponseChunk(HttpServerStack &stack,
                                          HttpStream &stream,
                                          unsigned char *chunk,
                                          ESB::UInt32 chunkSize) {
  assert(chunk);
  assert(0 < chunkSize);
  HttpOriginContext *context = (HttpOriginContext *)stream.context();
  assert(context);

  unsigned int totalBytesRemaining = BodySize - context->getBytesSent();
  unsigned int bytesToSend =
      chunkSize > totalBytesRemaining ? totalBytesRemaining : chunkSize;

  memcpy(chunk, ((unsigned char *)BODY) + context->getBytesSent(), bytesToSend);

  context->addBytesSent(bytesToSend);
}

void HttpOriginHandler::endServerTransaction(HttpServerStack &stack,
                                             HttpStream &stream, State state) {
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

ESB::UInt32 HttpOriginHandler::reserveRequestChunk(HttpServerStack &stack,
                                                   HttpStream &stream) {
  return ESB_UINT32_MAX;
}

void HttpOriginHandler::receivePaused(HttpServerStack &stack,
                                      HttpStream &stream) {
  assert(0 == "HttpOriginHandler should not be paused");
}

}  // namespace ES
