#ifndef ES_HTTP_TEST_PARAMS_H
#define ES_HTTP_TEST_PARAMS_H

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifdef ESB_CI_BUILD
#define ESB_TIMEOUT_MULTIPLIER 2
#else
#define ESB_TIMEOUT_MULTIPLIER 1
#endif

namespace ES {

class HttpTestParams {
 public:
  HttpTestParams();

  virtual ~HttpTestParams();

  /**
   * Override test params with command line arguments, check for consistency, and log
   *
   * @param argc the argc passed to main()
   * @param argv the argv passed to main()
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error override(int argc, char **argv);

  void dump();

  inline HttpTestParams &proxyPort(ESB::UInt16 port) {
    _proxyPort = port;
    return *this;
  }

  inline HttpTestParams &originPort(ESB::UInt16 port) {
    _originPort = port;
    return *this;
  }

  inline HttpTestParams &destinationPort(ESB::UInt16 port) {
    _destinationPort = port;
    return *this;
  }

  inline HttpTestParams &clientThreads(ESB::UInt32 clientThreads) {
    _clientThreads = clientThreads;
    return *this;
  }

  inline HttpTestParams &originThreads(ESB::UInt32 originThreads) {
    _originThreads = originThreads;
    return *this;
  }

  inline HttpTestParams &proxyThreads(ESB::UInt32 proxyThreads) {
    _proxyThreads = proxyThreads;
    return *this;
  }

  inline HttpTestParams &connections(ESB::UInt32 connections) {
    _connections = connections;
    return *this;
  }

  inline HttpTestParams &requestsPerConnection(ESB::UInt32 requestsPerConnection) {
    _requestsPerConnection = requestsPerConnection;
    return *this;
  }

  inline HttpTestParams &requestSize(ESB::UInt64 requestSize) {
    _requestSize = requestSize;
    return *this;
  }

  inline HttpTestParams &responseSize(ESB::UInt64 responseSize) {
    _responseSize = responseSize;
    return *this;
  }

  inline HttpTestParams &useContentLengthHeader(bool useContentLengthHeader) {
    _useContentLengthHeader = useContentLengthHeader;
    return *this;
  }

  inline HttpTestParams &reuseConnections(bool reuseConnections) {
    _reuseConnections = reuseConnections;
    return *this;
  }

  inline HttpTestParams &secure(bool secure) {
    _secure = secure;
    return *this;
  }

  inline HttpTestParams &logLevel(ESB::Logger::Severity logLevel) {
    ESB::Logger::Instance().setSeverity(logLevel);
    _logLevel = logLevel;
    return *this;
  }

  inline HttpTestParams &destinationAddress(const char *destinationAddress) {
    _destinationAddress = destinationAddress;
    return *this;
  }

  inline HttpTestParams &hostHeader(const char *hostHeader) {
    _hostHeader = hostHeader;
    return *this;
  }

  inline HttpTestParams &method(const char *method) {
    _method = method;
    return *this;
  }

  inline HttpTestParams &contentType(const char *contentType) {
    _contentType = contentType;
    return *this;
  }

  inline HttpTestParams &absPath(const char *absPath) {
    _absPath = absPath;
    return *this;
  }

  inline HttpTestParams &caPath(const char *caPath) {
    assert(caPath);
    strncpy(_caPath, caPath, sizeof(_caPath));
    _caPath[sizeof(_caPath) - 1] = 0;
    return *this;
  }

  inline HttpTestParams &serverKeyPath(const char *serverKeyPath) {
    assert(serverKeyPath);
    strncpy(_serverKeyPath, serverKeyPath, sizeof(_serverKeyPath));
    _serverKeyPath[sizeof(_serverKeyPath) - 1] = 0;
    return *this;
  }

  inline HttpTestParams &serverCertPath(const char *serverCertPath) {
    assert(serverCertPath);
    strncpy(_serverCertPath, serverCertPath, sizeof(_serverCertPath));
    _serverCertPath[sizeof(_serverCertPath) - 1] = 0;
    return *this;
  }

  inline HttpTestParams &maxVerifyDepth(ESB::UInt32 maxVerifyDepth) {
    _maxVerifyDepth = maxVerifyDepth;
    return *this;
  }

  inline ESB::UInt16 proxyPort() const { return _proxyPort; }

  inline ESB::UInt16 originPort() const { return _originPort; }

  inline ESB::UInt16 destinationPort() const { return _destinationPort; }

  inline ESB::UInt32 clientThreads() const { return _clientThreads; }

  inline ESB::UInt32 originThreads() const { return _originThreads; }

  inline ESB::UInt32 proxyThreads() const { return _proxyThreads; }

  inline ESB::UInt32 connections() const { return _connections; }

  inline ESB::UInt32 requestsPerConnection() const { return _requestsPerConnection; }

  inline ESB::UInt64 requestSize() const { return _requestSize; }

  inline ESB::UInt64 responseSize() const { return _responseSize; }

  inline bool useContentLengthHeader() const { return _useContentLengthHeader; }

  inline bool reuseConnections() const { return _reuseConnections; }

  inline bool secure() const { return _secure; }

  inline ESB::Logger::Severity logLevel() const { return _logLevel; }

  inline const char *destinationAddress() const { return _destinationAddress; }

  inline const char *hostHeader() const { return _hostHeader; }

  inline const char *method() const { return _method; }

  inline const char *contentType() const { return _contentType; }

  inline const char *absPath() const { return _absPath; }

  inline const char *caPath() const { return _caPath; }

  inline const char *serverKeyPath() const { return _serverKeyPath; }

  inline const char *serverCertPath() const { return _serverCertPath; }

  inline int maxVerifyDepth() const { return _maxVerifyDepth; }

  enum DisruptTransaction {
    HAPPY_PATH = 0,
    STALL_SERVER_RECV_HEADERS = 1,
    STALL_SERVER_RECV_BODY = 2,
    STALL_SERVER_SEND_HEADERS = 3,
    STALL_SERVER_SEND_BODY = 4,
    CLOSE_SERVER_RECV_HEADERS = 5,
    CLOSE_SERVER_RECV_BODY = 6,
    CLOSE_SERVER_SEND_HEADERS = 7,
    CLOSE_SERVER_SEND_BODY = 8,
    STALL_CLIENT_RECV_HEADERS = 9,
    STALL_CLIENT_RECV_BODY = 10,
    STALL_CLIENT_SEND_HEADERS = 11,
    STALL_CLIENT_SEND_BODY = 12,
    CLOSE_CLIENT_RECV_HEADERS = 13,
    CLOSE_CLIENT_RECV_BODY = 14,
    CLOSE_CLIENT_SEND_HEADERS = 15,
    CLOSE_CLIENT_SEND_BODY = 16,
  };

  inline DisruptTransaction disruptTransaction() const { return _disruptTransaction; }

  inline HttpTestParams &disruptTransaction(DisruptTransaction distruptTransaction) {
    _disruptTransaction = distruptTransaction;
    return *this;
  }

  ESB::UInt32 clientTimeoutMsec() const { return _clientTimeoutMsec * ESB_TIMEOUT_MULTIPLIER; }

  HttpTestParams &clientTimeoutMsec(ESB::UInt32 clientTimeoutMsec) {
    _clientTimeoutMsec = clientTimeoutMsec;
    return *this;
  }

  ESB::UInt32 proxyTimeoutMsec() const { return _proxyTimeoutMsec * ESB_TIMEOUT_MULTIPLIER; }

  HttpTestParams &proxyTimeoutMsec(ESB::UInt32 proxyTimeoutMsec) {
    _proxyTimeoutMsec = proxyTimeoutMsec;
    return *this;
  }

  ESB::UInt32 originTimeoutMsec() const { return _originTimeoutMsec * ESB_TIMEOUT_MULTIPLIER; }

  HttpTestParams &originTimeoutMsec(ESB::UInt32 originTimeoutMsec) {
    _originTimeoutMsec = originTimeoutMsec;
    return *this;
  }

  void printUsage(const char *progName) const;

 private:
  ESB::UInt16 _proxyPort;
  ESB::UInt16 _originPort;
  ESB::UInt16 _destinationPort;
  ESB::UInt32 _clientThreads;
  ESB::UInt32 _originThreads;
  ESB::UInt32 _proxyThreads;
  ESB::UInt32 _connections;
  ESB::UInt32 _requestsPerConnection;
  ESB::UInt64 _requestSize;
  ESB::UInt64 _responseSize;
  ESB::UInt32 _clientTimeoutMsec;
  ESB::UInt32 _proxyTimeoutMsec;
  ESB::UInt32 _originTimeoutMsec;
  bool _useContentLengthHeader;  // if true use content-length header, else use chunked transfer encoding
  bool _reuseConnections;
  bool _secure;
  ESB::Logger::Severity _logLevel;
  const char *_destinationAddress;
  const char *_hostHeader;
  const char *_method;
  const char *_contentType;
  const char *_absPath;
  char _caPath[ESB_MAX_PATH + 1];
  char _serverKeyPath[ESB_MAX_PATH + 1];
  char _serverCertPath[ESB_MAX_PATH + 1];
  ESB::UInt32 _maxVerifyDepth;
  DisruptTransaction _disruptTransaction;

  ESB_DISABLE_AUTO_COPY(HttpTestParams);
};

}  // namespace ES

#endif
