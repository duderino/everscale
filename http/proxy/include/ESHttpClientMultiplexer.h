#ifndef ES_HTTP_CLIENT_MULTIPLEXER_H
#define ES_HTTP_CLIENT_MULTIPLEXER_H

#ifndef ES_HTTP_MULTIPLEXER_H
#include <ESHttpMultiplexer.h>
#endif

#ifndef ES_HTTP_CLIENT_HANDLER_H
#include <ESHttpClientHandler.h>
#endif

#ifndef ES_HTTP_CLIENT_COUNTERS_H
#include <ESHttpClientCounters.h>
#endif

#ifndef ES_HTTP_CLIENT_SOCKET_FACTORY_H
#include <ESHttpClientSocketFactory.h>
#endif

#ifndef ES_HTTP_CLIENT_TRANSACTION_FACTORY_H
#include <ESHttpClientTransactionFactory.h>
#endif

namespace ES {

class HttpClientMultiplexer : public HttpMultiplexer {
 public:
  HttpClientMultiplexer(ESB::UInt32 connections,
                        HttpSeedTransactionHandler &seedTransactionHandler,
                        ESB::UInt32 maxSockets, HttpClientHandler &handler,
                        HttpClientCounters &counters,
                        ESB::Allocator &allocator);

  virtual ~HttpClientMultiplexer();

  virtual const char *name() const;
  virtual bool run(ESB::SharedInt *isRunning);
  virtual ESB::CleanupHandler *cleanupHandler();

 private:
  // disabled
  HttpClientMultiplexer(const HttpClientMultiplexer &);
  void operator=(const HttpClientMultiplexer &);

  class HttpClientStackImpl : public HttpClientSocket::Stack {
   public:
    HttpClientStackImpl(ESB::EpollMultiplexer &multiplexer,
                        HttpClientSocketFactory &socketFactory,
                        HttpClientTransactionFactory &transactionFactory,
                        HttpClientHandler &handler,
                        HttpClientCounters &counters,
                        ESB::BufferPool &bufferPool);
    virtual ~HttpClientStackImpl();

    virtual HttpClientTransaction *createTransaction();
    virtual bool isRunning();
    virtual ESB::Error executeClientTransaction(
        HttpClientTransaction *transaction);
    virtual void destroyTransaction(HttpClientTransaction *transaction);
    virtual HttpClientHandler &handler();
    virtual HttpClientCounters &counters();
    virtual ESB::BufferPool &bufferPool();

   private:
    // Disabled
    HttpClientStackImpl(const HttpClientStackImpl &);
    HttpClientStackImpl &operator=(const HttpClientStackImpl &);

    ESB::EpollMultiplexer &_multiplexer;
    HttpClientSocketFactory &_socketFactory;
    HttpClientTransactionFactory &_transactionFactory;
    HttpClientHandler &_handler;
    HttpClientCounters &_counters;
    ESB::BufferPool &_bufferPool;
  };

  ESB::UInt32 _connections;
  HttpSeedTransactionHandler &_seedTransactionHandler;
  HttpClientSocketFactory _socketFactory;
  HttpClientTransactionFactory _transactionFactory;
  ESB::DiscardAllocator _ioBufferPoolAllocator;
  ESB::BufferPool _ioBufferPool;
  HttpClientStackImpl _clientStack;
};

}  // namespace ES

#endif
