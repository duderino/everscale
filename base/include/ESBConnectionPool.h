#ifndef ESB_CONNECTION_POOL_H
#define ESB_CONNECTION_POOL_H

#ifndef ESB_SHARED_EMBEDDED_MAP_H
#include <ESBSharedEmbeddedMap.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_CONNECTED_SOCKET_H
#include <ESBConnectedSocket.h>
#endif

#ifndef ESB_CLIENT_TLS_CONTEXT_INDEX_H
#include <ESBClientTLSContextIndex.h>
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
   * @param numBuckets more buckets, fewer collisions, more memory.
   * @param numLocks more locks, less contention, more memory.  if 0, no internal locking will be performed
   * @param contextIndex map of fqdn wildcards to TLS contexts - a TLS context appropriate for the TLSSocket will be
   * borrowed from this index.
   */
  ConnectionPool(const char *namePrefix, UInt32 numBuckets, UInt32 numLocks, ClientTLSContextIndex &contextIndex,
                 Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.  No cleanup handlers are called
   */
  virtual ~ConnectionPool();

  /** Remove, close, and destroy all connections in the connection pool
   */
  void clear();

  /** Remove a clear text connection to a peer from the connection pool, or create a new one if none can be found.
   *
   *  @param peerAddress The peer address
   *  @param connection On success, will be set to a established (if found) or establishing (if new) connection the peer
   * address.
   *  @param reused Set to true if the connection was taken from the pool or false if a new connection was created.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error acquireClearSocket(const SocketAddress &peerAddress, ConnectedSocket **connection, bool *reused);

  /** Remove a TLS encrypted connection to a peer from the connection pool, or create a new one if none can be found.
   *
   * @param fqdn The DNS name of the peer
   *  @param peerAddress The peer address
   *  @param connection On success, will be set to a established (if found) or establishing (if new) connection the peer
   * address.
   *  @param reused Set to true if the connection was taken from the pool or false if a new connection was created.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error acquireTLSSocket(const char *fqdn, const SocketAddress &peerAddress, ConnectedSocket **connection,
                         bool *reused);

  /** Return a connection to the connection pool
   *
   *  @param connection The connection to return to the connection pool
   */
  void release(ConnectedSocket *connection);

  inline int size() const { return _activeSockets.size(); }

  inline int hits() const { return _hits.get(); }

  inline int misses() const { return _misses.get(); }

 private:
  // Callbacks for the hash table-based connection pool
  class SocketAddressCallbacks : public ESB::EmbeddedMapCallbacks {
   public:
    SocketAddressCallbacks(ESB::Allocator &allocator);

    virtual int compare(const void *f, const void *s) const;
    virtual ESB::UInt64 hash(const void *key) const;
    virtual void cleanup(ESB::EmbeddedMapElement *element);

   private:
    Allocator &_allocator;

    ESB_DEFAULT_FUNCS(SocketAddressCallbacks);
  };

  const char *_prefix;
  Allocator &_allocator;
  ClientTLSContextIndex &_contextIndex;
  SharedInt _hits;
  SharedInt _misses;
  SocketAddressCallbacks _callbacks;
  SharedEmbeddedMap _activeSockets;
  EmbeddedList _deconstructedClearSockets;
  EmbeddedList _deconstructedTLSSockets;

  ESB_DEFAULT_FUNCS(ConnectionPool);
};

}  // namespace ESB

#endif
