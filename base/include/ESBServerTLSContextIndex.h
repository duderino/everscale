#ifndef ESB_SERVER_TLS_CONTEXT_INDEX_H
#define ESB_SERVER_TLS_CONTEXT_INDEX_H

#ifndef ESB_TLS_SOCKET_H
#include <ESBTLSSocket.h>
#endif

#ifndef ESB_WILDCARD_INDEX_H
#include <ESBWildcardIndex.h>
#endif

#ifndef ESB_SHARED_EMBEDDED_LIST_H
#include <ESBSharedEmbeddedList.h>
#endif

namespace ESB {

class ServerTLSContext : public ReferenceCount {
 public:
  static Error Create(SmartPointer &pointer, const char *privateKeyPath, const char *certificatePath,
                      ServerTLSContext *memory, CleanupHandler *cleanupHandler);
  virtual ~ServerTLSContext();

  virtual CleanupHandler *cleanupHandler() { return _cleanupHandler; }

  Error commonName(char *buffer, UInt32 size) const;

  Error subjectAltName(char *buffer, UInt32 size, UInt32 *position);

  /**
   * Determine how many DNS subject alternative names are present in the certificate
   *
   * @return The number of DNS SANs in the context's certificate
   */
  inline UInt32 numSubjectAltNames() {
    if (!_subjectAltNames && ESB_SUCCESS != loadSubjectAltNames()) {
      return 0;
    }
    return _numSubjectAltNames;
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param memory The object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ServerTLSContext *memory) noexcept { return memory; }

 private:
  ServerTLSContext(CleanupHandler *handler, SSL_CTX *context);
  Error loadSubjectAltNames();

  CleanupHandler *_cleanupHandler;
  SSL_CTX *_context;
  STACK_OF(GENERAL_NAME) * _subjectAltNames;
  UInt32 _numSubjectAltNames;

  ESB_DISABLE_AUTO_COPY(ServerTLSContext);
};

ESB_SMART_POINTER(ServerTLSContext, ServerTLSContextPointer, SmartPointer);

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
 */
class ServerTLSContextIndex {
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
   * Load the default private key + X509 certificate pair into a TLS context and index the context with all of the X509
   * certificate's subject alt names (or just its common name if it has no subject alt names).  This default context is
   * used for server-side TLS handshakes when the client does not provide the SNI.  For client-side TLS handshakes this
   * is ignored.
   *
   * @param privateKeyPath The path of the PEM encoded private key file
   * @param certificatePath The path of the PEM encoded X509 certificate file
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error indexDefaultContext(const char *privateKeyPath, const char *certificatePath);

  /**
   * Load a private key + X509 certificate pair into a TLS context and index the context with all of the X509
   * certificate's subject alt names (or just its common name if it has no subject alt names).
   *
   * @param privateKeyPath The path of the PEM encoded private key file
   * @param certificatePath The path of the PEM encoded X509 certificate file
   * @param out An optional smart pointer that points to the newly created TLS context
   * @param maskSanConflicts if true (the default), subsequent inserts may overwrite the subject alt name to TLS context
   * mappings of previous inserts.
   * @return ESB_SUCCESS if successful, ESB_UNIQUENESS_VIOLATION if maskSanConflicts is false and 1+ subject alt names
   * already exist in the index, another error code otherwise.
   */
  Error indexContext(const char *privateKeyPath, const char *certificatePath, ServerTLSContextPointer *out = NULL,
                     bool maskSanConflicts = true);

  /**
   * Get the default context used for server-side TLS handshakes when the peer/client doesn't provide an SNI.
   *
   * @return The default context for server-side TLS handshakes
   */
  inline ServerTLSContextPointer &defaultContext() { return _defaultContext; }

  /**
   * Find the TLS context to use for a given fully qualified domain name (fqdn).
   *
   * @param fqdn The fqdn
   * @param pointer A smart pointer to point to the TLS context if the fqdn is found.
   * @return ESB_SUCCESS if found, ESB_CANNOT_FIND if a TLS context cannot be found for the fqdn, another error code
   * otherwise.
   */
  Error matchContext(const char *fqdn, ServerTLSContextPointer &pointer);

  void clear();

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
  class ServerTLSContextCleanupHandler : public CleanupHandler {
   public:
    ServerTLSContextCleanupHandler(SharedEmbeddedList &list) : _deadContexts(list) {}
    virtual ~ServerTLSContextCleanupHandler(){};

    virtual void destroy(Object *object);

   private:
    SharedEmbeddedList &_deadContexts;

    ESB_DISABLE_AUTO_COPY(ServerTLSContextCleanupHandler);
  };

  WildcardIndex _contexts;
  SharedEmbeddedList _deadContexts;
  ServerTLSContextPointer _defaultContext;
  ServerTLSContextCleanupHandler _cleanupHandler;
  Allocator &_allocator;

  ESB_DISABLE_AUTO_COPY(ServerTLSContextIndex);
};

}  // namespace ESB

#endif
