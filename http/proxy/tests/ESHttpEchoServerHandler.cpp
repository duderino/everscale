#ifndef ES_HTTP_ECHO_SERVER_HANDLER_H
#include <ESHttpEchoServerHandler.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
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
    ESB::SocketMultiplexer &multiplexer, ESB::SocketAddress *address) {
  if (ESB_INFO_LOGGABLE) {
    char dottedIP[ESB_IPV6_PRESENTATION_SIZE];
    address->presentationAddress(dottedIP, sizeof(dottedIP));
    ESB_LOG_INFO("Accepted new connection from %s:%u", dottedIP,
                 address->port());
  }
  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpEchoServerHandler::beginServerTransaction(
    ESB::SocketMultiplexer &multiplexer, HttpTransaction *transaction) {
  assert(transaction);
  ESB::Allocator &allocator = transaction->getAllocator();

  if (ESB_DEBUG_LOGGABLE) {
    char dottedIP[ESB_IPV6_PRESENTATION_SIZE];
    transaction->getPeerAddress()->presentationAddress(dottedIP,
                                                       sizeof(dottedIP));
    ESB_LOG_DEBUG("Begin new transaction with %s:%u", dottedIP,
                  transaction->getPeerAddress()->port());
  }

  HttpEchoServerContext *context = new (allocator) HttpEchoServerContext();

  if (!context && ESB_WARNING_LOGGABLE) {
    char dottedIP[ESB_IPV6_PRESENTATION_SIZE];
    transaction->getPeerAddress()->presentationAddress(dottedIP,
                                                       sizeof(dottedIP));
    ESB_LOG_WARNING("Cannot allocate new transaction for %s:%u", dottedIP,
                    transaction->getPeerAddress()->port());
    return ES_HTTP_SERVER_HANDLER_CLOSE;
  }

  assert(!transaction->getApplicationContext());
  transaction->setApplicationContext(context);
  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpEchoServerHandler::receiveRequestHeaders(
    ESB::SocketMultiplexer &multiplexer, HttpTransaction *transaction) {
  assert(transaction);
  HttpRequest *request = transaction->getRequest();
  assert(request);
  HttpRequestUri *requestUri = request->getRequestUri();
  assert(requestUri);

  if (ESB_DEBUG_LOGGABLE) {
    ESB_LOG_DEBUG("Received request headers");
    ESB_LOG_DEBUG("Method: %s", transaction->getRequest()->getMethod());

    switch (transaction->getRequest()->getRequestUri()->getType()) {
      case HttpRequestUri::ES_URI_ASTERISK:
        ESB_LOG_DEBUG("Asterisk Request-URI");
        break;
      case HttpRequestUri::ES_URI_HTTP:
      case HttpRequestUri::ES_URI_HTTPS:
        ESB_LOG_DEBUG("Scheme: %s",
                      HttpRequestUri::ES_URI_HTTP == requestUri->getType()
                          ? "http"
                          : "https");
        ESB_LOG_DEBUG("Host: %s", ESB_SAFE_STR(requestUri->getHost()));
        ESB_LOG_DEBUG("Port: %d", requestUri->getPort());
        ESB_LOG_DEBUG("AbsPath: %s", ESB_SAFE_STR(requestUri->getAbsPath()));
        ESB_LOG_DEBUG("Query: %s", ESB_SAFE_STR(requestUri->getQuery()));
        ESB_LOG_DEBUG("Fragment: %s", ESB_SAFE_STR(requestUri->getFragment()));
        break;
      case HttpRequestUri::ES_URI_OTHER:
        ESB_LOG_DEBUG("Other: %s", ESB_SAFE_STR(requestUri->getOther()));
        break;
    }

    ESB_LOG_DEBUG("Version: HTTP/%d.%d", request->getHttpVersion() / 100,
                  request->getHttpVersion() % 100 / 10);

    for (HttpHeader *header = (HttpHeader *)request->headers().first(); header;
         header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("%s: %s", ESB_SAFE_STR(header->fieldName()),
                    ESB_SAFE_STR(header->fieldValue()));
    }
  }

  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpEchoServerHandler::receiveRequestBody(
    ESB::SocketMultiplexer &multiplexer, HttpTransaction *transaction,
    unsigned const char *chunk, unsigned int chunkSize) {
  assert(transaction);
  assert(chunk);

  HttpResponse *response = transaction->getResponse();

  assert(response);

  if (0U == chunkSize) {
    ESB_LOG_DEBUG("Request body finished");
    response->setStatusCode(200);
    response->setReasonPhrase((const unsigned char *)"OK");
    return ES_HTTP_SERVER_HANDLER_SEND_RESPONSE;
  }

  if (ESB_DEBUG_LOGGABLE) {
    char buffer[4096];
    unsigned int size =
        (sizeof(buffer) - 1) > chunkSize ? chunkSize : (sizeof(buffer) - 1);
    memcpy(buffer, chunk, size);
    buffer[size] = 0;
    ESB_LOG_DEBUG("Received body chunk: %s", buffer);
  }

  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

int HttpEchoServerHandler::reserveResponseChunk(
    ESB::SocketMultiplexer &multiplexer, HttpTransaction *transaction) {
  HttpEchoServerContext *context =
      (HttpEchoServerContext *)transaction->getApplicationContext();
  assert(context);
  return BodySize - context->getBytesSent();
}

void HttpEchoServerHandler::fillResponseChunk(
    ESB::SocketMultiplexer &multiplexer, HttpTransaction *transaction,
    unsigned char *chunk, unsigned int chunkSize) {
  assert(transaction);
  assert(chunk);
  assert(0 < chunkSize);
  HttpEchoServerContext *context =
      (HttpEchoServerContext *)transaction->getApplicationContext();
  assert(context);

  unsigned int totalBytesRemaining = BodySize - context->getBytesSent();
  unsigned int bytesToSend =
      chunkSize > totalBytesRemaining ? totalBytesRemaining : chunkSize;

  memcpy(chunk, ((unsigned char *)BODY) + context->getBytesSent(), bytesToSend);

  context->addBytesSent(bytesToSend);
}

void HttpEchoServerHandler::endServerTransaction(
    ESB::SocketMultiplexer &multiplexer, HttpTransaction *transaction,
    HttpServerHandler::State state) {
  assert(transaction);
  HttpEchoServerContext *context =
      (HttpEchoServerContext *)transaction->getApplicationContext();
  assert(context);
  ESB::Allocator &allocator = transaction->getAllocator();

  context->~HttpEchoServerContext();
  allocator.deallocate(context);
  transaction->setApplicationContext(0);

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
      ESB_LOG_DEBUG("Transaction finished");
      break;
    default:
      ESB_LOG_WARNING("Transaction failed at unknown state");
  }
}

}  // namespace ES
