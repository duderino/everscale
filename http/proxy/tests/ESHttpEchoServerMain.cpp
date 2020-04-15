#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#ifndef ES_HTTP_SERVER_H
#include <ESHttpServer.h>
#endif

#ifndef ES_HTTP_ECHO_SERVER_HANDLER_H
#include <ESHttpEchoServerHandler.h>
#endif

#ifndef ESB_SYSTEM_DNS_CLIENT_H
#include <ESBSystemDnsClient.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
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

class AddListeningSocketCommand : public HttpServerCommand {
 public:
  AddListeningSocketCommand(ESB::ListeningTCPSocket &socket,
                             ESB::CleanupHandler &cleanupHandler)
      : _socket(socket), _cleanupHandler(cleanupHandler) {}

  virtual ~AddListeningSocketCommand(){};

  virtual ESB::Error run(HttpServerStack &stack) {
    return stack.addListeningSocket(_socket);
  }

  virtual ESB::CleanupHandler *cleanupHandler() { return &_cleanupHandler; }

  virtual const char *name() { return "ListeningSocketCommand"; }

 private:
  // Disabled
  AddListeningSocketCommand(const AddListeningSocketCommand &);
  AddListeningSocketCommand &operator=(const AddListeningSocketCommand &);

  ESB::ListeningTCPSocket &_socket;
  ESB::CleanupHandler &_cleanupHandler;
};

}  // namespace ES

static volatile ESB::Word IsRunning = 1;
static void SignalHandler(int signal) { IsRunning = 0; }

using namespace ES;

static void printHelp() {
  fprintf(stderr, "Usage: -l <logLevel> -m <threads> -p <port>\n");
}

int main(int argc, char **argv) {
  int port = 8080;
  int threads = 4;
  int logLevel = ESB::Logger::Notice;

  {
    int result = 0;

    while (true) {
      result = getopt(argc, argv, "l:m:p:");

      if (0 > result) {
        break;
      }

      switch (result) {
        case 'l':

          /*
          None = 0,
          Emergency = 1,   System-wide non-recoverable error.
          Alert = 2,       System-wide non-recoverable error imminent.
          Critical = 3,    System-wide potentially recoverable error.
          Error = 4,       Localized non-recoverable error.
          Warning = 5,     Localized potentially recoverable error.
          Notice = 6,      Important non-error event.
          Info = 7,        Non-error event.
          Debug = 8        Debugging event.
          */

          logLevel = atoi(optarg);
          break;

        case 'm':

          threads = atoi(optarg);
          break;

        case 'p':

          port = atoi(optarg);
          break;

        default:

          printHelp();

          return 2;
      }
    }
  }

  ESB::Time::Instance().start();
  ESB::SimpleFileLogger logger(stdout);
  logger.setSeverity((ESB::Logger::Severity)logLevel);
  ESB::Logger::SetInstance(&logger);

  ESB_LOG_NOTICE("[main] starting. logLevel: %d, threads: %d, port: %d",
                 logLevel, threads, port);

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

  ESB::Error error = ESB::SystemConfig::Instance().setSocketSoftMax(
      ESB::SystemConfig::Instance().socketHardMax());

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot raise max fd limit");
    return -1;
  }

  //
  // Create listening socket
  //

  ESB::ListeningTCPSocket listener(port, ESB_UINT16_MAX);

  error = listener.bind();

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot bind to port %u",
                           listener.listeningAddress().port());
    return -2;
  }

  ESB_LOG_NOTICE("Bound to port %u", listener.listeningAddress().port());

  error = listener.listen();

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot listen on port %u",
                           listener.listeningAddress().port());
    return -3;
  }

  // Init

  HttpEchoServerHandler handler;
  HttpServer server(threads, handler);

  error = server.initialize();

  if (ESB_SUCCESS != error) {
    return -4;
  }

  // Start

  error = server.start();

  if (ESB_SUCCESS != error) {
    return -5;
  }

  // add listening sockets to running server

  for (int i = 0; i < server.threads(); ++i) {
    AddListeningSocketCommand *command =
        new (ESB::SystemAllocator::Instance()) AddListeningSocketCommand(
            listener, ESB::SystemAllocator::Instance().cleanupHandler());
    error = server.push(command, i);
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "Cannot push seed command");
      return -6;
    }
  }

  // Wait for ctrl-C

  while (IsRunning) {
    sleep(1);
  }

  // Stop server

  error = server.stop();

  if (ESB_SUCCESS != error) {
    return -7;
  }

  server.counters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);
  server.destroy();

  return 0;
}

void HttpEchoServerSignalHandler(int signal) { IsRunning = 0; }
