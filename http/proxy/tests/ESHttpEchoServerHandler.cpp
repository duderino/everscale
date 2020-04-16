#ifndef ES_HTTP_ECHO_SERVER_HANDLER_H
#include <ESHttpEchoServerHandler.h>
#endif

#ifndef ES_HTTP_ECHO_SERVER_CONTEXT_H
#include <ESHttpEchoServerContext.h>
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

HttpEchoServerHandler::HttpEchoServerHandler() {}

HttpEchoServerHandler::~HttpEchoServerHandler() {}

HttpServerHandler::Result HttpEchoServerHandler::acceptConnection(
    HttpServerStack &stack, ESB::SocketAddress *address) {
  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpEchoServerHandler::beginServerTransaction(
    HttpServerStack &stack, HttpServerTransaction *transaction) {
  assert(transaction);
  ESB::Allocator &allocator = transaction->allocator();
  HttpEchoServerContext *context = new (allocator) HttpEchoServerContext();

  if (!context) {
    if (ESB_WARNING_LOGGABLE) {
      char dottedIP[ESB_IPV6_PRESENTATION_SIZE];
      transaction->peerAddress().presentationAddress(dottedIP,
                                                     sizeof(dottedIP));
      ESB_LOG_WARNING("Cannot allocate new transaction for %s:%u", dottedIP,
                      transaction->peerAddress().port());
    }
    return ES_HTTP_SERVER_HANDLER_CLOSE;
  }

  assert(!transaction->context());
  transaction->setContext(context);
  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpEchoServerHandler::receiveRequestHeaders(
    HttpServerStack &stack, HttpServerTransaction *transaction) {
  assert(transaction);
  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpEchoServerHandler::receiveRequestBody(
    HttpServerStack &stack, HttpServerTransaction *transaction,
    unsigned const char *chunk, unsigned int chunkSize) {
  assert(transaction);
  assert(chunk);

  HttpResponse &response = transaction->response();

  if (0U == chunkSize) {
    response.setStatusCode(200);
    response.setReasonPhrase("OK");
    return ES_HTTP_SERVER_HANDLER_SEND_RESPONSE;
  }

  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

int HttpEchoServerHandler::reserveResponseChunk(
    HttpServerStack &stack, HttpServerTransaction *transaction) {
  HttpEchoServerContext *context =
      (HttpEchoServerContext *)transaction->context();
  assert(context);
  return BodySize - context->getBytesSent();
}

void HttpEchoServerHandler::fillResponseChunk(
    HttpServerStack &stack, HttpServerTransaction *transaction,
    unsigned char *chunk, unsigned int chunkSize) {
  assert(transaction);
  assert(chunk);
  assert(0 < chunkSize);
  HttpEchoServerContext *context =
      (HttpEchoServerContext *)transaction->context();
  assert(context);

  unsigned int totalBytesRemaining = BodySize - context->getBytesSent();
  unsigned int bytesToSend =
      chunkSize > totalBytesRemaining ? totalBytesRemaining : chunkSize;

  memcpy(chunk, ((unsigned char *)BODY) + context->getBytesSent(), bytesToSend);

  context->addBytesSent(bytesToSend);
}

void HttpEchoServerHandler::endServerTransaction(
    HttpServerStack &stack, HttpServerTransaction *transaction,
    HttpServerHandler::State state) {
  assert(transaction);
  HttpEchoServerContext *context =
      (HttpEchoServerContext *)transaction->context();
  assert(context);
  ESB::Allocator &allocator = transaction->allocator();

  context->~HttpEchoServerContext();
  allocator.deallocate(context);
  transaction->setContext(0);

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
