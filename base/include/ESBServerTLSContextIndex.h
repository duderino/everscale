#ifndef ESB_SERVER_TLS_CONTEXT_INDEX_H
#define ESB_SERVER_TLS_CONTEXT_INDEX_H

#ifndef ESB_TLS_CONTEXT_INDEX_H
#include <ESBTLSContextIndex.h>
#endif

namespace ESB {

/**
 * A TLSContextIndex with an additional default context which can be used for server-SNI interactions.  The default
 * context will handle non-SNI clients with the default cert.  When SNIs are presented, the default context can look
 * up the SNI against the index and potentially replace the context with a better match.
 */
class ServerTLSContextIndex : public TLSContextIndex {
 public:
  /**
   * Construct a new TLS Context Index.
   *
   * @param numBuckets Number of buckets for internal hash table.  More buckets -> more memory, fewer collisions.
   * @param numLocks Number of locks for internal hash table.  If 0, no locking.  Else, more locks -> more memory, less
   * contention.
   * @param allocator The allocator to use for allocating internal buckets and nodes.
   */
  ServerTLSContextIndex(UInt32 numBuckets, UInt32 numLocks, Allocator &allocator);

  virtual ~ServerTLSContextIndex();

  /**
   * Load the default private key + X509 certificate pair into a TLS context and index the context with all of the
   * X509 certificate's subject alt names (or just its common name if it has no subject alt names).  This default
   * context is used for server-side TLS handshakes when the client does not provide the SNI.  For client-side TLS
   * handshakes this is ignored.
   *
   * @param params private key and certificate paths and other options
   * @return ESB_SUCCESS if successful, ESB_UNIQUENESS_VIOLATION if the default context has already been indexed,
   * another error code otherwise.
   */
  Error indexDefaultContext(const TLSContext::Params &params);

  /**
   * Get the default context used for server-side TLS handshakes when the peer/client doesn't provide an SNI.
   *
   * @return The default context for server-side TLS handshakes
   */
  inline TLSContextPointer &defaultContext() { return _defaultContext; }

  virtual void clear();

 private:
  TLSContextPointer _defaultContext;

  ESB_DEFAULT_FUNCS(ServerTLSContextIndex);
};

}  // namespace ESB

#endif
