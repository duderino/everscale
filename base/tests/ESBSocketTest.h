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

#ifndef ESB_ECHO_SERVER_H
#include "ESBEchoServer.h"
#endif

#include <gtest/gtest.h>

namespace ESB {

static SimpleFileLogger TestLogger(stdout, Logger::Info);

class SocketTest : public ::testing::Test {
 public:
  SocketTest(Allocator &allocator = SystemAllocator::Instance()) : _server(allocator) {
    memset(_message, 'a', sizeof(_message));
    _message[sizeof(_message) - 1] = 0;
  }
  virtual ~SocketTest() {}

  static void SetUpTestSuite() { Logger::SetInstance(&TestLogger); }

  static void TearDownTestSuite() { Logger::SetInstance(NULL); }

  virtual void SetUp() {
    Error error = _server.initialize();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot init echo server");
      exit(error);
    }

    error = _server.start();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot start echo server");
      exit(error);
    }
  }

  virtual void TearDown() {
    _server.stop();
    Error error = _server.join();
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot join echo server");
      exit(error);
    }
  }

 protected:
  EchoServer _server;
  char _message[42];
};

}  // namespace ESB

#endif