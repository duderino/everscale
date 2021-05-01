#ifndef ES_HTTP_REQUEST_PARSER_H
#include <ESHttpRequestParser.h>
#endif

#ifndef ES_HTTP_RESPONSE_PARSER_H
#include <ESHttpResponseParser.h>
#endif

#ifndef ES_HTTP_REQUEST_FORMATTER_H
#include <ESHttpRequestFormatter.h>
#endif

#ifndef ES_HTTP_RESPONSE_FORMATTER_H
#include <ESHttpResponseFormatter.h>
#endif

#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#ifndef ESB_RAND_H
#include <ESBRand.h>
#endif

#include <dirent.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <unistd.h>

namespace ES {

static ESB::SimpleFileLogger Logger(stdout, ESB::Logger::Warning);

class ParserFormatterTest : public ::testing::Test {
 public:
  ParserFormatterTest()
      : _inputBuffer(_inputBufferStorage, sizeof(_inputBufferStorage)),
        _outputBuffer(_outputBufferStorage, sizeof(_outputBufferStorage)),
        _requestParserBuffer(_requestBufferStorage, sizeof(_requestBufferStorage)),
        _responseParserBuffer(_responseBufferStorage, sizeof(_responseBufferStorage)),
        _allocator(4096, sizeof(ESB::UWord), 1, ESB::SystemAllocator::Instance(), true),
        _requestParser(&_requestParserBuffer, _allocator),
        _responseParser(&_responseParserBuffer, _allocator),
        _requestFormatter(),
        _responseFormatter(),
        _random(42) {}

  static void SetUpTestSuite() { ESB::Logger::SetInstance(&Logger); }

  virtual void SetUp() {
    _inputBuffer.clear();
    _outputBuffer.clear();
    _requestParserBuffer.clear();
    _responseParserBuffer.clear();
    _allocator.reset();
    _requestParser.reset();
    _responseParser.reset();
    _requestFormatter.reset();
    _responseFormatter.reset();
  }

 protected:
  ESB::Error parseRequest(const char *filename, HttpRequest &request, ESB::Buffer &body);
  ESB::Error parseResponse(const char *filename, HttpResponse &response, ESB::Buffer &body);
  ESB::Error formatRequest(const char *filename, const HttpRequest &request, ESB::Buffer &body);
  ESB::Error formatResponse(const char *filename, const HttpResponse &response, ESB::Buffer &body);
  void expectHeader(const HttpMessage &message, const char *fieldName, const char *fieldValue);
  int headerValueToInt(const HttpMessage &message, const char *fieldName);
  void init(const ::testing::TestInfo *info);

  ESB::Buffer _inputBuffer;
  ESB::Buffer _outputBuffer;
  ESB::Buffer _requestParserBuffer;
  ESB::Buffer _responseParserBuffer;
  ESB::DiscardAllocator _allocator;
  HttpRequestParser _requestParser;
  HttpResponseParser _responseParser;
  HttpRequestFormatter _requestFormatter;
  HttpResponseFormatter _responseFormatter;
  ESB::Rand _random;
  char _inputFile[ESB_MAX_FILENAME + 1];
  char _outputFile[ESB_MAX_FILENAME + 1];

 private:
  ESB::Error readOpen(const char *filename, int *fd);
  ESB::Error writeOpen(const char *filename, int *fd);
  ESB::Error parseHeaders(const char *filename, int fd, HttpMessageParser &parser, HttpMessage &message);
  ESB::Error parseBody(const char *filename, int fd, HttpMessageParser &parser, ESB::Buffer &body);
  ESB::Error formatHeaders(const char *filename, int fd, HttpMessageFormatter &formatter, const HttpMessage &message);
  ESB::Error formatBody(const char *filename, int fd, HttpMessageFormatter &formatter, ESB::Buffer &body);

  unsigned char _inputBufferStorage[4096];
  unsigned char _outputBufferStorage[4096];
  unsigned char _requestBufferStorage[4096];
  unsigned char _responseBufferStorage[4096];
};

TEST_F(ParserFormatterTest, AsteriskRequest) {
  init(test_info_);
  HttpRequest request;
  unsigned char storage[4096];
  ESB::Buffer body(storage, sizeof(storage));

  ESB::Error error = parseRequest(_inputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_ASTERISK, request.requestUri().type());
  EXPECT_EQ(0, strcmp("OPTIONS", (const char *)request.method()));
  expectHeader(request, "host", "vip-aws.back.den.p4pnet.net");
  expectHeader(request, "connection", "close");
  EXPECT_EQ(0, body.readable());

  error = formatRequest(_outputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);

  _requestParser.reset();
  error = parseRequest(_outputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_ASTERISK, request.requestUri().type());
  EXPECT_EQ(0, strcmp("OPTIONS", (const char *)request.method()));
  expectHeader(request, "host", "vip-aws.back.den.p4pnet.net");
  expectHeader(request, "connection", "close");
  EXPECT_EQ(0, body.readable());
}

TEST_F(ParserFormatterTest, ComplexRequestUri) {
  init(test_info_);
  HttpRequest request;
  unsigned char storage[4096];
  ESB::Buffer body(storage, sizeof(storage));

  ESB::Error error = parseRequest(_inputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("GET", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_HTTP, request.requestUri().type());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().host(), "vip-wsmt-cp.back.sc.p4pnet.net"));
  EXPECT_EQ(7005, request.requestUri().port());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().absPath(), "/Router/wsdl"));
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().query(),
                      "uri=urn%3Abts-Service%2FOverture%2FResearch%2F1&router=http://"
                      "vip-wsmt-cp.back.sc.p4pnet.net:7005/Router/dispatch"));
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().fragment(), "some%2Ffragment%2Fis%2Fhere"));
  expectHeader(request, "host", "vip-wsmt-cp.back.sc.p4pnet.net:7005");
  expectHeader(request, "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
  expectHeader(request, "User-Agent",
               "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.7.8) Gecko/20050512 Red Hat/1.0.4-1.4.1 Firefox/1.0.4");
  EXPECT_EQ(0, body.readable());

  error = formatRequest(_outputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);

  SetUp();
  request.reset();
  body.clear();
  error = parseRequest(_outputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("GET", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_HTTP, request.requestUri().type());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().host(), "vip-wsmt-cp.back.sc.p4pnet.net"));
  EXPECT_EQ(7005, request.requestUri().port());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().absPath(), "/Router/wsdl"));
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().query(),
                      "uri=urn%3Abts-Service%2FOverture%2FResearch%2F1&router=http://"
                      "vip-wsmt-cp.back.sc.p4pnet.net:7005/Router/dispatch"));
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().fragment(), "some%2Ffragment%2Fis%2Fhere"));
  expectHeader(request, "host", "vip-wsmt-cp.back.sc.p4pnet.net:7005");
  expectHeader(request, "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
  expectHeader(request, "User-Agent",
               "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.7.8) Gecko/20050512 Red Hat/1.0.4-1.4.1 Firefox/1.0.4");
  EXPECT_EQ(0, body.readable());
}

TEST_F(ParserFormatterTest, Http10) {
  init(test_info_);
  HttpRequest request;
  unsigned char storage1[4096];
  ESB::Buffer body1(storage1, sizeof(storage1));
  unsigned char storage2[4096];
  ESB::Buffer body2(storage2, sizeof(storage2));
  const int expectedContentLength = 522;

  ESB::Error error = parseRequest(_inputFile, request, body1);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("POST", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_HTTP, request.requestUri().type());
  EXPECT_EQ(NULL, request.requestUri().host());
  EXPECT_EQ(-1, request.requestUri().port());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().absPath(), "/autoreview/services/CheckService"));
  EXPECT_EQ(NULL, request.requestUri().query());
  EXPECT_EQ(NULL, request.requestUri().fragment());
  expectHeader(request, "host", "deveps-001.ysm.oc2.yahoo.com:8090");
  expectHeader(request, "Accept", "application/soap+xml, application/dime, multipart/related, text/*");
  EXPECT_EQ(expectedContentLength, headerValueToInt(request, "content-length"));
  EXPECT_EQ(expectedContentLength, body1.readable());

  error = formatRequest(_outputFile, request, body1);
  EXPECT_EQ(ESB_SUCCESS, error);

  SetUp();
  request.reset();
  error = parseRequest(_outputFile, request, body2);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("POST", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_HTTP, request.requestUri().type());
  EXPECT_EQ(NULL, request.requestUri().host());
  EXPECT_EQ(-1, request.requestUri().port());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().absPath(), "/autoreview/services/CheckService"));
  EXPECT_EQ(NULL, request.requestUri().query());
  EXPECT_EQ(NULL, request.requestUri().fragment());
  expectHeader(request, "host", "deveps-001.ysm.oc2.yahoo.com:8090");
  expectHeader(request, "Accept", "application/soap+xml, application/dime, multipart/related, text/*");
  EXPECT_EQ(expectedContentLength, headerValueToInt(request, "content-length"));
  EXPECT_EQ(expectedContentLength, body2.readable());

  EXPECT_EQ(0, memcmp(body1.buffer(), body2.buffer(), expectedContentLength));
}

TEST_F(ParserFormatterTest, ContentLength) {
  init(test_info_);
  HttpResponse response;
  unsigned char storage1[16 * 1024];
  ESB::Buffer body1(storage1, sizeof(storage1));
  unsigned char storage2[16 * 1024];
  ESB::Buffer body2(storage2, sizeof(storage2));
  const int expectedContentLength = 11830;

  ESB::Error error = parseResponse(_inputFile, response, body1);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(200, response.statusCode());
  EXPECT_EQ(0, strcmp("OK", (const char *)response.reasonPhrase()));
  EXPECT_EQ(110, response.httpVersion());
  expectHeader(response, "connection", "close");
  expectHeader(response, "date", "Wed, 12 Jul 2006 17:22:25 GMT");
  expectHeader(response, "content-type", "text/xml; charset=utf-8");
  EXPECT_EQ(expectedContentLength, headerValueToInt(response, "content-length"));
  EXPECT_EQ(expectedContentLength, body1.readable());

  error = formatResponse(_outputFile, response, body1);
  EXPECT_EQ(ESB_SUCCESS, error);

  SetUp();
  response.reset();
  error = parseResponse(_outputFile, response, body2);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(200, response.statusCode());
  EXPECT_EQ(0, strcmp("OK", (const char *)response.reasonPhrase()));
  EXPECT_EQ(110, response.httpVersion());
  expectHeader(response, "connection", "close");
  expectHeader(response, "date", "Wed, 12 Jul 2006 17:22:25 GMT");
  expectHeader(response, "content-type", "text/xml; charset=utf-8");
  EXPECT_EQ(expectedContentLength, headerValueToInt(response, "content-length"));
  EXPECT_EQ(expectedContentLength, body2.readable());

  EXPECT_EQ(0, memcmp(body1.buffer(), body2.buffer(), expectedContentLength));
}

TEST_F(ParserFormatterTest, HexEncoding) {
  init(test_info_);
  HttpRequest request;
  unsigned char storage[4096];
  ESB::Buffer body(storage, sizeof(storage));

  ESB::Error error = parseRequest(_inputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("GET", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_HTTP, request.requestUri().type());
  EXPECT_EQ(NULL, request.requestUri().host());
  EXPECT_EQ(-1, request.requestUri().port());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().absPath(), "/Research/V1"));
  expectHeader(request, "host", "vip-aws.back.den.p4pnet.net");
  expectHeader(
      request, "User-Agent",
      "%4dozilla/5.0%20(X11;%20U;%20Linux i686;%20en-US; rv:1.7.8)%0dGecko/20050512 Red Hat/1.0.4-1.4.1 Firefox/1.0.4");
  EXPECT_EQ(0, body.readable());

  error = formatRequest(_outputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);

  SetUp();
  request.reset();
  body.clear();
  error = parseRequest(_outputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("GET", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_HTTP, request.requestUri().type());
  EXPECT_EQ(NULL, request.requestUri().host());
  EXPECT_EQ(-1, request.requestUri().port());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().absPath(), "/Research/V1"));
  expectHeader(request, "host", "vip-aws.back.den.p4pnet.net");
  expectHeader(
      request, "User-Agent",
      "%4dozilla/5.0%20(X11;%20U;%20Linux i686;%20en-US; rv:1.7.8)%0dGecko/20050512 Red Hat/1.0.4-1.4.1 Firefox/1.0.4");
  EXPECT_EQ(0, body.readable());
}

TEST_F(ParserFormatterTest, MissingCarriageReturn) {
  init(test_info_);
  HttpResponse response;
  unsigned char storage1[1024];
  ESB::Buffer body1(storage1, sizeof(storage1));
  unsigned char storage2[1024];
  ESB::Buffer body2(storage2, sizeof(storage2));
  const int expectedContentLength = 0;

  ESB::Error error = parseResponse(_inputFile, response, body1);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(206, response.statusCode());
  EXPECT_EQ(0, strcmp("Partial content", (const char *)response.reasonPhrase()));
  EXPECT_EQ(110, response.httpVersion());
  expectHeader(response, "content-range", "bytes 21010-47021/47022");
  EXPECT_EQ(expectedContentLength, headerValueToInt(response, "content-length"));
  EXPECT_EQ(expectedContentLength, body1.readable());

  error = formatResponse(_outputFile, response, body1);
  EXPECT_EQ(ESB_SUCCESS, error);

  SetUp();
  response.reset();
  error = parseResponse(_outputFile, response, body2);
  EXPECT_EQ(206, response.statusCode());
  EXPECT_EQ(0, strcmp("Partial content", (const char *)response.reasonPhrase()));
  EXPECT_EQ(110, response.httpVersion());
  expectHeader(response, "content-range", "bytes 21010-47021/47022");
  EXPECT_EQ(expectedContentLength, headerValueToInt(response, "content-length"));
  EXPECT_EQ(expectedContentLength, body1.readable());

  EXPECT_EQ(0, memcmp(body1.buffer(), body2.buffer(), expectedContentLength));
}

TEST_F(ParserFormatterTest, NoBody) {
  init(test_info_);
  HttpRequest request;
  unsigned char storage[4096];
  ESB::Buffer body(storage, sizeof(storage));

  ESB::Error error = parseRequest(_inputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("GET", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_HTTP, request.requestUri().type());
  EXPECT_EQ(NULL, request.requestUri().host());
  EXPECT_EQ(-1, request.requestUri().port());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().absPath(), "/Research/V1"));
  expectHeader(request, "host", "vip-aws.back.den.p4pnet.net");
  EXPECT_EQ(0, body.readable());

  error = formatRequest(_outputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);

  SetUp();
  request.reset();
  body.clear();
  error = parseRequest(_outputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("GET", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_HTTP, request.requestUri().type());
  EXPECT_EQ(NULL, request.requestUri().host());
  EXPECT_EQ(-1, request.requestUri().port());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().absPath(), "/Research/V1"));
  expectHeader(request, "host", "vip-aws.back.den.p4pnet.net");
  EXPECT_EQ(0, body.readable());
}

TEST_F(ParserFormatterTest, SIP) {
  init(test_info_);
  HttpRequest request;
  unsigned char storage1[1024];
  ESB::Buffer body1(storage1, sizeof(storage1));
  unsigned char storage2[1024];
  ESB::Buffer body2(storage2, sizeof(storage2));
  const int expectedContentLength = 147;

  ESB::Error error = parseRequest(_inputFile, request, body1);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("INVITE", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_OTHER, request.requestUri().type());
  EXPECT_EQ(NULL, request.requestUri().host());
  EXPECT_EQ(-1, request.requestUri().port());
  EXPECT_EQ(NULL, request.requestUri().absPath());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().other(), "sip:vivekg@chair.dnrc.bell-labs.com"));
  expectHeader(request, "to", "sip:vivekg@chair.dnrc.bell-labs.com ; tag = 1a1b1f1H33n");
  expectHeader(request, "from", "\"J Rosenberg \\\\\\\"\" <sip:jdrosen@lucent.com> ; tag = 98asjd8");
  expectHeader(request, "NewFangledHeader", "newfangled value more newfangled value");
  expectHeader(request, "v", "SIP / 2.0 / TCP 12.3.4.5 ; branch = 9ikj8 , SIP / 2.0 / UDP 1.2.3.4 ; hidden");
  expectHeader(request, "m",
               "\"Quoted string \\\"\\\"\" <sip:jdrosen@bell-labs.com> ; newparam = newvalue ; secondparam = "
               "secondvalue ; q = 0.33 , tel:4443322");
  EXPECT_EQ(expectedContentLength, body1.readable());

  error = formatRequest(_outputFile, request, body1);
  EXPECT_EQ(ESB_SUCCESS, error);

  SetUp();
  request.reset();
  error = parseRequest(_outputFile, request, body2);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("INVITE", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_OTHER, request.requestUri().type());
  EXPECT_EQ(NULL, request.requestUri().host());
  EXPECT_EQ(-1, request.requestUri().port());
  EXPECT_EQ(NULL, request.requestUri().absPath());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().other(), "sip:vivekg@chair.dnrc.bell-labs.com"));
  expectHeader(request, "to", "sip:vivekg@chair.dnrc.bell-labs.com ; tag = 1a1b1f1H33n");
  expectHeader(request, "from", "\"J Rosenberg \\\\\\\"\" <sip:jdrosen@lucent.com> ; tag = 98asjd8");
  expectHeader(request, "NewFangledHeader", "newfangled value more newfangled value");
  expectHeader(request, "v", "SIP / 2.0 / TCP 12.3.4.5 ; branch = 9ikj8 , SIP / 2.0 / UDP 1.2.3.4 ; hidden");
  expectHeader(request, "m",
               "\"Quoted string \\\"\\\"\" <sip:jdrosen@bell-labs.com> ; newparam = newvalue ; secondparam = "
               "secondvalue ; q = 0.33 , tel:4443322");
  EXPECT_EQ(expectedContentLength, body2.readable());

  EXPECT_EQ(0, memcmp(body1.buffer(), body2.buffer(), expectedContentLength));
}

TEST_F(ParserFormatterTest, Soap) {
  init(test_info_);
  HttpRequest request;
  unsigned char storage1[1024];
  ESB::Buffer body1(storage1, sizeof(storage1));
  unsigned char storage2[1024];
  ESB::Buffer body2(storage2, sizeof(storage2));
  const int expectedContentLength = 764;

  ESB::Error error = parseRequest(_inputFile, request, body1);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("POST", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_HTTP, request.requestUri().type());
  EXPECT_EQ(NULL, request.requestUri().host());
  EXPECT_EQ(-1, request.requestUri().port());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().absPath(), "/Echo/V1/EchoOperation"));
  EXPECT_EQ(NULL, request.requestUri().query());
  EXPECT_EQ(NULL, request.requestUri().fragment());
  expectHeader(request, "host", "vip-aws.back.den.p4pnet.net");
  expectHeader(request, "SOAPAction", "\"urn:yahoo:overture:aws:echo:Echo\"");
  EXPECT_EQ(expectedContentLength, headerValueToInt(request, "content-length"));
  EXPECT_EQ(expectedContentLength, body1.readable());

  error = formatRequest(_outputFile, request, body1);
  EXPECT_EQ(ESB_SUCCESS, error);

  SetUp();
  request.reset();
  error = parseRequest(_outputFile, request, body2);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("POST", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_HTTP, request.requestUri().type());
  EXPECT_EQ(NULL, request.requestUri().host());
  EXPECT_EQ(-1, request.requestUri().port());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().absPath(), "/Echo/V1/EchoOperation"));
  EXPECT_EQ(NULL, request.requestUri().query());
  EXPECT_EQ(NULL, request.requestUri().fragment());
  expectHeader(request, "host", "vip-aws.back.den.p4pnet.net");
  expectHeader(request, "SOAPAction", "\"urn:yahoo:overture:aws:echo:Echo\"");
  EXPECT_EQ(expectedContentLength, headerValueToInt(request, "content-length"));
  EXPECT_EQ(expectedContentLength, body2.readable());

  EXPECT_EQ(0, memcmp(body1.buffer(), body2.buffer(), expectedContentLength));
}

TEST_F(ParserFormatterTest, TinyChunks) {
  init(test_info_);
  HttpResponse response;
  unsigned char storage1[1024];
  ESB::Buffer body1(storage1, sizeof(storage1));
  unsigned char storage2[1024];
  ESB::Buffer body2(storage2, sizeof(storage2));
  const int expectedContentLength = 767;

  ESB::Error error = parseResponse(_inputFile, response, body1);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(200, response.statusCode());
  EXPECT_EQ(0, strcmp("OK", (const char *)response.reasonPhrase()));
  EXPECT_EQ(110, response.httpVersion());
  expectHeader(response, "transfer-encoding", "chunked");
  EXPECT_EQ(expectedContentLength, body1.readable());

  error = formatResponse(_outputFile, response, body1);
  EXPECT_EQ(ESB_SUCCESS, error);

  SetUp();
  response.reset();
  error = parseResponse(_outputFile, response, body2);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(200, response.statusCode());
  EXPECT_EQ(0, strcmp("OK", (const char *)response.reasonPhrase()));
  EXPECT_EQ(110, response.httpVersion());
  expectHeader(response, "transfer-encoding", "chunked");
  EXPECT_EQ(expectedContentLength, body2.readable());

  EXPECT_EQ(0, memcmp(body1.buffer(), body2.buffer(), expectedContentLength));
}

TEST_F(ParserFormatterTest, Tomcat) {
  init(test_info_);
  HttpResponse response;
  unsigned char storage1[1024];
  ESB::Buffer body1(storage1, sizeof(storage1));
  unsigned char storage2[1024];
  ESB::Buffer body2(storage2, sizeof(storage2));
  // ALERT: tomcat doesn't include a content-length or transfer encoding, so to the parser it seems to not have a body
  // The proxy impl needs to instead note the Connection: close header and treat the rest of the stream as the body
  const int expectedContentLength = 0;

  ESB::Error error = parseResponse(_inputFile, response, body1);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(200, response.statusCode());
  EXPECT_EQ(0, strcmp("OK", (const char *)response.reasonPhrase()));
  EXPECT_EQ(110, response.httpVersion());
  expectHeader(response, "server", "Apache-Coyote/1.1");
  EXPECT_EQ(expectedContentLength, body1.readable());

  error = formatResponse(_outputFile, response, body1);
  EXPECT_EQ(ESB_SUCCESS, error);

  SetUp();
  response.reset();
  error = parseResponse(_outputFile, response, body2);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(200, response.statusCode());
  EXPECT_EQ(0, strcmp("OK", (const char *)response.reasonPhrase()));
  EXPECT_EQ(110, response.httpVersion());
  expectHeader(response, "server", "Apache-Coyote/1.1");
  EXPECT_EQ(expectedContentLength, body2.readable());

  EXPECT_EQ(0, memcmp(body1.buffer(), body2.buffer(), expectedContentLength));
}

TEST_F(ParserFormatterTest, Whitespace) {
  init(test_info_);
  HttpRequest request;
  unsigned char storage[4096];
  ESB::Buffer body(storage, sizeof(storage));

  ESB::Error error = parseRequest(_inputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("GET", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_HTTP, request.requestUri().type());
  EXPECT_EQ(NULL, request.requestUri().host());
  EXPECT_EQ(-1, request.requestUri().port());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().absPath(), "/%52esearch/V1"));
  expectHeader(request, "host", "vip-aws.back.den.p4pnet.net");
  expectHeader(request, "user-agent",
               "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.7.8) Gecko/20050512 Red Hat/1.0.4-1.4.1 Firefox/1.0.4");
  expectHeader(request, "accept",
               "text/xml,application/xml,application/xhtml+xml, text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
  EXPECT_EQ(0, body.readable());

  error = formatRequest(_outputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);

  SetUp();
  request.reset();
  body.clear();
  error = parseRequest(_outputFile, request, body);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(0, strcmp("GET", (const char *)request.method()));
  EXPECT_EQ(ES::HttpRequestUri::ES_URI_HTTP, request.requestUri().type());
  EXPECT_EQ(NULL, request.requestUri().host());
  EXPECT_EQ(-1, request.requestUri().port());
  EXPECT_EQ(0, strcmp((const char *)request.requestUri().absPath(), "/%52esearch/V1"));
  expectHeader(request, "host", "vip-aws.back.den.p4pnet.net");
  expectHeader(request, "user-agent",
               "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.7.8) Gecko/20050512 Red Hat/1.0.4-1.4.1 Firefox/1.0.4");
  expectHeader(request, "accept",
               "text/xml,application/xml,application/xhtml+xml, text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
  EXPECT_EQ(0, body.readable());
}

ESB::Error ParserFormatterTest::parseRequest(const char *filename, HttpRequest &request, ESB::Buffer &body) {
  int fd = -1;
  ESB::Error error = readOpen(filename, &fd);
  if (ESB_SUCCESS != error) {
    return error;
  }

  error = parseHeaders(filename, fd, _requestParser, request);
  if (ESB_SUCCESS != error) {
    close(fd);
    return error;
  }

  if (ESB_DEBUG_LOGGABLE) {
    ESB_LOG_DEBUG("headers parsed for file %s", filename);
    ESB_LOG_DEBUG("Method: %s", request.method());
    ESB_LOG_DEBUG("RequestUri");

    switch (request.requestUri().type()) {
      case HttpRequestUri::ES_URI_ASTERISK:
        ESB_LOG_DEBUG("  Asterisk");
        break;
      case HttpRequestUri::ES_URI_HTTP:
      case HttpRequestUri::ES_URI_HTTPS:
        ESB_LOG_DEBUG("  Scheme: %s", HttpRequestUri::ES_URI_HTTP == request.requestUri().type() ? "http" : "https");
        ESB_LOG_DEBUG("  Host: %s", !request.requestUri().host() ? "none" : (const char *)request.requestUri().host());
        ESB_LOG_DEBUG("  Port: %d", request.requestUri().port());
        ESB_LOG_DEBUG("  AbsPath: %s", request.requestUri().absPath());
        ESB_LOG_DEBUG("  Query: %s",
                      !request.requestUri().query() ? "none" : (const char *)request.requestUri().query());
        ESB_LOG_DEBUG("  Fragment: %s",
                      !request.requestUri().fragment() ? "none" : (const char *)request.requestUri().fragment());
        break;

      case HttpRequestUri::ES_URI_OTHER:
        ESB_LOG_DEBUG("  Other: %s", request.requestUri().other());
        break;
    }

    ESB_LOG_DEBUG("Version: HTTP/%d.%d", request.httpVersion() / 100, request.httpVersion() % 100 / 10);
    ESB_LOG_DEBUG("Headers");
    for (HttpHeader *header = (HttpHeader *)request.headers().first(); header; header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("   %s: %s\n", (const char *)header->fieldName(),
                    !header->fieldValue() ? "null" : (const char *)header->fieldValue());
    }
  }

  error = parseBody(filename, fd, _requestParser, body);
  close(fd);
  return error;
}

ESB::Error ParserFormatterTest::parseResponse(const char *filename, HttpResponse &response, ESB::Buffer &body) {
  int fd = -1;
  ESB::Error error = readOpen(filename, &fd);
  if (ESB_SUCCESS != error) {
    return error;
  }

  error = parseHeaders(filename, fd, _responseParser, response);
  if (ESB_SUCCESS != error) {
    close(fd);
    return error;
  }

  if (ESB_DEBUG_LOGGABLE) {
    ESB_LOG_DEBUG("headers parsed for file %s", filename);
    ESB_LOG_DEBUG("StatusCode: %d", response.statusCode());
    ESB_LOG_DEBUG("ReasonPhrase: %s", response.reasonPhrase());
    ESB_LOG_DEBUG("Version: HTTP/%d.%d", response.httpVersion() / 100, response.httpVersion() % 100 / 10);
    ESB_LOG_DEBUG("Headers");
    for (HttpHeader *header = (HttpHeader *)response.headers().first(); header; header = (HttpHeader *)header->next()) {
      ESB_LOG_DEBUG("   %s: %s", (const char *)header->fieldName(),
                    !header->fieldValue() ? "null" : (const char *)header->fieldValue());
    }
  }

  error = parseBody(filename, fd, _responseParser, body);
  close(fd);
  return error;
}

ESB::Error ParserFormatterTest::formatRequest(const char *filename, const HttpRequest &request, ESB::Buffer &body) {
  int fd = -1;
  ESB::Error error = writeOpen(filename, &fd);
  if (ESB_SUCCESS != error) {
    return error;
  }

  error = formatHeaders(filename, fd, _requestFormatter, request);
  if (ESB_SUCCESS != error) {
    close(fd);
    return error;
  }

  error = formatBody(filename, fd, _requestFormatter, body);
  close(fd);
  return error;
}

ESB::Error ParserFormatterTest::formatResponse(const char *filename, const HttpResponse &response, ESB::Buffer &body) {
  int fd = -1;
  ESB::Error error = writeOpen(filename, &fd);
  if (ESB_SUCCESS != error) {
    return error;
  }

  error = formatHeaders(filename, fd, _responseFormatter, response);
  if (ESB_SUCCESS != error) {
    close(fd);
    return error;
  }

  error = formatBody(filename, fd, _responseFormatter, body);
  close(fd);
  return error;
}

ESB::Error ParserFormatterTest::readOpen(const char *filename, int *fd) {
  *fd = open(filename, O_RDONLY);

  if (0 > *fd) {
    ESB::Error error = ESB::LastError();
    ESB_LOG_ERROR_ERRNO(error, "cannot open %s", filename);
    return error;
  }

  return ESB_SUCCESS;
}

ESB::Error ParserFormatterTest::writeOpen(const char *filename, int *fd) {
  *fd = open(filename, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);

  if (0 > *fd) {
    ESB::Error error = ESB::LastError();
    ESB_LOG_ERROR_ERRNO(error, "cannot open %s", filename);
    return error;
  }

  return ESB_SUCCESS;
}

ESB::Error ParserFormatterTest::parseHeaders(const char *filename, int fd, HttpMessageParser &parser,
                                             HttpMessage &message) {
  while (true) {
    ESB::Error error = parser.parseHeaders(&_inputBuffer, message);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("need more data from file %s", filename);

      if (!_inputBuffer.isWritable()) {
        ESB_LOG_DEBUG("compacting input buffer for %s", filename);

        if (!_inputBuffer.compact()) {
          ESB_LOG_ERROR("cannot parse %s: parser jammed\n", filename);
          return ESB_OVERFLOW;
        }
      }

      ESB::SSize result =
          read(fd, _inputBuffer.buffer() + _inputBuffer.writePosition(), _random.generate(1, _inputBuffer.writable()));

      if (0 > result) {
        error = ESB::LastError();
        ESB_LOG_ERROR_ERRNO(error, "cannot read %s", filename);
        return error;
      }

      if (0 == result) {
        ESB_LOG_ERROR("unexpected EOF reading %s", filename);
        return ESB_CLOSED;
      }

      _inputBuffer.setWritePosition(_inputBuffer.writePosition() + result);

      ESB_LOG_DEBUG("read %ld bytes from file %s", result, filename);
      continue;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "cannot parse %s", filename);
      return error;
    }

    return ESB_SUCCESS;
  }
}

ESB::Error ParserFormatterTest::parseBody(const char *filename, int fd, HttpMessageParser &parser, ESB::Buffer &body) {
  while (true) {
    ESB::UInt64 bufferOffset = 0;
    ESB::UInt64 chunkSize = 0;
    ESB::Error error = parser.parseBody(&_inputBuffer, &bufferOffset, &chunkSize);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("need more data from file %s", filename);

      if (!_inputBuffer.isWritable()) {
        ESB_LOG_DEBUG("compacting input buffer for %s", filename);

        if (!_inputBuffer.compact()) {
          ESB_LOG_ERROR("cannot parse %s: parser jammed", filename);
          return ESB_OVERFLOW;
        }
      }

      ESB::Size result =
          read(fd, _inputBuffer.buffer() + _inputBuffer.writePosition(), _random.generate(1, _inputBuffer.writable()));

      if (0 > result) {
        error = ESB::LastError();
        ESB_LOG_ERROR_ERRNO(error, "cannot read %s", filename);
        return error;
      }

      if (0 == result) {
        ESB_LOG_DEBUG("EOF reading body from %s", filename);
        return ESB_SUCCESS;
      }

      _inputBuffer.setWritePosition(_inputBuffer.writePosition() + result);
      ESB_LOG_DEBUG("read %ld bytes from file %s", result, filename);
      continue;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "cannot parse %s", filename);
      return error;
    }

    if (0 == chunkSize) {
      ESB_LOG_DEBUG("finished reading body %s", filename);
      return ESB_SUCCESS;
    }

    if (body.writable() < chunkSize) {
      ESB_LOG_ERROR_ERRNO(ESB_OVERFLOW, "body buffer too small for %s", filename);
      return ESB_OVERFLOW;
    }

    memcpy(body.buffer() + body.writePosition(), _inputBuffer.buffer() + bufferOffset, chunkSize);
    body.setWritePosition(body.writePosition() + chunkSize);
    parser.consumeBody(&_inputBuffer, chunkSize);
  }
}

ESB::Error ParserFormatterTest::formatHeaders(const char *filename, int fd, HttpMessageFormatter &formatter,
                                              const HttpMessage &message) {
  while (true) {
    ESB::Error error = formatter.formatHeaders(&_outputBuffer, message);

    if (ESB_AGAIN == error) {
      ESB_LOG_DEBUG("flushing output buffer for %s", filename);

      if (!_outputBuffer.isReadable()) {
        ESB_LOG_ERROR("cannot format %s: formatter jammed", filename);
        return ESB_OVERFLOW;
      }

      while (_outputBuffer.isReadable()) {
        ESB::SSize result = write(fd, _outputBuffer.buffer() + _outputBuffer.readPosition(), _outputBuffer.readable());

        if (0 > result) {
          error = ESB::LastError();
          ESB_LOG_ERROR_ERRNO(error, "cannot write to %s", filename);
          return error;
        }

        _outputBuffer.setReadPosition(_outputBuffer.readPosition() + result);
        _outputBuffer.compact();
      }

      continue;
    }

    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "cannot format to %s", filename);
      return error;
    }

    break;
  }

  // flush any header data left in the output buffer

  ESB_LOG_DEBUG("flushing headers to %s", filename);

  while (_outputBuffer.isReadable()) {
    ESB::SSize result = write(fd, _outputBuffer.buffer() + _outputBuffer.readPosition(), _outputBuffer.readable());

    if (0 > result) {
      ESB::Error error = ESB::LastError();
      ESB_LOG_ERROR_ERRNO(error, "cannot write to %s", filename);
      return error;
    }

    _outputBuffer.setReadPosition(_outputBuffer.readPosition() + result);
    _outputBuffer.compact();
  }

  return ESB_SUCCESS;
}

ESB::Error ParserFormatterTest::formatBody(const char *filename, int fd, HttpMessageFormatter &formatter,
                                           ESB::Buffer &body) {
  while (body.isReadable()) {
    ESB::UInt64 chunkSize = _random.generate(1, body.readable());
    ESB::UInt64 availableSize = 0U;

    // Begin chunk
    while (true) {
      ESB::Error error = formatter.beginBlock(&_outputBuffer, chunkSize, &availableSize);

      if (ESB_AGAIN == error) {
        ESB_LOG_DEBUG("flushing output buffer for %s", filename);

        if (!_outputBuffer.isReadable()) {
          ESB_LOG_ERROR("cannot format %s: formatter jammed", filename);
          return ESB_OVERFLOW;
        }

        while (_outputBuffer.isReadable()) {
          ESB::SSize result =
              write(fd, _outputBuffer.buffer() + _outputBuffer.readPosition(), _outputBuffer.readable());

          if (0 > result) {
            error = ESB::LastError();
            ESB_LOG_ERROR_ERRNO(error, "cannot write %s", filename);
            return error;
          }

          _outputBuffer.setReadPosition(_outputBuffer.readPosition() + result);
          _outputBuffer.compact();
        }

        continue;
      }

      if (ESB_SUCCESS != error) {
        ESB_LOG_ERROR_ERRNO(error, "error formatting %s", filename);
        return error;
      }

      break;
    }

    // Write chunk
    memcpy(_outputBuffer.buffer() + _outputBuffer.writePosition(), body.buffer() + body.readPosition(), availableSize);
    _outputBuffer.setWritePosition(_outputBuffer.writePosition() + availableSize);
    body.setReadPosition(body.readPosition() + availableSize);

    // End chunk
    while (true) {
      ESB::Error error = formatter.endBlock(&_outputBuffer);

      if (ESB_AGAIN == error) {
        ESB_LOG_DEBUG("flushing output buffer for %s", filename);

        if (!_outputBuffer.isReadable()) {
          ESB_LOG_ERROR("cannot format %s: formatter jammed", filename);
          return ESB_OVERFLOW;
        }

        while (_outputBuffer.isReadable()) {
          ESB::SSize result =
              write(fd, _outputBuffer.buffer() + _outputBuffer.readPosition(), _outputBuffer.readable());

          if (0 > result) {
            error = ESB::LastError();
            ESB_LOG_ERROR_ERRNO(error, "error writing %s", filename);
            return error;
          }

          _outputBuffer.setReadPosition(_outputBuffer.readPosition() + result);
          _outputBuffer.compact();
        }

        continue;
      }

      if (ESB_SUCCESS != error) {
        ESB_LOG_ERROR_ERRNO(error, "error formatting %s", filename);
        return error;
      }

      break;
    }
  }

  // flush any body data left in the output buffer

  ESB_LOG_DEBUG("flushing body to %s", filename);

  while (_outputBuffer.isReadable()) {
    ESB::SSize result = write(fd, _outputBuffer.buffer() + _outputBuffer.readPosition(), _outputBuffer.readable());

    if (0 > result) {
      ESB::Error error = ESB::LastError();
      ESB_LOG_ERROR_ERRNO(error, "cannot write to %s", filename);
      return error;
    }

    _outputBuffer.setReadPosition(_outputBuffer.readPosition() + result);
    _outputBuffer.compact();
  }

  return ESB_SUCCESS;
}

void ParserFormatterTest::expectHeader(const HttpMessage &message, const char *fieldName, const char *fieldValue) {
  const HttpHeader *header = message.findHeader(fieldName);
  EXPECT_TRUE(header);
  EXPECT_EQ(0, strcasecmp((const char *)header->fieldName(), fieldName));
  EXPECT_EQ(0, strcmp((const char *)header->fieldValue(), fieldValue));
}

int ParserFormatterTest::headerValueToInt(const HttpMessage &message, const char *fieldName) {
  const HttpHeader *header = message.findHeader(fieldName);
  if (!header) {
    return -1;
  }

  int value = 0;
  const char *p = (const char *)header->fieldValue();
  int length = strlen(p);

  for (; *p; ++p, --length) {
    int digit = *p - '0';
    for (int i = 0; i < length - 1; ++i) {
      digit *= 10;
    }
    value += digit;
  }

  return value;
}

void ParserFormatterTest::init(const ::testing::TestInfo *info) {
  snprintf(_inputFile, sizeof(_inputBuffer), "Test%s.in", info->name());
  snprintf(_outputFile, sizeof(_outputFile), "Test%s.out", info->name());
}

}  // namespace ES