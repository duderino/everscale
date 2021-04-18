#ifndef ESB_CLIENT_TLS_CONTEXT_INDEX_H
#define ESB_CLIENT_TLS_CONTEXT_INDEX_H

#ifndef ESB_TLS_CONTEXT_INDEX_H
#include <ESBTLSContextIndex.h>
#endif

namespace ESB {

/**
 * A TLSContextIndex intended for TLS Client use.
 */
class ClientTLSContextIndex : public TLSContextIndex {
 public:
  /**
   * Construct a new TLS Context Index.
   *
   * @param numBuckets Number of buckets for internal hash table.  More buckets -> more memory, fewer collisions.
   * @param numLocks Number of locks for internal hash table.  If 0, no locking.  Else, more locks -> more memory, less
   * contention.
   * @param allocator The allocator to use for allocating internal buckets and nodes.
   */
  ClientTLSContextIndex(UInt32 numBuckets, UInt32 numLocks, Allocator &allocator);

  virtual ~ClientTLSContextIndex();

  /**
   * Load a default context to be used when there are no better matches (unless the default context actually is the
   * best match).
   *
   * @param params private key and certificate paths and other options
   * @return ESB_SUCCESS if successful, ESB_UNIQUENESS_VIOLATION if the default context has already been indexed,
   * another error code otherwise.
   */
  Error indexDefaultContext(const TLSContext::Params &params);

  /**
   * Get the default context to be used when there are no better matches.
   *
   * @return The default context for client TLS sessions
   */
  inline TLSContextPointer &defaultContext() { return _defaultContext; }

  virtual void clear();

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  TLSContextPointer _defaultContext;

  ESB_DISABLE_AUTO_COPY(ClientTLSContextIndex);
};

}  // namespace ESB

#endif
