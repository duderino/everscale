#ifndef ES_HTTP_TEST_PARAMS_H
#include "ESHttpTestParams.h"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

namespace ES {

#define BASE_TEST_DIR "../../../base/tests/"
#define CA_PATH BASE_TEST_DIR "ca.crt"
#define CERT_PATH BASE_TEST_DIR "server.crt"
#define KEY_PATH BASE_TEST_DIR "server.key"

HttpTestParams::HttpTestParams()
    : _port(0),
      _clientThreads(1),
      _originThreads(1),
      _proxyThreads(1),
      _connections(1),
      _requestsPerConnection(1),
      _requestSize(0),
      _responseSize(0),
      _clientTimeoutMsec(1000),
      _proxyTimeoutMsec(1000),
      _originTimeoutMsec(1000),
      _useContentLengthHeader(false),
      _reuseConnections(true),
      _secure(false),
      _logLevel(ESB::Logger::Notice),
      _destinationAddress("127.0.0.1"),
      _hostHeader("localhost.localdomain"),
      _method("GET"),
      _contentType("octet-stream"),
      _absPath("/"),
      _requestBody(NULL),
      _responseBody(NULL),
      _maxVerifyDepth(3),
      _disruptTransaction(HAPPY_PATH) {
  caPath(CA_PATH);
  serverKeyPath(KEY_PATH);
  serverCertPath(CERT_PATH);
}

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

  _requestBody = (unsigned char *)malloc(requestSize);
  for (int i = 0; i < requestSize; ++i) {
    _requestBody[i] = 'a' + i % 26;
  }
  _requestSize = requestSize;
  return *this;
}

HttpTestParams &HttpTestParams::responseSize(ESB::UInt32 responseSize) {
  if (_responseBody) {
    free((void *)_responseBody);
  }

  _responseBody = (unsigned char *)malloc(responseSize);
  for (int i = 0; i < responseSize; ++i) {
    _responseBody[i] = 'A' + i % 26;
  }
  _responseSize = responseSize;
  return *this;
}

static void printUsage(const char *progName) {
  fprintf(stderr, "Usage: %s <options>\n", progName);
  fprintf(stderr, "\n");
  fprintf(stderr, "\tOptions:\n");
  fprintf(stderr, "\t--clientThreads <number, default 1>\n");
  fprintf(stderr, "\t--originThreads <number, default 1>\n");
  fprintf(stderr, "\t--proxyThreads <number, default 1>\n");
  fprintf(stderr, "\t--connections <number, default 1>\n");
  fprintf(stderr, "\t--requestsPerConnection <number, default 1>\n");
  fprintf(stderr, "\t--port <number, default 0/pick free ephemeral>\n");
  fprintf(stderr, "\t--reuseConnections <number, default to 1/reuse>\n");
  fprintf(stderr, "\t--secure <number, default to 0/insecure>\n");
  fprintf(stderr, "\t--caCertPath <path, defaults to " CA_PATH ">\n");
  fprintf(stderr, "\t--serverKeyPath <path, defaults to " KEY_PATH ">\n");
  fprintf(stderr, "\t--serverCertPath <path, defaults to " CERT_PATH ">\n");
  fprintf(stderr, "\t--logError\n");
  fprintf(stderr, "\t--logWarning\n");
  fprintf(stderr, "\t--logInfo\n");
  fprintf(stderr, "\t--logDebug\n");
  fprintf(stderr, "\t--logLevel <Log Level, defults to 6/NOTICE>\n");
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
#ifndef HAVE_GETOPT_LONG
#error "getopt_long() or equivalent is required"
#endif

  while (true) {
    int idx = 0;
    static struct option options[] = {{"logLevel", required_argument, NULL, 'l'},
                                      {"clientThreads", required_argument, NULL, 'c'},
                                      {"originThreads", required_argument, NULL, 'o'},
                                      {"proxyThreads", required_argument, NULL, 'p'},
                                      {"connections", required_argument, NULL, 's'},
                                      {"requestsPerConnection", required_argument, NULL, 'i'},
                                      {"reuseConnections", required_argument, NULL, 'r'},
                                      {"port", required_argument, NULL, 't'},
                                      {"secure", required_argument, NULL, 0},
                                      {"caCertPath", required_argument, NULL, 0},
                                      {"serverKeyPath", required_argument, NULL, 0},
                                      {"serverCertPath", required_argument, NULL, 0},
                                      {"logError", no_argument, NULL, 0},
                                      {"logWarning", no_argument, NULL, 0},
                                      {"logInfo", no_argument, NULL, 0},
                                      {"logDebug", no_argument, NULL, 0},
                                      {"help", required_argument, NULL, 'h'},
                                      {NULL, 0, NULL, 0}};

    int result = getopt_long(argc, argv, "l:c:o:p:s:i:r:t:", options, &idx);
    if (0 > result) {
      break;
    }

    switch (result) {
      case 0:
        if (0 == strcasecmp("secure", options[idx].name)) {
          secure(0 != atoi(optarg));
        } else if (0 == strcasecmp("caCertPath", options[idx].name)) {
          caPath(optarg);
        } else if (0 == strcasecmp("serverKeyPath", options[idx].name)) {
          serverKeyPath(optarg);
        } else if (0 == strcasecmp("serverCertPath", options[idx].name)) {
          serverCertPath(optarg);
        } else if (0 == strcasecmp("logError", options[idx].name)) {
          logLevel(ESB::Logger::Err);
        } else if (0 == strcasecmp("logWarning", options[idx].name)) {
          logLevel(ESB::Logger::Warning);
        } else if (0 == strcasecmp("logInfo", options[idx].name)) {
          logLevel(ESB::Logger::Info);
        } else if (0 == strcasecmp("logDebug", options[idx].name)) {
          logLevel(ESB::Logger::Debug);
        } else {
          printUsage(argv[0]);
          return ESB_INVALID_ARGUMENT;
        }
        break;

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
            logLevel((ESB::Logger::Severity)v);
            break;
          default:
            printUsage(argv[0]);
            return ESB_INVALID_ARGUMENT;
        }
        break;
      case 'c':
        clientThreads(atoi(optarg));
        break;
      case 'o':
        originThreads(atoi(optarg));
        break;
      case 'p':
        proxyThreads(atoi(optarg));
        break;
      case 's':
        connections(atoi(optarg));
        break;
      case 'i':
        requestsPerConnection(atoi(optarg));
        break;
      case 'r':
        reuseConnections(0 != atoi(optarg));
        break;
      case 't':
        port(atoi(optarg));
        break;
      case 'h':
        printUsage(argv[0]);
        exit(0);
      case '?':
      default:
        printUsage(argv[0]);
        return ESB_INVALID_ARGUMENT;
    }
  }

  if (0 < clientThreads() && connections() % clientThreads()) {
    connections(connections() / clientThreads() * clientThreads() + clientThreads());
  }

  dump();
  return ESB_SUCCESS;
}

void HttpTestParams::dump() {
  ESB_LOG_NOTICE(
      "[params] clientThreads=%u, originThreads=%u, proxyThreads=%u, connections=%u, requestsPerConnection=%u, "
      "requestSize=%u, "
      "responseSize=%u, reuseConnections=%d, destination=%s, caPath=%s, serverKeyPath=%s, serverCertPath=%s",
      _clientThreads, _originThreads, _proxyThreads, _connections, _requestsPerConnection, _requestSize, _responseSize,
      _reuseConnections, _destinationAddress, _caPath, _serverKeyPath, _serverCertPath);
}

}  // namespace ES
