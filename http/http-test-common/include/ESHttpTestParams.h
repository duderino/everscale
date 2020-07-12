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

  inline HttpTestParams &iterations(ESB::UInt32 iterations) {
    _iterations = iterations;
    return *this;
  }

  HttpTestParams &requestSize(ESB::UInt32 requestSize);

  HttpTestParams &responseSize(ESB::UInt32 responseSize);

  inline HttpTestParams &reuseConnections(bool reuseConnections) {
    _reuseConnections = reuseConnections;
    return *this;
  }

  inline HttpTestParams &logLevel(ESB::Logger::Severity logLevel) {
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

  inline ESB::UInt16 port() const { return _port; }

  inline ESB::UInt32 clientThreads() const { return _clientThreads; }

  inline ESB::UInt32 originThreads() const { return _originThreads; }

  inline ESB::UInt32 proxyThreads() const { return _proxyThreads; }

  inline ESB::UInt32 connections() const { return _connections; }

  inline ESB::UInt32 iterations() const { return _iterations; }

  inline ESB::UInt32 requestSize() const { return _requestSize; }

  inline ESB::UInt32 responseSize() const { return _responseSize; }

  inline bool reuseConnections() const { return _reuseConnections; }

  inline ESB::Logger::Severity logLevel() const { return _logLevel; }

  inline const char *destinationAddress() const { return _destinationAddress; }

  inline const char *hostHeader() const { return _hostHeader; }

  inline const char *method() const { return _method; }

  inline const char *contentType() const { return _contentType; }

  inline const char *absPath() const { return _absPath; }

  inline const unsigned char *requestBody() const { return _requestBody; }

  inline const unsigned char *responseBody() const { return _responseBody; }

 private:
  // Disabled
  HttpTestParams(const HttpTestParams &);
  void operator=(const HttpTestParams &);

  ESB::UInt16 _port;
  ESB::UInt32 _clientThreads;
  ESB::UInt32 _originThreads;
  ESB::UInt32 _proxyThreads;
  ESB::UInt32 _connections;
  ESB::UInt32 _iterations;  // requests per connection
  ESB::UInt32 _requestSize;
  ESB::UInt32 _responseSize;
  bool _reuseConnections;
  ESB::Logger::Severity _logLevel;
  const char *_destinationAddress;
  const char *_hostHeader;
  const char *_method;
  const char *_contentType;
  const char *_absPath;
  const unsigned char *_requestBody;
  const unsigned char *_responseBody;
};

}  // namespace ES

#endif
