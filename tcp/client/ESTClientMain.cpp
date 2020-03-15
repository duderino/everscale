#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_PROCESS_LIMITS_H
#include <ESBProcessLimits.h>
#endif

#ifndef ESB_EPOLL_MULTIPLEXER_FACTORY_H
#include <ESBEpollMultiplexerFactory.h>
#endif

#ifndef ESB_SOCKET_MULTIPLEXER_DISPATCHER_H
#include <ESBSocketMultiplexerDispatcher.h>
#endif

#ifndef EST_CLIENT_SOCKET_H
#include <ESTClientSocket.h>
#endif

#ifndef ESB_SHARED_ALLOCATOR_H
#include <ESBSharedAllocator.h>
#endif

#ifndef ESB_ALLOCATOR_CLEANUP_HANDLER_H
#include <ESBAllocatorCleanupHandler.h>
#endif

#ifndef EST_PERFORMANCE_COUNTER_H
#include <ESTPerformanceCounter.h>
#endif

#ifndef ESB_TIME_H
#include <ESBTime.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

static void ClientSignalHandler(int signal);
static volatile ESB::Word IsRunning = 1;
static EST::PerformanceCounter SuccessCounter("successes");
static struct timeval StopTime;

static void printHelp() {
  fprintf(stderr,
          "Usage: -l <logLevel> -m <threads> -a <serverIp> -p <serverPort> -s "
          "<sockets> -r\n");
}

int main(int argc, char **argv) {
  ESB::UInt16 multiplexerCount = 4;
  const char *dottedIp = "127.0.0.1";
  ESB::UInt16 port = 8080;
  ESB::UInt32 sockets = 1;
  bool reuseConnections = false;
  int logLevel = ESB::Logger::Notice;

  {
    int result = 0;

    while (true) {
      result = getopt(argc, argv, "l:m:a:p:s:r");

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

          multiplexerCount = atoi(optarg);
          break;

        case 'a':

          dottedIp = optarg;
          break;

        case 'p':

          port = atoi(optarg);
          break;

        case 's':

          sockets = (ESB::UInt32)atoi(optarg);
          break;

        case 'r':

          reuseConnections = true;
          break;

        default:

          printHelp();

          return 2;
      }
    }
  }

  ESB::Time::Instance().start();
  ESB::SimpleFileLogger logger;
  logger.setSeverity((ESB::Logger::Severity)logLevel);
  ESB::Logger::SetInstance(&logger);

  ESB_LOG_NOTICE(
      "starting. logLevel: %d, threads: %d, server: %s, port: "
      "%d, sockets: %u, reuseConnections: %s",
      logLevel, multiplexerCount, dottedIp, port, sockets,
      reuseConnections ? "true" : "false");

  EST::ClientSocket::SetReuseConnections(reuseConnections);
  ESB::SocketAddress serverAddress(dottedIp, port, ESB::SocketAddress::TCP);

  //
  // Install signal handlers
  //

  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, ClientSignalHandler);
  signal(SIGQUIT, ClientSignalHandler);
  signal(SIGTERM, ClientSignalHandler);

  ESB::DiscardAllocator discardAllocator(4000,
                                         ESB::SystemAllocator::GetInstance());
  ESB::SharedAllocator rootAllocator(&discardAllocator);
  ESB::AllocatorCleanupHandler rootAllocatorCleanupHandler(&rootAllocator);

  ESB::Error error = rootAllocator.initialize();

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot initialize root allocator");
    return error;
  }

  ESB::EpollMultiplexerFactory epollFactory("EpollMultiplexer", &rootAllocator);

  ESB::ProcessLimits::SetSocketSoftMax(ESB::ProcessLimits::GetSocketHardMax());
  ESB::UInt32 maxSockets = ESB::ProcessLimits::GetSocketSoftMax();

  if (sockets > maxSockets) {
    ESB_LOG_ERROR("Raise ulimit -n, only %d sockets can be created",
                  maxSockets);
    return 1;
  }

  ESB_LOG_NOTICE("Maximum sockets %d", maxSockets);

  ESB::SocketMultiplexerDispatcher dispatcher(maxSockets, multiplexerCount,
                                              &epollFactory, &rootAllocator,
                                              "EpollDispatcher");

  error = dispatcher.start();

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot start multiplexer dispatcher");
    return error;
  }

  EST::ClientSocketFactory factory(maxSockets, &SuccessCounter, &dispatcher);

  sleep(1);  // give the worker threads a chance to start - cleans up perf
             // testing numbers a bit

  struct timeval startTime;
  EST::PerformanceCounter::GetTime(&startTime);

  for (ESB::UInt32 i = 0; i < sockets; ++i) {
    error = factory.addNewConnection(serverAddress);

    if (ESB_SUCCESS != error) {
      ESB_LOG_WARNING_ERRNO(error, "Cannot add new connection");
      return error;
    }
  }

  ESB_LOG_NOTICE("started");
  while (IsRunning) {
    sleep(1);
  }

  ESB_LOG_NOTICE("stopping");
  dispatcher.stop();
  ESB_LOG_NOTICE("stopped");

  ESB::UInt32 seconds = StopTime.tv_sec - startTime.tv_sec;
  ESB::UInt32 microseconds = 0;

  if (StopTime.tv_usec < startTime.tv_usec) {
    --seconds;
    microseconds =
        StopTime.tv_usec - (ESB_UINT32_C(1000000) - startTime.tv_usec);
  } else {
    microseconds = StopTime.tv_usec - startTime.tv_usec;
  }

  SuccessCounter.printSummary();
  fprintf(stdout, "\tTotal Sec: %u.%u\n", seconds, microseconds);

  ESB::Time::Instance().stop();
  error = ESB::Time::Instance().join();
  assert(ESB_SUCCESS == error);

  return ESB_SUCCESS;
}

void ClientSignalHandler(int signal) {
  IsRunning = 0;
  EST::PerformanceCounter::GetTime(&StopTime);
}
