#ifndef ESB_TLS_CONTEXT_INDEX_H
#define ESB_TLS_CONTEXT_INDEX_H

#ifndef ESB_WILDCARD_INDEX_H
#include <ESBWildcardIndex.h>
#endif

#ifndef ESB_SHARED_EMBEDDED_LIST_H
#include <ESBSharedEmbeddedList.h>
#endif

#ifndef ESB_TLS_CONTEXT_H
#include <ESBTLSContext.h>
#endif

namespace ESB {

/**
 * An index of ServerTLSContexts which supports requirements 1-3 of RFC 6125 wildcard certificate matching.
 *
 * From https://tools.ietf.org/html/rfc6125#section-6.4.3:
 *
 * 6.4.3.  Checking of Wildcard Certificates
 *
 *   A client employing this specification's rules MAY match the reference
 *   identifier against a presented identifier whose DNS domain name
 *   portion contains the wildcard character '*' as part or all of a label
 *   (following the description of labels and domain names in
 *   [DNS-CONCEPTS]).
 *
 *   For information regarding the security characteristics of wildcard
 *   certificates, see Section 7.2.
 *
 *   If a client matches the reference identifier against a presented
 *   identifier whose DNS domain name portion contains the wildcard
 *   character '*', the following rules apply:
 *
 *   1.  The client SHOULD NOT attempt to match a presented identifier in
 *       which the wildcard character comprises a label other than the
 *       left-most label (e.g., do not match bar.*.example.net).
 *
 *   2.  If the wildcard character is the only character of the left-most
 *       label in the presented identifier, the client SHOULD NOT compare
 *       against anything but the left-most label of the reference
 *       identifier (e.g., *.example.com would match foo.example.com but
 *       not bar.foo.example.com or example.com).
 *
 *   3.  The client MAY match a presented identifier in which the wildcard
 *       character is not the only character of the label (e.g.,
 *       baz*.example.net and *baz.example.net and b*z.example.net would
 *       be taken to match baz1.example.net and foobaz.example.net and
 *       buzz.example.net, respectively).  However, the client SHOULD NOT
 *       attempt to match a presented identifier where the wildcard
 *       character is embedded within an A-label or U-label [IDNA-DEFS] of
 *       an internationalized domain name [IDNA-PROTO].
 *
 * THREAD-SAFETY
 *
 * This index shares immutable SSL_CTX objects across threads without any locking (BoringSSL locks internally).
 *
 * This should be safe according to these comments on the threadsafety of SSL_CTX from one of the maintainers
 * (davidben@davidben.net):
 *
 * From https://github.com/openssl/openssl/issues/2165#issuecomment-270007943
 *
 * An SSL_CTX may be used on multiple threads provided it is not reconfigured. (Even without threads, reconfiguring an
 * SSL_CTX after calling SSL_new will behave weirdly in places.) Observe that the session cache is locked and
 * everything. Also observe that an RSA object goes through a lot of trouble to work around RSA_new + setters (a
 * better API pattern would be functions like RSA_new_private and RSA_new_public which take all their parameters in a
 * single shot and then remove all RSA_set* functions) with BN_MONT_set_locked so that two threads may concurrently
 * perform operations. This is, in part, so that two SSLs on different threads may sign with that shared key in the
 * SSL_CTX.
 *
 * The API typically considers "logically immutable" use of an object to be safe across threads (otherwise there would
 * be little point in even thread-safe refcounts, much less CRYPTO_THREAD_*_lock), but "logically mutable"
 * reconfiguring an object to not be. Of course, the key bits here are "typically" and the scare quotes, so better
 * documentation is probably worthwhile.
 *
 * A bit below that from https://github.com/openssl/openssl/issues/2165#issuecomment-270012533
 *
 * A thread may not reconfigure an SSL_CTX while another thread is accessing it.
 *
 * Even stronger, if there is an SSL attached to the SSL_CTX, reconfiguring the SSL_CTX should probably be undefined
 * (except when documented otherwise). This is how BoringSSL is documented to behave. This is because some fields are
 * copied from SSL_CTX to SSL while others are referenced directly off of SSL_CTX. Reconfiguring the SSL_CTX will
 * behave differently depending on this. If you look at the set of APIs there are, it's not clear either
 * interpretation makes particular sense as universal. I think it's best to just say you don't get to do that. Less
 * combinatorial explosion of cases to test.
 *
 */
class TLSContextIndex {
 public:
  /**
   * Construct a new TLS Context Index.
   *
   * @param numBuckets Number of buckets for internal hash table.  More buckets -> more memory, fewer collisions.
   * @param numLocks Number of locks for internal hash table.  If 0, no locking.  Else, more locks -> more memory, less
   * contention.
   * @param allocator The allocator to use for allocating internal buckets and nodes.
   */
  TLSContextIndex(UInt32 numBuckets, UInt32 numLocks, Allocator &allocator);

  virtual ~TLSContextIndex();

  /**
   * Load a private key + X509 certificate pair into a TLS context and index the context with all of the X509
   * certificate's subject alt names (or just its common name if it has no subject alt names).
   *
   * @param params private key and certificate paths and other options
   * @param out An optional smart pointer that points to the newly created TLS context
   * @param maskSanConflicts if true (the default), subsequent inserts may overwrite the subject alt name to TLS
   * context mappings of previous inserts.
   * @return ESB_SUCCESS if successful, ESB_UNIQUENESS_VIOLATION if maskSanConflicts is false and 1+ subject alt names
   * already exist in the index, ESB_CANNOT_CONVERT if context has no certificate so couldn't be indexed, another error
   * code otherwise.
   */
  Error indexContext(const TLSContext::Params &params, TLSContextPointer *out = NULL, bool maskSanConflicts = true);

  /**
   * Find the TLS context to use for a given fully qualified domain name (fqdn).
   *
   * @param fqdn The fqdn
   * @param pointer A smart pointer to point to the TLS context if the fqdn is found.
   * @return ESB_SUCCESS if found, ESB_CANNOT_FIND if a TLS context cannot be found for the fqdn, another error code
   * otherwise.
   */
  Error matchContext(const char *fqdn, TLSContextPointer &pointer);

  virtual void clear();

  // TODO support removing and updating contexts.  Need to work out what key to use and how to expose clobbered fqdn to
  // SAN associations when maskSanConflicts == true.

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  class TLSContextCleanupHandler : public CleanupHandler {
   public:
    TLSContextCleanupHandler(SharedEmbeddedList &list) : _deadContexts(list) {}
    virtual ~TLSContextCleanupHandler(){};

    virtual void destroy(Object *object);

   private:
    SharedEmbeddedList &_deadContexts;

    ESB_DISABLE_AUTO_COPY(TLSContextCleanupHandler);
  };

  WildcardIndex _contexts;
  SharedEmbeddedList _deadContexts;
  TLSContextCleanupHandler _cleanupHandler;
  Allocator &_allocator;

  ESB_DISABLE_AUTO_COPY(TLSContextIndex);
};

}  // namespace ESB

#endif
