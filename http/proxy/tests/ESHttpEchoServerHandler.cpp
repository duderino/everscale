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

HttpEchoServerHandler::HttpEchoServerHandler(ESB::Logger *logger)
    : _logger(logger) {}

HttpEchoServerHandler::~HttpEchoServerHandler() {}

HttpServerHandler::Result HttpEchoServerHandler::acceptConnection(
    ESB::SocketAddress *address) {
  if (_logger->isLoggable(ESB::Logger::Debug)) {
    char dottedIP[16];

    address->getIPAddress(dottedIP, sizeof(dottedIP));

    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[handler] Accepted new connection: %s/%u", dottedIP,
                 address->getPort());
  }

  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpEchoServerHandler::begin(
    HttpTransaction *transaction) {
  assert(transaction);

  ESB::Allocator *allocator = transaction->getAllocator();

  assert(allocator);

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    char dottedIP[16];

    transaction->getPeerAddress()->getIPAddress(dottedIP, sizeof(dottedIP));

    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[handler] Beginning new transaction: %s/%u", dottedIP,
                 transaction->getPeerAddress()->getPort());
  }

  HttpEchoServerContext *context = new (allocator) HttpEchoServerContext();

  if (0 == context) {
    if (_logger->isLoggable(ESB::Logger::Critical)) {
      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[handler] Cannot allocate new context");
    }

    return ES_HTTP_SERVER_HANDLER_CLOSE;
  }

  assert(0 == transaction->getApplicationContext());

  transaction->setApplicationContext(context);

  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpEchoServerHandler::receiveRequestHeaders(
    HttpTransaction *transaction) {
  assert(transaction);

  HttpRequest *request = transaction->getRequest();

  assert(request);

  HttpRequestUri *requestUri = request->getRequestUri();

  assert(requestUri);

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__, "[handler] Method: %s",
                 transaction->getRequest()->getMethod());

    switch (transaction->getRequest()->getRequestUri()->getType()) {
      case HttpRequestUri::ES_URI_ASTERISK:

        _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                     "[handler] Asterisk Request-URI");

        break;

      case HttpRequestUri::ES_URI_HTTP:
      case HttpRequestUri::ES_URI_HTTPS:

        _logger->log(
            ESB::Logger::Debug, __FILE__, __LINE__, "[handler] Scheme: %s",
            HttpRequestUri::ES_URI_HTTP == requestUri->getType() ? "http"
                                                                 : "https");
        _logger->log(
            ESB::Logger::Debug, __FILE__, __LINE__, "[handler] Host: %s",
            0 == requestUri->getHost() ? "none"
                                       : (const char *)requestUri->getHost());
        _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                     "[handler] Port: %d", requestUri->getPort());
        _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                     "[handler] AbsPath: %s", requestUri->getAbsPath());
        _logger->log(
            ESB::Logger::Debug, __FILE__, __LINE__, "[handler] Query: %s",
            requestUri->getQuery() ? "none"
                                   : (const char *)requestUri->getQuery());
        _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                     "[handler] Fragment: %s",
                     requestUri->getFragment()
                         ? "none"
                         : (const char *)requestUri->getFragment());

        break;

      case HttpRequestUri::ES_URI_OTHER:

        _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                     "[handler] Other: %s", requestUri->getOther());

        break;
    }

    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[handler] Version: HTTP/%d.%d\n",
                 request->getHttpVersion() / 100,
                 request->getHttpVersion() % 100 / 10);

    for (HttpHeader *header = (HttpHeader *)request->getHeaders()->getFirst();
         header; header = (HttpHeader *)header->getNext()) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__, "[handler] %s: %s\n",
                   (const char *)header->getFieldName(),
                   0 == header->getFieldValue()
                       ? "null"
                       : (const char *)header->getFieldValue());
    }
  }

  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

HttpServerHandler::Result HttpEchoServerHandler::receiveRequestBody(
    HttpTransaction *transaction, unsigned const char *chunk,
    unsigned int chunkSize) {
  assert(transaction);
  assert(chunk);

  HttpResponse *response = transaction->getResponse();

  assert(response);

  if (0U == chunkSize) {
    if (_logger->isLoggable(ESB::Logger::Debug)) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[handler] Request body finished");
    }

    response->setStatusCode(200);
    response->setReasonPhrase((const unsigned char *)"OK");

    return ES_HTTP_SERVER_HANDLER_SEND_RESPONSE;
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

  return ES_HTTP_SERVER_HANDLER_CONTINUE;
}

int HttpEchoServerHandler::reserveResponseChunk(HttpTransaction *transaction) {
  HttpEchoServerContext *context =
      (HttpEchoServerContext *)transaction->getApplicationContext();

  assert(context);

  return BodySize - context->getBytesSent();
}

void HttpEchoServerHandler::fillResponseChunk(HttpTransaction *transaction,
                                              unsigned char *chunk,
                                              unsigned int chunkSize) {
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

void HttpEchoServerHandler::end(HttpTransaction *transaction,
                                HttpServerHandler::State state) {
  assert(transaction);

  HttpEchoServerContext *context =
      (HttpEchoServerContext *)transaction->getApplicationContext();

  assert(context);

  ESB::Allocator *allocator = transaction->getAllocator();

  assert(allocator);

  context->~HttpEchoServerContext();
  allocator->deallocate(context);
  transaction->setApplicationContext(0);

  switch (state) {
    case ES_HTTP_SERVER_HANDLER_BEGIN:

      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                     "[handler] Transaction failed at begin state");
      }

      break;

    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS:

      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(
            ESB::Logger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at request header parse state");
      }

      break;

    case ES_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY:

      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(
            ESB::Logger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at request body parse state");
      }

      break;

    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS:

      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(
            ESB::Logger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at response header send state");
      }

      break;

    case ES_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY:

      if (_logger->isLoggable(ESB::Logger::Warning)) {
        _logger->log(
            ESB::Logger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at response header send state");
      }

      break;

    case ES_HTTP_SERVER_HANDLER_END:

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
}

}  // namespace ES
