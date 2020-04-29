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

#ifndef ES_HTTP_CLIENT_COMMAND_SOCKET_H
#include <ESHttpClientCommandSocket.h>
#endif

namespace ES {

class HttpClientMultiplexer : public HttpMultiplexer {
 public:
  HttpClientMultiplexer(ESB::UInt32 maxSockets,
                        HttpClientHandler &clientHandler,
                        HttpClientCounters &clientCounters);

  virtual ~HttpClientMultiplexer();

  virtual const char *name() const;
  virtual bool run(ESB::SharedInt *isRunning);
  virtual ESB::CleanupHandler *cleanupHandler();

  /**
   * Enqueue a command in the multiplexer and wake it up.  When the multiplexer
   * wakes up, it will dequeue the command and execute it in it's thread of
   * control.
   *
   * @param command The command to execute
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  inline ESB::Error pushClientCommand(HttpClientCommand *command) {
    return _clientCommandSocket.push(command);
  }

  // This is what the HttpClientMultiplexer exposes to HttpClientHandler
  class HttpClientStackImpl : public HttpClientStack {
   public:
    HttpClientStackImpl(ESB::EpollMultiplexer &multiplexer,
                        HttpClientSocketFactory &clientSocketFactory,
                        HttpClientTransactionFactory &clientTransactionFactory,
                        ESB::BufferPool &bufferPool);
    virtual ~HttpClientStackImpl();

    virtual HttpClientTransaction *createClientTransaction();
    virtual bool isRunning();
    virtual ESB::Error executeTransaction(HttpClientTransaction *transaction);
    virtual void destroyTransaction(HttpClientTransaction *transaction);
    virtual ESB::Buffer *acquireBuffer();
    virtual void releaseBuffer(ESB::Buffer *buffer);

   private:
    // Disabled
    HttpClientStackImpl(const HttpClientStackImpl &);
    HttpClientStackImpl &operator=(const HttpClientStackImpl &);

    ESB::BufferPool &_bufferPool;
    ESB::EpollMultiplexer &_multiplexer;
    HttpClientSocketFactory &_clientSocketFactory;
    HttpClientTransactionFactory &_clientTransactionFactory;
  };

 private:
  // disabled
  HttpClientMultiplexer(const HttpClientMultiplexer &);
  void operator=(const HttpClientMultiplexer &);

  HttpClientSocketFactory _clientSocketFactory;
  HttpClientTransactionFactory _clientTransactionFactory;
  HttpClientStackImpl _clientStack;
  HttpClientCommandSocket _clientCommandSocket;
};

}  // namespace ES

#endif
