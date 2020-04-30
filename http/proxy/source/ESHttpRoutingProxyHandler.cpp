#ifndef ES_HTTP_ROUTING_PROXY_HANDLER_H
#include <ESHttpRoutingProxyHandler.h>
#endif

#ifndef ES_HTTP_PROXY_CONTEXT_H
#include <ESHttpRoutingProxyContext.h>
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

HttpRoutingProxyHandler::HttpRoutingProxyHandler(HttpRouter &router)
    : _router(router) {}

HttpRoutingProxyHandler::~HttpRoutingProxyHandler() {}

HttpServerHandler::Result HttpRoutingProxyHandler::acceptConnection(
    HttpServerStack &stack, ESB::SocketAddress *address) {
  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpRoutingProxyHandler::beginServerTransaction(
    HttpServerStack &stack, HttpStream &stream) {
  HttpRoutingProxyContext *context =
      new (stream.allocator()) HttpRoutingProxyContext();

  if (!context) {
    ESB_LOG_WARNING_ERRNO(ESB_OUT_OF_MEMORY, "[%s] Cannot create proxy context",
                          stream.logAddress());
    return ES_HTTP_SERVER_HANDLER_CLOSE;
  }

  assert(!stream.context());
  stream.setContext(context);
  context->setInboundStream(&stream);

  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpRoutingProxyHandler::receiveRequestHeaders(
    HttpServerStack &stack, HttpStream &stream) {
  // TODO validate headers

  HttpClientTransaction *transaction = stack.createClientTransaction();

  if (!transaction) {
    ESB_LOG_WARNING_ERRNO(ESB_OUT_OF_MEMORY,
                          "[%s] Cannot create client transaction",
                          stream.logAddress());
    return sendResponse(stream, 500, "Internal Server Error");
  }

  ESB::SocketAddress destination;
  ESB::Error error = _router.route(stream, transaction, destination);

  if (ESB_SUCCESS != error) {
    switch (error) {
      case ESB_CANNOT_FIND:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot route request",
                            stream.logAddress());
        return sendResponse(stream, 404, "Not Found");
      case ESB_NOT_OWNER:
        ESB_LOG_DEBUG_ERRNO(error, "[%s] Cannot route request",
                            stream.logAddress());
        return sendResponse(stream, 403, "Forbidden");
      default:
        ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot route request",
                              stream.logAddress());
        return sendResponse(stream, 500, "Internal Server Error");
    }
  }

  transaction->setContext(stream.context());

  char dottedIP[ESB_IPV6_PRESENTATION_SIZE];
  stream.peerAddress().presentationAddress(dottedIP, sizeof(dottedIP));

  error = stack.executeTransaction(transaction);

  if (ESB_SUCCESS != error) {
    stack.destroyTransaction(transaction);
    ESB_LOG_WARNING_ERRNO(error, "[%s] Cannot execute transaction",
                          stream.logAddress());
    return sendResponse(stream, 500, "Internal Server Error");
  }

  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

ESB::UInt32 HttpRoutingProxyHandler::reserveRequestChunk(HttpServerStack &stack,
                                                         HttpStream &stream) {
  // TODO return 0 until we get a OK response from upstream.  When we get
  // that response from upstream, resume the inbound stream if it is paused.
  // Alt, if we have a bad upstream interaction, close the stream.  Make sure
  // close works even if stream isn't paused.
  // TODO maybe stream.resume and stream.cancel should just work even if not
  // paused
  return ESB_UINT32_MAX;
}

HttpServerHandler::Result HttpRoutingProxyHandler::receiveRequestChunk(
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

ESB::UInt32 HttpRoutingProxyHandler::reserveResponseChunk(
    HttpServerStack &stack, HttpStream &stream) {
  HttpRoutingProxyContext *context =
      (HttpRoutingProxyContext *)stream.context();
  assert(context);
  return BodySize - context->getBytesSent();
}

void HttpRoutingProxyHandler::fillResponseChunk(HttpServerStack &stack,
                                                HttpStream &stream,
                                                unsigned char *chunk,
                                                ESB::UInt32 chunkSize) {
  assert(chunk);
  assert(0 < chunkSize);
  HttpRoutingProxyContext *context =
      (HttpRoutingProxyContext *)stream.context();
  assert(context);

  unsigned int totalBytesRemaining = BodySize - context->getBytesSent();
  unsigned int bytesToSend =
      chunkSize > totalBytesRemaining ? totalBytesRemaining : chunkSize;

  memcpy(chunk, ((unsigned char *)BODY) + context->getBytesSent(), bytesToSend);

  context->addBytesSent(bytesToSend);
}

void HttpRoutingProxyHandler::endServerTransaction(
    HttpServerStack &stack, HttpStream &stream,
    HttpServerHandler::State state) {
  HttpRoutingProxyContext *context =
      (HttpRoutingProxyContext *)stream.context();
  assert(context);
  ESB::Allocator &allocator = stream.allocator();

  context->~HttpRoutingProxyContext();
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

void HttpRoutingProxyHandler::receivePaused(HttpServerStack &stack,
                                            HttpStream &stream) {
  assert(0 == "HttpProxy should not be paused");
}
ESB::UInt32 HttpRoutingProxyHandler::reserveRequestChunk(HttpClientStack &stack,
                                                         HttpStream &stream) {
  return 0;
}
void HttpRoutingProxyHandler::fillRequestChunk(HttpClientStack &stack,
                                               HttpStream &stream,
                                               unsigned char *chunk,
                                               ESB::UInt32 chunkSize) {}
HttpClientHandler::Result HttpRoutingProxyHandler::receiveResponseHeaders(
    HttpClientStack &stack, HttpStream &stream) {
  return ES_HTTP_CLIENT_HANDLER_CLOSE;
}
ESB::UInt32 HttpRoutingProxyHandler::reserveResponseChunk(
    HttpClientStack &stack, HttpStream &stream) {
  return 0;
}
void HttpRoutingProxyHandler::receivePaused(HttpClientStack &stack,
                                            HttpStream &stream) {}
HttpClientHandler::Result HttpRoutingProxyHandler::receiveResponseChunk(
    HttpClientStack &stack, HttpStream &stream, unsigned const char *chunk,
    ESB::UInt32 chunkSize) {
  return ES_HTTP_CLIENT_HANDLER_CLOSE;
}
void HttpRoutingProxyHandler::endClientTransaction(
    HttpClientStack &stack, HttpStream &stream,
    HttpClientHandler::State state) {}

HttpServerHandler::Result HttpRoutingProxyHandler::sendResponse(
    HttpStream &stream, int statusCode, const char *reasonPhrase) {
  stream.response().setStatusCode(500);
  stream.response().setReasonPhrase("Internal Server Error");
  return ES_HTTP_SERVER_HANDLER_SEND_RESPONSE;
}

}  // namespace ES
