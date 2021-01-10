#ifndef ESB_SOCKET_TEST_H
#define ESB_SOCKET_TEST_H

#ifndef ESB_LISTENING_SOCKET_H
#include <ESBListeningSocket.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#include <gtest/gtest.h>

namespace ESB {

class SocketTest : public ::testing::Test {
 public:
  SocketTest()
      : _clearListenerAddress("127.0.0.1", ESB_ANY_PORT, SocketAddress::TCP),
        _secureListenerAddress("127.0.0.1", ESB_ANY_PORT, SocketAddress::TLS),
        _clearListener("clear", _clearListenerAddress, ESB_UINT16_MAX),
        _secureListener("secure", _secureListenerAddress, ESB_UINT16_MAX) {
    for (int i = 0; i < sizeof(_message) - 1; ++i) {
      _message[i] = '0' + i % 10;
    }

    _message[sizeof(_message) - 1] = 0;
  }

  static void SetUpTestSuite() { Logger::SetInstance(&_Logger); }

  static void TearDownTestSuite() { Logger::SetInstance(NULL); }

  virtual void SetUp() {
    Error error = _clearListener.bind();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s] cannot bind to ephemeral port", _clearListener.name());
      assert(!"Cannot bind to ephemeral port");
      return;
    }

    error = _secureListener.bind();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s] cannot bind to ephemeral port", _secureListener.name());
      assert(!"Cannot bind to ephemeral port");
      return;
    }

    error = _clearListener.listen();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s] cannot listen on ephemeral port", _clearListener.name());
      assert(!"Cannot listen on ephemeral port");
      return;
    }

    error = _secureListener.listen();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s] cannot listen on ephemeral port", _secureListener.name());
      assert(!"Cannot listen on ephemeral port");
      return;
    }

    _clearListenerAddress.setPort(_clearListener.listeningAddress().port());
    _secureListenerAddress.setPort(_secureListener.listeningAddress().port());
  }

 protected:
  SocketAddress _clearListenerAddress;
  SocketAddress _secureListenerAddress;
  ListeningSocket _clearListener;
  ListeningSocket _secureListener;
  char _message[42];

 private:
  static SimpleFileLogger _Logger;
};

SimpleFileLogger SocketTest::_Logger(stdout, Logger::Debug);

}  // namespace ESB

#endif