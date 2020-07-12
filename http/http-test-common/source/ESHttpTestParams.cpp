#ifndef ES_HTTP_TEST_PARAMS_H
#include "ESHttpTestParams.h"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace ES {

HttpTestParams::HttpTestParams()
    : _port(8080),
      _clientThreads(0),
      _originThreads(0),
      _proxyThreads(0),
      _connections(0),
      _iterations(0),
      _requestSize(0),
      _responseSize(0),
      _reuseConnections(true),
      _logLevel(ESB::Logger::Notice),
      _destinationAddress("127.0.0.1"),
      _hostHeader("localhost.localdomain"),
      _method("GET"),
      _contentType("octet-stream"),
      _absPath("/"),
      _requestBody(NULL),
      _responseBody(NULL) {}

HttpTestParams::~HttpTestParams() {
  if (_requestBody) {
    free((void *)_requestBody);
    _requestBody = NULL;
  }
  if (_responseBody) {
    free((void *)_responseBody);
    _responseBody = NULL;
  }
}

HttpTestParams &HttpTestParams::requestSize(ESB::UInt32 requestSize) {
  if (_requestBody) {
    free((void *)_requestBody);
  }

  _requestBody = (const unsigned char *)malloc(requestSize);
  memset((void *)_requestBody, 'a', requestSize);
  _requestSize = requestSize;
  return *this;
}

HttpTestParams &HttpTestParams::responseSize(ESB::UInt32 responseSize) {
  if (_responseBody) {
    free((void *)_responseBody);
  }

  _responseBody = (const unsigned char *)malloc(responseSize);
  memset((void *)_responseBody, 'a', responseSize);
  _responseSize = responseSize;
  return *this;
}

static void printUsage(const char *progName) {
  fprintf(stderr, "Usage: %s <options>\n", progName);
  fprintf(stderr, "\n");
  fprintf(stderr, "\tOptions:\n");
  fprintf(stderr, "\t-l <logLevel>      Defaults to 7 (INFO)\n");
  fprintf(stderr, "\t-c <clientThreads> Defaults to 1.\n");
  fprintf(stderr, "\t-o <originThreads> Defaults to 1.\n");
  fprintf(stderr, "\t-p <proxyThreads>  Defaults to 1.\n");
  fprintf(stderr, "\t-s <connections>   Defaults to 1 connection\n");
  fprintf(stderr, "\t-i <iterations>    Defaults to 1 request per connection\n");
  fprintf(stderr, "\t-r <reuse 1 or 0>  Defaults to 1 (reuse connections)\n");
  fprintf(stderr, "\t-t <dest port>     Defaults to 80\n");
  fprintf(stderr, "\n");
  fprintf(stderr,
          "\tLog Levels:\n"
          "\tNone = 0,         All logging disabled.\n"
          "\tEmergency = 1,    System-wide non-recoverable error.\n"
          "\tAlert = 2,        System-wide non-recoverable error imminent.\n"
          "\tCritical = 3,     System-wide potentially recoverable error.\n"
          "\tError = 4,        Localized non-recoverable error.\n"
          "\tWarning = 5,      Localized potentially recoverable error.\n"
          "\tNotice = 6,       Important non-error event.\n"
          "\tInfo = 7,         Non-error event.\n"
          "\tDebug = 8         Debugging event.\n");
}

ESB::Error HttpTestParams::override(int argc, char **argv) {
  while (true) {
    int result = getopt(argc, argv, "l:c:o:p:s:i:r:t:");

    if (0 > result) {
      break;
    }

    switch (result) {
      case 'l':
        switch (int v = atoi(optarg)) {
          case ESB::Logger::Emergency:
          case ESB::Logger::Alert:
          case ESB::Logger::Critical:
          case ESB::Logger::Err:
          case ESB::Logger::Warning:
          case ESB::Logger::Notice:
          case ESB::Logger::Info:
          case ESB::Logger::Debug:
            _logLevel = (ESB::Logger::Severity)v;
            break;
          default:
            printUsage(argv[0]);
            return ESB_INVALID_ARGUMENT;
        }
        break;
      case 'c':
        _clientThreads = atoi(optarg);
        break;
      case 'o':
        _originThreads = atoi(optarg);
        break;
      case 'p':
        _proxyThreads = atoi(optarg);
        break;
      case 's':
        _connections = atoi(optarg);
        break;
      case 'i':
        _iterations = atoi(optarg);
        break;
      case 'r':
        _reuseConnections = 0 != atoi(optarg);
        break;
      case 't':
        _port = atoi(optarg);
        break;
      case 'h':
        printUsage(argv[0]);
        exit(0);
      default:
        printUsage(argv[0]);
        return ESB_INVALID_ARGUMENT;
    }
  }

  if (0 < _clientThreads && _connections % _clientThreads) {
    _connections = _connections / _clientThreads * _clientThreads + _clientThreads;
  }

  dump();
  return ESB_SUCCESS;
}

void HttpTestParams::dump() {
  ESB::Logger::Instance().setSeverity(logLevel());
  ESB_LOG_NOTICE(
      "[params] clientThreads=%u, originThreads=%u, proxyThreads=%u, connections=%u, iterations=%u, requestSize=%u, "
      "responseSize=%u, reuseConnections=%d, destination=%s",
      _clientThreads, _originThreads, _proxyThreads, _connections, _iterations, _requestSize, _responseSize,
      _reuseConnections, _destinationAddress);
}

}  // namespace ES
