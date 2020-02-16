/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_ECHO_SERVER_HANDLER_H
#include <AWSHttpEchoServerHandler.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

#ifndef AWS_HTTP_ECHO_SERVER_CONTEXT_H
#include <AWSHttpEchoServerContext.h>
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
static unsigned int BodySize = (sizeof(BODY) - 1);

AWSHttpEchoServerHandler::AWSHttpEchoServerHandler(ESFLogger *logger)
    : _logger(logger) {}

AWSHttpEchoServerHandler::~AWSHttpEchoServerHandler() {}

AWSHttpServerHandler::Result AWSHttpEchoServerHandler::acceptConnection(
    ESFSocketAddress *address) {
  if (_logger->isLoggable(ESFLogger::Debug)) {
    char dottedIP[16];

    address->getIPAddress(dottedIP, sizeof(dottedIP));

    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[handler] Accepted new connection: %s/%u", dottedIP,
                 address->getPort());
  }

  return AWS_HTTP_SERVER_HANDLER_CONTINUE;
}

AWSHttpServerHandler::Result AWSHttpEchoServerHandler::begin(
    AWSHttpTransaction *transaction) {
  ESF_ASSERT(transaction);

  ESFAllocator *allocator = transaction->getAllocator();

  ESF_ASSERT(allocator);

  if (_logger->isLoggable(ESFLogger::Debug)) {
    char dottedIP[16];

    transaction->getPeerAddress()->getIPAddress(dottedIP, sizeof(dottedIP));

    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[handler] Beginning new transaction: %s/%u", dottedIP,
                 transaction->getPeerAddress()->getPort());
  }

  AWSHttpEchoServerContext *context =
      new (allocator) AWSHttpEchoServerContext();

  if (0 == context) {
    if (_logger->isLoggable(ESFLogger::Critical)) {
      _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                   "[handler] Cannot allocate new context");
    }

    return AWS_HTTP_SERVER_HANDLER_CLOSE;
  }

  ESF_ASSERT(0 == transaction->getApplicationContext());

  transaction->setApplicationContext(context);

  return AWS_HTTP_SERVER_HANDLER_CONTINUE;
}

AWSHttpServerHandler::Result AWSHttpEchoServerHandler::receiveRequestHeaders(
    AWSHttpTransaction *transaction) {
  ESF_ASSERT(transaction);

  AWSHttpRequest *request = transaction->getRequest();

  ESF_ASSERT(request);

  AWSHttpRequestUri *requestUri = request->getRequestUri();

  ESF_ASSERT(requestUri);

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__, "[handler] Method: %s",
                 transaction->getRequest()->getMethod());

    switch (transaction->getRequest()->getRequestUri()->getType()) {
      case AWSHttpRequestUri::AWS_URI_ASTERISK:

        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[handler] Asterisk Request-URI");

        break;

      case AWSHttpRequestUri::AWS_URI_HTTP:
      case AWSHttpRequestUri::AWS_URI_HTTPS:

        _logger->log(
            ESFLogger::Debug, __FILE__, __LINE__, "[handler] Scheme: %s",
            AWSHttpRequestUri::AWS_URI_HTTP == requestUri->getType() ? "http"
                                                                     : "https");
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__, "[handler] Host: %s",
                     0 == requestUri->getHost()
                         ? "none"
                         : (const char *)requestUri->getHost());
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__, "[handler] Port: %d",
                     requestUri->getPort());
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[handler] AbsPath: %s", requestUri->getAbsPath());
        _logger->log(
            ESFLogger::Debug, __FILE__, __LINE__, "[handler] Query: %s",
            requestUri->getQuery() ? "none"
                                   : (const char *)requestUri->getQuery());
        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[handler] Fragment: %s",
                     requestUri->getFragment()
                         ? "none"
                         : (const char *)requestUri->getFragment());

        break;

      case AWSHttpRequestUri::AWS_URI_OTHER:

        _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                     "[handler] Other: %s", requestUri->getOther());

        break;
    }

    _logger->log(
        ESFLogger::Debug, __FILE__, __LINE__, "[handler] Version: HTTP/%d.%d\n",
        request->getHttpVersion() / 100, request->getHttpVersion() % 100 / 10);

    for (AWSHttpHeader *header =
             (AWSHttpHeader *)request->getHeaders()->getFirst();
         header; header = (AWSHttpHeader *)header->getNext()) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__, "[handler] %s: %s\n",
                   (const char *)header->getFieldName(),
                   0 == header->getFieldValue()
                       ? "null"
                       : (const char *)header->getFieldValue());
    }
  }

  return AWS_HTTP_SERVER_HANDLER_CONTINUE;
}

AWSHttpServerHandler::Result AWSHttpEchoServerHandler::receiveRequestBody(
    AWSHttpTransaction *transaction, unsigned const char *chunk,
    unsigned int chunkSize) {
  ESF_ASSERT(transaction);
  ESF_ASSERT(chunk);

  AWSHttpResponse *response = transaction->getResponse();

  ESF_ASSERT(response);

  if (0U == chunkSize) {
    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[handler] Request body finished");
    }

    response->setStatusCode(200);
    response->setReasonPhrase((const unsigned char *)"OK");

    return AWS_HTTP_SERVER_HANDLER_SEND_RESPONSE;
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

  return AWS_HTTP_SERVER_HANDLER_CONTINUE;
}

int AWSHttpEchoServerHandler::reserveResponseChunk(
    AWSHttpTransaction *transaction) {
  AWSHttpEchoServerContext *context =
      (AWSHttpEchoServerContext *)transaction->getApplicationContext();

  ESF_ASSERT(context);

  return BodySize - context->getBytesSent();
}

void AWSHttpEchoServerHandler::fillResponseChunk(
    AWSHttpTransaction *transaction, unsigned char *chunk,
    unsigned int chunkSize) {
  ESF_ASSERT(transaction);
  ESF_ASSERT(chunk);
  ESF_ASSERT(0 < chunkSize);

  AWSHttpEchoServerContext *context =
      (AWSHttpEchoServerContext *)transaction->getApplicationContext();

  ESF_ASSERT(context);

  unsigned int totalBytesRemaining = BodySize - context->getBytesSent();
  unsigned int bytesToSend =
      chunkSize > totalBytesRemaining ? totalBytesRemaining : chunkSize;

  memcpy(chunk, BODY + context->getBytesSent(), bytesToSend);

  context->addBytesSent(bytesToSend);
}

void AWSHttpEchoServerHandler::end(AWSHttpTransaction *transaction,
                                   AWSHttpServerHandler::State state) {
  ESF_ASSERT(transaction);

  AWSHttpEchoServerContext *context =
      (AWSHttpEchoServerContext *)transaction->getApplicationContext();

  ESF_ASSERT(context);

  ESFAllocator *allocator = transaction->getAllocator();

  ESF_ASSERT(allocator);

  context->~AWSHttpEchoServerContext();
  allocator->deallocate(context);
  transaction->setApplicationContext(0);

  switch (state) {
    case AWS_HTTP_SERVER_HANDLER_BEGIN:

      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                     "[handler] Transaction failed at begin state");
      }

      break;

    case AWS_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS:

      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(
            ESFLogger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at request header parse state");
      }

      break;

    case AWS_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY:

      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(
            ESFLogger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at request body parse state");
      }

      break;

    case AWS_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS:

      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(
            ESFLogger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at response header send state");
      }

      break;

    case AWS_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY:

      if (_logger->isLoggable(ESFLogger::Warning)) {
        _logger->log(
            ESFLogger::Warning, __FILE__, __LINE__,
            "[handler] Transaction failed at response header send state");
      }

      break;

    case AWS_HTTP_SERVER_HANDLER_END:

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
}
