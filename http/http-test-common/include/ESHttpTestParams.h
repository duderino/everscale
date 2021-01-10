#ifndef ES_HTTP_TEST_PARAMS_H
#define ES_HTTP_TEST_PARAMS_H

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
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

  inline HttpTestParams &port(ESB::UInt16 port) {
    _port = port;
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

  HttpTestParams &requestSize(ESB::UInt32 requestSize);

  HttpTestParams &responseSize(ESB::UInt32 responseSize);

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

  inline ESB::UInt16 port() const { return _port; }

  inline ESB::UInt32 clientThreads() const { return _clientThreads; }

  inline ESB::UInt32 originThreads() const { return _originThreads; }

  inline ESB::UInt32 proxyThreads() const { return _proxyThreads; }

  inline ESB::UInt32 connections() const { return _connections; }

  inline ESB::UInt32 requestsPerConnection() const { return _requestsPerConnection; }

  inline ESB::UInt32 requestSize() const { return _requestSize; }

  inline ESB::UInt32 responseSize() const { return _responseSize; }

  inline bool useContentLengthHeader() const { return _useContentLengthHeader; }

  inline bool reuseConnections() const { return _reuseConnections; }

  inline bool secure() const { return _secure; }

  inline ESB::Logger::Severity logLevel() const { return _logLevel; }

  inline const char *destinationAddress() const { return _destinationAddress; }

  inline const char *hostHeader() const { return _hostHeader; }

  inline const char *method() const { return _method; }

  inline const char *contentType() const { return _contentType; }

  inline const char *absPath() const { return _absPath; }

  inline const unsigned char *requestBody() const { return _requestBody; }

  inline const unsigned char *responseBody() const { return _responseBody; }

  inline const char *caPath() const { return _caPath; }

  inline const char *serverKeyPath() const { return _serverKeyPath; }

  inline const char *serverCertPath() const { return _serverCertPath; }

  inline int maxVerifyDepth() const { return _maxVerifyDepth; }

 private:
  // Disabled
  HttpTestParams(const HttpTestParams &);
  void operator=(const HttpTestParams &);

  ESB::UInt16 _port;
  ESB::UInt32 _clientThreads;
  ESB::UInt32 _originThreads;
  ESB::UInt32 _proxyThreads;
  ESB::UInt32 _connections;
  ESB::UInt32 _requestsPerConnection;
  ESB::UInt32 _requestSize;
  ESB::UInt32 _responseSize;
  bool _useContentLengthHeader;  // if true use content-length header, else use chunked transfer encoding
  bool _reuseConnections;
  bool _secure;
  ESB::Logger::Severity _logLevel;
  const char *_destinationAddress;
  const char *_hostHeader;
  const char *_method;
  const char *_contentType;
  const char *_absPath;
  unsigned char *_requestBody;
  unsigned char *_responseBody;
  char _caPath[ESB_MAX_PATH + 1];
  char _serverKeyPath[ESB_MAX_PATH + 1];
  char _serverCertPath[ESB_MAX_PATH + 1];
  ESB::UInt32 _maxVerifyDepth;
};

}  // namespace ES

#endif