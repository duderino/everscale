#ifndef ESB_CONNECTION_POOL_H
#define ESB_CONNECTION_POOL_H

#ifndef ESB_SHARED_EMBEDDED_MAP_H
#include <ESBSharedEmbeddedMap.h>
#endif

#ifndef ESB_CONNECTED_SOCKET_H
#include <ESBConnectedSocket.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

namespace ESB {

/** A connection pool that can optionally be shared by multiple threads.
 *
 *  @ingroup network
 */
class ConnectionPool {
 public:
  /** Constructor.
   *
   * @param namePrefix connected sockets created by this pool will have names prefixed by this string
   * @param nameSuffix connected sockets created by this pool will have this string appended to the end
   * @param numBuckets more buckets, fewer collisions, more memory.
   * @param numLocks more locks, less contention, more memory.  if 0, no
   * internal locking will be performed
   */
  ConnectionPool(const char *namePrefix, const char *nameSuffix, UInt32 numBuckets, UInt32 numLocks,
                 Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.  No cleanup handlers are called
   */
  virtual ~ConnectionPool();

  /** Remove, close, and destroy all connections in the connection pool
   */
  void clear();

  /** Find a connection in the connection pool
   *
   *  @param peerAddress The peer address
   *  @param connection Either set to a connection already connected to the peerAddress, or NULL if no connection could
   * be found.
   *  @param reused Set to true if the connection was taken from the pool or false if a new connection was created.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error acquire(const SocketAddress &peerAddress, ConnectedSocket **connection, bool *reused);

  /** Return a connection to the connection pool
   *
   *  @param connection The connection to return to the connection pool
   */
  void release(ConnectedSocket *connection);

  inline int size() const { return _activeSockets.size(); }

  inline int hits() const { return _hits.get(); }

  inline int misses() const { return _misses.get(); }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  // Disabled
  ConnectionPool(const ConnectionPool &);
  ConnectionPool &operator=(const ConnectionPool &);

  // Callbacks for the hash table-based connection pool
  class SocketAddressCallbacks : public ESB::SharedEmbeddedMap::Callbacks {
   public:
    SocketAddressCallbacks(ESB::Allocator &allocator);

    virtual int compare(const void *f, const void *s) const;
    virtual ESB::UInt32 hash(const void *key) const;
    virtual void cleanup(ESB::EmbeddedMapElement *element);

   private:
    // Disabled
    SocketAddressCallbacks(const SocketAddressCallbacks &);
    SocketAddressCallbacks &operator=(const SocketAddressCallbacks &);

    Allocator &_allocator;
  };

  const char *_namePrefix;
  const char *_nameSuffix;
  Allocator &_allocator;
  SharedInt _hits;
  SharedInt _misses;
  SocketAddressCallbacks _callbacks;
  SharedEmbeddedMap _activeSockets;
  EmbeddedList _deconstructedClearSockets;
  EmbeddedList _deconstructedTLSSockets;
};

}  // namespace ESB

#endif
