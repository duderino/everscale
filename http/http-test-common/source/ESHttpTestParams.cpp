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
      _clientThreads(1),
      _originThreads(1),
      _proxyThreads(1),
      _connections(1),
      _iterations(1),
      _requestSize(1024),
      _responseSize(1024),
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

const unsigned char *HttpTestParams::requestBody() {
  if (!_requestBody) {
    _requestBody = (const unsigned char *)malloc(_requestSize);
    memset((void *)_requestBody, 'a', _requestSize);
  }
  return _requestBody;
}

const unsigned char *HttpTestParams::responseBody() {
  if (!_responseBody) {
    _responseBody = (const unsigned char *)malloc(_responseSize);
    memset((void *)_responseBody, 'b', _responseSize);
  }
  return _responseBody;
}

HttpTestParams &HttpTestParams::requestSize(ESB::UInt32 requestSize) {
  if (_requestBody) {
    free((void *)_requestBody);
    _requestBody = NULL;
  }
  _requestSize = requestSize;
  return *this;
}

HttpTestParams &HttpTestParams::responseSize(ESB::UInt32 responseSize) {
  if (_responseBody) {
    free((void *)_responseBody);
    _responseBody = NULL;
  }
  _responseSize = responseSize;
  return *this;
}

static void printUsage(const char *progName) {
  fprintf(stderr, "Usage: %s <options>\n", progName);
  fprintf(stderr, "\n");
  fprintf(stderr, "\tOptions:\n");
  fprintf(stderr,
          "\t-l <logLevel>     Defaults to 7 / INFO.  See below for other "
          "levels.\n");
  fprintf(stderr, "\t-c <clientThreads> Defaults to 1.\n");
  fprintf(stderr, "\t-o <originThreads> Defaults to 1.\n");
  fprintf(stderr, "\t-c <proxyThreads>  Defaults to 1.\n");
  fprintf(stderr, "\t-c <clientThreads> Defaults to 1.\n");
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
    int result = getopt(argc, argv, "l:c:o:p:s:i:r:");

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
            ESB_LOG_WARNING_ERRNO(ESB_INVALID_ARGUMENT, "unknown log level %d", v);
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

  return ESB_SUCCESS;
}

}  // namespace ES
