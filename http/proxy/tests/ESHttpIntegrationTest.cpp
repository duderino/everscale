#ifndef ES_HTTP_INTEGRATION_TEST_H
#include "ESHttpIntegrationTest.h"
#endif

#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ES_HTTP_CLIENT_SOCKET_H
#include <ESHttpClientSocket.h>
#endif

#ifndef ES_HTTP_LOADGEN_SEED_COMMAND_H
#include <ESHttpLoadgenSeedCommand.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

namespace ES {

static volatile ESB::Word IsRunning = 1;
static void SignalHandler(int signal) { IsRunning = 0; }

HttpIntegrationTest::HttpIntegrationTest(TestParams &testParams)
    : _params(testParams),
      _logger(stdout),
      // bind to port 0 so kernel will choose a free ephemeral port
      _originListener("origin-listener", 0, ESB_UINT16_MAX),
      _proxyListener("proxy-listener", 0, ESB_UINT16_MAX),
      _serverHandler(_params.contentType(), _params.responseBody(), _params.responseSize(), _params.requestSize()),
      _originServer("origin", _params.originThreads(), _serverHandler),
      _clientHandler(_params.absPath(), _params.method(), _params.contentType(), _params.requestBody(),
                     _params.requestSize(), _params.responseSize()),
      _client("loadgen", _params.clientThreads(), _clientHandler),
      _proxyRouter(),
      _proxyHandler(_proxyRouter),
      _proxyServer("proxy", _params.proxyThreads(), _proxyHandler) {
  HttpClientSocket::SetReuseConnections(_params.reuseConnections());
}

HttpIntegrationTest::~HttpIntegrationTest() {
  _client.destroy();
  _originServer.destroy();
  if (0 < _params.proxyThreads()) {
    _proxyServer.destroy();
  }
}

ESB::Error HttpIntegrationTest::run() {
  const ESB::UInt32 totalTransactions = _params.connections() * _params.iterations();
  HttpLoadgenContext::SetTotalIterations(totalTransactions);

  ESB::Time::Instance().start();
  ESB::SimpleFileLogger logger(stdout);
  logger.setSeverity(_params.logLevel());
  ESB::Logger::SetInstance(&logger);

  //
  // Install signal handlers: Ctrl-C and kill will start clean shutdown sequence
  //

  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, SignalHandler);
  signal(SIGQUIT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  //
  // Max out open files
  //

  ESB::Error error = ESB::SystemConfig::Instance().setSocketSoftMax(ESB::SystemConfig::Instance().socketHardMax());
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot raise max fd limit");
    return error;
  }

  ESB_LOG_NOTICE("maximum sockets %u", ESB::SystemConfig::Instance().socketSoftMax());

  //
  // Create listening sockets for the origin server and maybe the proxy
  //

  error = _originListener.bind();

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot bind origin to port %u", _originListener.listeningAddress().port());
    return error;
  }

  ESB_LOG_NOTICE("[%s] bound to port %u", _originListener.name(), _originListener.listeningAddress().port());

  ESB::SocketAddress originAddress(_params.destinationAddress(), _originListener.listeningAddress().port(),
                                   ESB::SocketAddress::TransportType::TCP);

  if (0 < _params.proxyThreads()) {
    _proxyRouter.setDestination(originAddress);

    error = _proxyListener.bind();

    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "cannot bind to proxy port %u", _proxyListener.listeningAddress().port());
      return error;
    }

    ESB_LOG_NOTICE("[%s] bound to port %u", _proxyListener.name(), _proxyListener.listeningAddress().port());
  }

  ESB::SocketAddress proxyAddress(_params.destinationAddress(), _proxyListener.listeningAddress().port(),
                                  ESB::SocketAddress::TransportType::TCP);

  //
  // Init client, server, and proxy
  //

  error = _originServer.initialize();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot init origin server");
    return error;
  }

  error = _client.initialize();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot init client");
    return error;
  }

  if (0 < _params.proxyThreads()) {
    error = _proxyServer.initialize();
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "cannot init proxy");
      return error;
    }
  }

  //
  // Start client, server, and proxy
  //

  error = _originServer.start();

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot start origin server");
    return error;
  }

  error = _originServer.addListener(_originListener);
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot add origin listener");
    return error;
  }

  if (0 < _params.proxyThreads()) {
    error = _proxyServer.start();
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "cannot start proxy");
      return error;
    }

    error = _proxyServer.addListener(_proxyListener);
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "cannot add proxy listener");
      return error;
    }
  }

  error = _client.start();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot start client");
    return error;
  }

  // add load generators to running client

  ESB::SocketAddress &clientDestination = 0 < _params.proxyThreads() ? proxyAddress : originAddress;

  for (int i = 0; i < _client.threads(); ++i) {
    HttpLoadgenSeedCommand *command = new (ESB::SystemAllocator::Instance())
        HttpLoadgenSeedCommand(_params.connections() / _params.clientThreads(), _params.iterations(), clientDestination,
                               clientDestination.port(), _params.hostHeader(), _params.absPath(), _params.method(),
                               _params.contentType(), ESB::SystemAllocator::Instance().cleanupHandler());
    error = _client.push(command, i);
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "cannot push seed command");
      return error;
    }
  }

  //
  // Wait for all requests to finish
  //

  while (IsRunning && !HttpLoadgenContext::IsFinished()) {
    sleep(1);
  }

  ESB_LOG_NOTICE("test finished");

  //
  // Stop client, server, and proxy
  //

  error = _client.stop();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot stop client");
    return error;
  }

  if (0 < _params.proxyThreads()) {
    error = _proxyServer.stop();
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "cannot stop proxy");
      return error;
    }
  }

  error = _originServer.stop();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot stop origin server");
    return error;
  }

  // Dump performance metrics

  _client.clientCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);
  _originServer.serverCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);

  if (0 < _params.proxyThreads()) {
    _proxyServer.serverCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);
  }

  ESB::Time::Instance().stop();
  error = ESB::Time::Instance().join();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot stop time thread");
    return error;
  }

  return ESB_SUCCESS;
}

HttpIntegrationTest::TestParams::TestParams()
    : _clientThreads(1),
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

HttpIntegrationTest::TestParams::~TestParams() {
  if (_requestBody) {
    free((void *)_requestBody);
    _requestBody = NULL;
  }
  if (_responseBody) {
    free((void *)_responseBody);
    _responseBody = NULL;
  }
}

const unsigned char *HttpIntegrationTest::TestParams::requestBody() {
  if (!_requestBody) {
    _requestBody = (const unsigned char *)malloc(_requestSize);
    memset((void *)_requestBody, 'a', _requestSize);
  }
  return _requestBody;
}

const unsigned char *HttpIntegrationTest::TestParams::responseBody() {
  if (!_responseBody) {
    _responseBody = (const unsigned char *)malloc(_responseSize);
    memset((void *)_responseBody, 'b', _responseSize);
  }
  return _responseBody;
}

HttpIntegrationTest::TestParams &HttpIntegrationTest::TestParams::requestSize(ESB::UInt32 requestSize) {
  if (_requestBody) {
    free((void *)_requestBody);
    _requestBody = NULL;
  }
  _requestSize = requestSize;
  return *this;
}

HttpIntegrationTest::TestParams &HttpIntegrationTest::TestParams::responseSize(ESB::UInt32 responseSize) {
  if (_responseBody) {
    free((void *)_responseBody);
    _responseBody = NULL;
  }
  _responseSize = responseSize;
  return *this;
}

ESB::Error HttpIntegrationTest::TestParams::override(int argc, char **argv) {
  while (true) {
    int result = getopt(argc, argv, "l:t:c:i:r:");

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
            return ESB_INVALID_ARGUMENT;
        }
        break;
      case 't':
        _clientThreads = atoi(optarg);
        _proxyThreads = _clientThreads;
        _originThreads = _clientThreads;
        break;
      case 'c':
        _connections = atoi(optarg);
        break;
      case 'i':
        _iterations = atoi(optarg);
        break;
      case 'r':
        _reuseConnections = 0 != atoi(optarg);
        break;
    }
  }

  return ESB_SUCCESS;
}

HttpIntegrationTest::HttpFixedRouter::HttpFixedRouter(ESB::SocketAddress &destination) : _destination(destination) {}

HttpIntegrationTest::HttpFixedRouter::HttpFixedRouter() : _destination() {}

HttpIntegrationTest::HttpFixedRouter::~HttpFixedRouter() {}

ESB::Error HttpIntegrationTest::HttpFixedRouter::route(const HttpServerStream &serverStream,
                                                       HttpClientTransaction &clientTransaction,
                                                       ESB::SocketAddress &destination) {
  destination = _destination;
  return ESB_SUCCESS;
}

}  // namespace ES
