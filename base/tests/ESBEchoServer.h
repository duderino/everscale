#ifndef ESB_ECHO_SERVER_H
#define ESB_ECHO_SERVER_H

#ifndef ESB_LISTENING_SOCKET_H
#include <ESBListeningSocket.h>
#endif

#ifndef ESB_EPOLL_MULTIPLEXER_H
#include <ESBEpollMultiplexer.h>
#endif

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#ifndef ESB_THREAD_H
#include <ESBThread.h>
#endif

#ifndef ESB_MULTIPLEXED_SOCKET_H
#include <ESBMultiplexedSocket.h>
#endif

#ifndef ESB_CLEAR_SOCKET_H
#include <ESBClearSocket.h>
#endif

#ifndef ESB_SERVER_TLS_SOCKET_H
#include <ESBServerTLSSocket.h>
#endif

namespace ESB {

class EchoServer : public Thread {
 public:
  EchoServer(Allocator &allocator = SystemAllocator::Instance());

  virtual ~EchoServer();

  Error initialize();

  inline const SocketAddress &clearAddress() const { return _clearListener.address(); }

  inline const SocketAddress &secureAddress() const { return _secureListener.address(); }

  inline ServerTLSContextIndex &contextIndex() { return _contextIndex; }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 protected:
  virtual void run();

 private:
  class ClearListener : public MultiplexedSocket {
   public:
    ClearListener(EpollMultiplexer &multiplexer, Allocator &allocator,
                  SocketAddress::TransportType type = SocketAddress::TCP);
    virtual ~ClearListener();

    Error initialize();

    inline const SocketAddress &address() const { return _address; }

    virtual CleanupHandler *cleanupHandler();
    virtual const void *key() const;
    virtual bool permanent();
    virtual bool wantAccept();
    virtual bool wantConnect();
    virtual bool wantRead();
    virtual bool wantWrite();
    virtual Error handleAccept();
    virtual Error handleConnect();
    virtual Error handleReadable();
    virtual Error handleWritable();
    virtual void handleError(Error error);
    virtual void handleRemoteClose();
    virtual void handleIdle();
    virtual void handleRemove();
    virtual SOCKET socketDescriptor() const;
    virtual const char *name() const;
    virtual void markDead();
    virtual bool dead() const;

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return The new object or NULL of the memory allocation failed.
     */
    inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

   protected:
    SocketAddress _address;
    ListeningSocket _listener;
    EpollMultiplexer &_multiplexer;
    Allocator &_allocator;

    ESB_DISABLE_AUTO_COPY(ClearListener);
  };

  class SecureListener : public ClearListener {
   public:
    SecureListener(EpollMultiplexer &multiplexer, Allocator &allocator, ServerTLSContextIndex &index);
    virtual ~SecureListener();

    virtual Error handleAccept();

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return The new object or NULL of the memory allocation failed.
     */
    inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

   private:
    ServerTLSContextIndex &_index;

    ESB_DISABLE_AUTO_COPY(SecureListener);
  };

  class EchoSocket : public MultiplexedSocket {
   public:
    EchoSocket(CleanupHandler &cleanupHandler);
    virtual ~EchoSocket();

    virtual CleanupHandler *cleanupHandler();
    virtual const void *key() const;
    virtual bool permanent();
    virtual bool wantAccept();
    virtual bool wantConnect();
    virtual bool wantRead();
    virtual bool wantWrite();
    virtual Error handleAccept();
    virtual Error handleConnect();
    virtual Error handleReadable();
    virtual Error handleWritable();
    virtual void handleError(Error error);
    virtual void handleRemoteClose();
    virtual void handleIdle();
    virtual void handleRemove();
    virtual SOCKET socketDescriptor() const;
    virtual const char *name() const;
    virtual void markDead();
    virtual bool dead() const;

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return The new object or NULL of the memory allocation failed.
     */
    inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

   protected:
    virtual ConnectedSocket &subclassSocket() = 0;

    virtual const ConnectedSocket &subclassSocket() const = 0;

   private:
    unsigned char _storage[796];
    bool _dead;
    Buffer _buffer;
    CleanupHandler &_cleanupHandler;

    ESB_DISABLE_AUTO_COPY(EchoSocket);
  };

  class ClearEchoSocket : public EchoSocket {
   public:
    ClearEchoSocket(const Socket::State &state, CleanupHandler &cleanupHandler);
    virtual ~ClearEchoSocket();

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return The new object or NULL of the memory allocation failed.
     */
    inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

   protected:
    virtual ConnectedSocket &subclassSocket();

    virtual const ConnectedSocket &subclassSocket() const;

   private:
    ClearSocket _socket;

    ESB_DISABLE_AUTO_COPY(ClearEchoSocket);
  };

  class SecureEchoSocket : public EchoSocket {
   public:
    SecureEchoSocket(const Socket::State &state, ServerTLSContextIndex &index, CleanupHandler &cleanupHandler);
    virtual ~SecureEchoSocket();

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return The new object or NULL of the memory allocation failed.
     */
    inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

   protected:
    virtual ConnectedSocket &subclassSocket();

    virtual const ConnectedSocket &subclassSocket() const;

   private:
    ServerTLSSocket _socket;

    ESB_DISABLE_AUTO_COPY(SecureEchoSocket);
  };

  Allocator &_allocator;
  ServerTLSContextIndex _contextIndex;
  EpollMultiplexer _multiplexer;
  ClearListener _clearListener;
  SecureListener _secureListener;

  ESB_DISABLE_AUTO_COPY(EchoServer);
};

}  // namespace ESB

#endif