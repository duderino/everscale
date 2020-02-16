/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_CLIENT_SOCKET_H
#define AWS_CLIENT_SOCKET_H

#ifndef ESF_MULTIPLEXED_SOCKET_H
#include <ESFMultiplexedSocket.h>
#endif

#ifndef ESF_CONNECTED_TCP_SOCKET_H
#include <ESFConnectedTCPSocket.h>
#endif

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

#ifndef ESF_BUFFER_H
#include <ESFBuffer.h>
#endif

#ifndef ESF_BUFFER_POOL_H
#include <ESFBufferPool.h>
#endif

#ifndef AWS_PERFORMANCE_COUNTER_H
#include <AWSPerformanceCounter.h>
#endif

#ifndef AWS_CLIENT_SOCKET_FACTORY_H
#include <AWSClientSocketFactory.h>
#endif

/** A socket that send and receives echo requests.
 *
 * TODO implement idle check
 */
class AWSClientSocket : public ESFMultiplexedSocket {
 public:
  /** Constructor
   *
   * @param peer The peer address to connect to
   * @param cleanupHandler An object that can be used to destroy this one
   * @param logger A logger
   */
  AWSClientSocket(AWSClientSocketFactory *factory,
                  AWSPerformanceCounter *counter, int requestsPerConnection,
                  const ESFSocketAddress &peer,
                  ESFCleanupHandler *cleanupHandler, ESFLogger *logger);

  /** Destructor.
   */
  virtual ~AWSClientSocket();

  /** Does this socket want to accept a new connection?  Implies the
   * implementation is a listening socket.
   *
   * @return true if this socket wants to accept a new connection, false
   * otherwise.
   */
  virtual bool wantAccept();

  /** Does this socket want to connect to a peer?  Implies the implementation is
   * a connected socket (client socket, not server socket).
   *
   * @return true if this socket wants to connect to a peer, false otherwise.
   */
  virtual bool wantConnect();

  /** Does this socket want to read data?  Implies the implementation is a
   * connected socket (client socket or server socket)
   *
   * @return true if this socket wants to read data, false otherwise.
   */
  virtual bool wantRead();

  /** Does this socket want to write data?  Implies the implementation is a
   * connected socket (client socket or server socket).
   *
   * @return true if this socket wants to write data, false otherwise.
   */
  virtual bool wantWrite();

  /** Is this socket idle?
   *
   * @return true if the socket has been idle for too long, false otherwise.
   */
  virtual bool isIdle();

  /** Listening socket may be ready to accept a new connection.  If multiple
   * threads are waiting on the same non-blocking listening socket, one or more
   * threads may not actually be able to accept a new connection when this is
   * called.  This is not an error condition.
   *
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @param logger Log messages should be sent to this object.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleAcceptEvent(ESFFlag *isRunning, ESFLogger *logger);

  /** Client connected socket has connected to the peer endpoint.
   *
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @param logger Log messages should be sent to this object.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleConnectEvent(ESFFlag *isRunning, ESFLogger *logger);

  /** Data is ready to be read.
   *
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @param logger Log messages should be sent to this object.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleReadableEvent(ESFFlag *isRunning, ESFLogger *logger);

  /** There is free space in the outgoing socket buffer.
   *
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @param logger Log messages should be sent to this object.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleWritableEvent(ESFFlag *isRunning, ESFLogger *logger);

  /** An error occurred on the socket while waiting for another event.  The
   * error code should be retrieved from the socket itself.
   *
   * @param errorCode The error code.
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @param logger Log messages should be sent to this object.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor.
   * @see ESFTCPSocket::getLastError to get the socket error
   */
  virtual bool handleErrorEvent(ESFError errorCode, ESFFlag *isRunning,
                                ESFLogger *logger);

  /** The socket's connection was closed.
   *
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @param logger Log messages should be sent to this object.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleEndOfFileEvent(ESFFlag *isRunning, ESFLogger *logger);

  /** The socket's connection has been idle for too long
   *
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @param logger Log messages should be sent to this object.
   * @return If true keep in the multiplexer, if false remove from the
   * multiplexer. Do not close the socket descriptor until after the socket has
   * been removed.
   * @see handleRemoveEvent to close the socket descriptor
   */
  virtual bool handleIdleEvent(ESFFlag *isRunning, ESFLogger *logger);

  /** The socket has been removed from the multiplexer
   *
   * @param isRunning If this object returns false, this method should return as
   * soon as possible.
   * @param logger Log messages should be sent to this object.
   * @return If true, caller should destroy the command with the CleanupHandler.
   */
  virtual bool handleRemoveEvent(ESFFlag *isRunning, ESFLogger *logger);

  /** Get the socket's socket descriptor.
   *
   *  @return the socket descriptor
   */
  virtual SOCKET getSocketDescriptor() const;

  /** Return an optional handler that can destroy the multiplexer.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual ESFCleanupHandler *getCleanupHandler();

  /** Get the name of the multiplexer.  This name can be used in logging
   * messages, etc.
   *
   * @return The multiplexer's name
   */
  virtual const char *getName() const;

  /** Run the command
   *
   * @param isRunning This object will return true as long as the controlling
   * thread isRunning, false when the controlling thread wants to shutdown.
   * @return If true, caller should destroy the command with the CleanupHandler.
   */
  virtual bool run(ESFFlag *isRunning);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

  inline ESFError connect() { return _socket.connect(); }

  static inline void SetReuseConnections(bool reuseConnections) {
    _ReuseConnections = reuseConnections;
  }

  static inline bool GetReuseConnections() { return _ReuseConnections; }

 private:
  // Disabled
  AWSClientSocket(const AWSClientSocket &);
  AWSClientSocket &operator=(const AWSClientSocket &);

  bool setupBuffer();

  struct timeval _start;
  int _requestsPerConnection;
  bool _inReadMode;
  AWSPerformanceCounter *_successCounter;
  ESFLogger *_logger;
  ESFCleanupHandler *_cleanupHandler;
  ESFBuffer *_buffer;
  ESFConnectedTCPSocket _socket;
  AWSClientSocketFactory *_factory;

  static bool _ReuseConnections;
  static ESFBufferPool _BufferPool;
};

#endif /* ! AWS_SERVER_SOCKET_H */
