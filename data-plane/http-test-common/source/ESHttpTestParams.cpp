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
    : _proxyPort(0),
      _originPort(0),
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
      _maxVerifyDepth(3),
      _disruptTransaction(HAPPY_PATH) {
  caPath(CA_PATH);
  serverKeyPath(KEY_PATH);
  serverCertPath(CERT_PATH);
}

HttpTestParams::~HttpTestParams() {}

void HttpTestParams::printUsage(const char *progName) const {
  fprintf(stderr, "Usage: %s <options>\n", progName);
  fprintf(stderr, "\n");
  fprintf(stderr, "\tOptions:\n");
  fprintf(stderr, "\t--clientThreads <number, default %u>\n", clientThreads());
  fprintf(stderr, "\t--originThreads <number, default %u>\n", originThreads());
  fprintf(stderr, "\t--proxyThreads <number, default %u>\n", proxyThreads());
  fprintf(stderr, "\t--connections <number, default %u>\n", connections());
  fprintf(stderr, "\t--requestsPerConnection <number, default %u>\n", requestsPerConnection());
  fprintf(stderr, "\t--proxyPort <number, default %u (0 means use any free ephemeral)>\n", proxyPort());
  fprintf(stderr, "\t--originPort <number, default %u (0 means use any free ephemeral)>\n", originPort());
  fprintf(stderr, "\t--destinationAddress <IP addr, default %s>\n", destinationAddress());
  fprintf(stderr, "\t--destinationPort <number, default %u (0 means use any free ephemeral)>\n", destinationPort());
  fprintf(stderr, "\t--reuseConnections <number, default %u>\n", reuseConnections());
  fprintf(stderr, "\t--requestBodySize <number, default %lu>\n", requestSize());
  fprintf(stderr, "\t--responseBodySize <number, default %lu>\n", responseSize());
  fprintf(stderr, "\t--clientTimeoutMsec <number, default %u>\n", clientTimeoutMsec());
  fprintf(stderr, "\t--proxyTimeoutMsec <number, default %u>\n", proxyTimeoutMsec());
  fprintf(stderr, "\t--originTimeoutMsec <number, default %u>\n", originTimeoutMsec());
  fprintf(stderr, "\t--secure <number, default %u>\n", secure());
  fprintf(stderr, "\t--caCertPath <path, default %s>\n", caPath());
  fprintf(stderr, "\t--serverKeyPath <path, default %s>\n", serverKeyPath());
  fprintf(stderr, "\t--serverCertPath <path, default %s>\n", serverCertPath());
  fprintf(stderr, "\t--hostHeader <string, default %s>\n", hostHeader());
  fprintf(stderr, "\t--logError\n");
  fprintf(stderr, "\t--logWarning\n");
  fprintf(stderr, "\t--logInfo\n");
  fprintf(stderr, "\t--logDebug\n");
  fprintf(stderr, "\t--logLevel <Log Level, default %u>\n", logLevel());
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
                                      {"requestBodySize", required_argument, NULL, 0},
                                      {"responseBodySize", required_argument, NULL, 0},
                                      {"clientTimeoutMsec", required_argument, NULL, 0},
                                      {"proxyTimeoutMsec", required_argument, NULL, 0},
                                      {"originTimeoutMsec", required_argument, NULL, 0},
                                      {"port", required_argument, NULL, 't'},
                                      {"proxyPort", required_argument, NULL, 0},
                                      {"originPort", required_argument, NULL, 0},
                                      {"destinationPort", required_argument, NULL, 0},
                                      {"destinationAddress", required_argument, NULL, 0},
                                      {"secure", required_argument, NULL, 0},
                                      {"caCertPath", required_argument, NULL, 0},
                                      {"serverKeyPath", required_argument, NULL, 0},
                                      {"serverCertPath", required_argument, NULL, 0},
                                      {"hostHeader", required_argument, NULL, 0},
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
        } else if (0 == strcasecmp("hostHeader", options[idx].name)) {
          hostHeader(optarg);
        } else if (0 == strcasecmp("proxyPort", options[idx].name)) {
          proxyPort(atoi(optarg));
        } else if (0 == strcasecmp("originPort", options[idx].name)) {
          originPort(atoi(optarg));
        } else if (0 == strcasecmp("destinationPort", options[idx].name)) {
          destinationPort(atoi(optarg));
        } else if (0 == strcasecmp("destinationAddress", options[idx].name)) {
          destinationAddress(optarg);
        } else if (0 == strcasecmp("logError", options[idx].name)) {
          logLevel(ESB::Logger::Err);
        } else if (0 == strcasecmp("logWarning", options[idx].name)) {
          logLevel(ESB::Logger::Warning);
        } else if (0 == strcasecmp("logInfo", options[idx].name)) {
          logLevel(ESB::Logger::Info);
        } else if (0 == strcasecmp("logDebug", options[idx].name)) {
          logLevel(ESB::Logger::Debug);
        } else if (0 == strcasecmp("requestBodySize", options[idx].name)) {
          requestSize(strtoul(optarg, NULL, 10));
        } else if (0 == strcasecmp("responseBodySize", options[idx].name)) {
          responseSize(strtoul(optarg, NULL, 10));
        } else if (0 == strcasecmp("clientTimeoutMsec", options[idx].name)) {
          clientTimeoutMsec(atoi(optarg));
        } else if (0 == strcasecmp("proxyTimeoutMsec", options[idx].name)) {
          proxyTimeoutMsec(atoi(optarg));
        } else if (0 == strcasecmp("originTimeoutMsec", options[idx].name)) {
          originTimeoutMsec(atoi(optarg));
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
        fprintf(stderr, "Use --proxyPort and --originPort instead of --port and -t\n");
        exit(1);
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
      "[params] clientThreads=%u, proxyThreads=%u, originThreads=%u, connections=%u, requestsPerConnection=%u, "
      "secure=%s, reuseConnection=%s, requestSize=%lu, responseSize=%lu, destination=%s:%u, caPath=%s, "
      "serverKeyPath=%s, "
      "serverCertPath=%s, proxyPort=%u, originPort=%u",
      _clientThreads, _proxyThreads, _originThreads, _connections, _requestsPerConnection, _secure ? "true" : "false",
      _reuseConnections ? "true" : "false", _requestSize, _responseSize, _destinationAddress, _destinationPort, _caPath,
      _serverKeyPath, _serverCertPath, _proxyPort, _originPort);
}

}  // namespace ES
