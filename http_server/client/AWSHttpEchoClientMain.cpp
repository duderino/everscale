/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef ESF_CONSOLE_LOGGER_H
#include <ESFConsoleLogger.h>
#endif

#ifndef ESF_DISCARD_ALLOCATOR_H
#include <ESFDiscardAllocator.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

#ifndef AWS_HTTP_STACK_H
#include <AWSHttpStack.h>
#endif

#ifndef AWS_HTTP_ECHO_CLIENT_CONTEXT_H
#include <AWSHttpEchoClientContext.h>
#endif

#ifndef AWS_HTTP_ECHO_CLIENT_HANDLER_H
#include <AWSHttpEchoClientHandler.h>
#endif

#ifndef AWS_HTTP_DEFAULT_RESOLVER_H
#include <AWSHttpDefaultResolver.h>
#endif

#ifndef AWS_HTTP_ECHO_CLIENT_REQUEST_BUILDER_H
#include <AWSHttpEchoClientRequestBuilder.h>
#endif

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static void AWSRawEchoClientSignalHandler(int signal);
static volatile ESFWord IsRunning = 1;

static void printHelp()
{
    fprintf(stderr, "Usage: -l <logLevel> -m <epollThreads> -h <serverHost> -p <serverPort> -c <connections> -i <iterations> -r\n");
    fprintf(stderr, "\tIf -r is supplied, persistent connections will be used\n");
    fprintf(stderr, "\tlogLevel:\n"
                    "\tNone = 0,\n"
                    "\tEmergency = 1,   System-wide non-recoverable error.\n"
                    "\tAlert = 2,       System-wide non-recoverable error imminent.\n"
                    "\tCritical = 3,    System-wide potentially recoverable error.\n"
                    "\tError = 4,       Localized non-recoverable error.\n"
                    "\tWarning = 5,     Localized potentially recoverable error.\n"
                    "\tNotice = 6,      Important non-error event.\n"
                    "\tInfo = 7,        Non-error event.\n"
                    "\tDebug = 8        Debugging event.\n");
}

int main(int argc, char **argv)
{
    int threads = 4;
    const char *host = "localhost.localdomain";
    int port = 8080;
    int connections = 1;
    int iterations = 1;
    bool reuseConnections = false;
    int logLevel = ESFLogger::Debug;

    {
        int result = 0;

        while (true)
        {
            result = getopt(argc, argv, "l:m:h:p:c:i:r");

            if (0 > result)
            {
                break;
            }

            switch (result)
            {
                case 'l':

                    logLevel = atoi(optarg);
                    break;

                case 'm':

                    threads = atoi(optarg);
                    break;

                case 'h':

                    host = optarg;
                    break;

                case 'p':

                    port = atoi(optarg);
                    break;

                case 'c':

                    connections = atoi(optarg);
                    break;

                case 'i':

                    iterations = atoi(optarg);
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

    ESFConsoleLogger::Initialize((ESFLogger::Severity) logLevel);
    ESFLogger *logger = ESFConsoleLogger::Instance();

    if (logger->isLoggable(ESFLogger::Notice))
    {
        logger->log(ESFLogger::Notice, __FILE__, __LINE__,
                    "[main] starting. logLevel: %d, threads: %d, host: %s, port: %d, connections: %d, iterations: %d, reuseConnections: %s",
                    logLevel, threads, host, port, connections, iterations, reuseConnections ? "true" : "false");
    }

    //
    // Install signal handlers: Ctrl-C and kill will start clean shutdown sequence
    //

    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, AWSRawEchoClientSignalHandler);
    signal(SIGQUIT, AWSRawEchoClientSignalHandler);
    signal(SIGTERM, AWSRawEchoClientSignalHandler);

    //
    // Create, initialize, and start the stack
    //

    AWSHttpDefaultResolver resolver(logger);

    AWSHttpStack stack(&resolver, threads, logger);

    AWSHttpEchoClientHandler handler(connections * iterations, &stack, logger);

    // TODO - make configuration stack-specific and increase options richness
    AWSHttpClientSocket::SetReuseConnections(reuseConnections);

    ESFError error = stack.initialize();

    if (ESF_SUCCESS != error)
    {
        return -1;
    }

    error = stack.start();

    if (ESF_SUCCESS != error)
    {
        return -2;
    }

    ESFDiscardAllocator echoClientContextAllocator(1024, ESFSystemAllocator::GetInstance());

    sleep(1);   // give the worker threads a chance to start - cleans up perf testing numbers a bit

    handler.setStartTime();

    // Create <connections> distinct client connections which each submit <iterations> SOAP requests

    AWSHttpEchoClientContext *context = 0;
    AWSHttpClientTransaction *transaction = 0;

    for (int i = 0; i < connections; ++i)
    {
        // Create the request context and transaction

        context = new(&echoClientContextAllocator) AWSHttpEchoClientContext(iterations - 1);

        if (0 == context)
        {
            if (logger->isLoggable(ESFLogger::Critical))
            {
                logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                            "[main] cannot create new client context");
            }

            return -3;
        }

        transaction = stack.createClientTransaction(&handler);

        if (0 == transaction)
        {
            context->~AWSHttpEchoClientContext();
            echoClientContextAllocator.deallocate(context);

            if (logger->isLoggable(ESFLogger::Critical))
            {
                logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                            "[main] cannot create new client transaction");
            }

            return -3;
        }

        transaction->setApplicationContext(context);

        // Build the request

        error = AWSHttpEchoClientRequestBuilder(host, port, transaction);

        if (ESF_SUCCESS != error)
        {
            context->~AWSHttpEchoClientContext();
            echoClientContextAllocator.deallocate(context);
            stack.destroyClientTransaction(transaction);

            if (logger->isLoggable(ESFLogger::Critical))
            {
                char buffer[100];

                ESFDescribeError(error, buffer, sizeof(buffer));

                logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                            "[main] cannot build request: %s");
            }

            return -4;
        }

        // Send the request (asynch) - the context will resubmit the request for <iteration> - 1 iterations.

        error = stack.executeClientTransaction(transaction);

        if (ESF_SUCCESS != error)
        {
            context->~AWSHttpEchoClientContext();
            echoClientContextAllocator.deallocate(context);
            stack.destroyClientTransaction(transaction);

            if (logger->isLoggable(ESFLogger::Critical))
            {
                char buffer[100];

                ESFDescribeError(error, buffer, sizeof(buffer));

                logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                            "[main] Cannot execute client transaction: %s", buffer);
            }

            return error;
        }
    }

    while (IsRunning)
    {
        sleep(60);
    }

    error = stack.stop();

    if (ESF_SUCCESS != error)
    {
        return -3;
    }

    ESFUInt32 seconds = handler.getStopTime()->tv_sec - handler.getStartTime()->tv_sec;
    ESFUInt32 microseconds = 0;

    if (handler.getStopTime()->tv_usec < handler.getStartTime()->tv_usec)
    {
        --seconds;
        microseconds = handler.getStopTime()->tv_usec - (ESF_UINT32_C(1000000) - handler.getStartTime()->tv_usec);
    }
    else
    {
        microseconds = handler.getStopTime()->tv_usec - handler.getStartTime()->tv_usec;
    }

    stack.getClientSuccessCounter()->printSummary();
    stack.getClientFailureCounter()->printSummary();

    fprintf(stdout, "Total Sec: %u.%u\n", seconds, microseconds);

    stack.destroy();

    echoClientContextAllocator.destroy();   // echo client context destructors will not be called.

    ESFConsoleLogger::Destroy();

    return ESF_SUCCESS;
}

void AWSRawEchoClientSignalHandler(int signal)
{
    IsRunning = 0;
}

