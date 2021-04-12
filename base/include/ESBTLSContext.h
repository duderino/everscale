#ifndef ESB_TLS_CONTEXT_H
#define ESB_TLS_CONTEXT_H

#ifndef ESB_REFERENCE_COUNT_H
#include <ESBReferenceCount.h>
#endif

#ifndef ESB_SMART_POINTER_H
#include <ESBSmartPointer.h>
#endif

#include <openssl/ssl.h>

namespace ESB {

class X509Certificate {
 public:
  X509Certificate();

  virtual ~X509Certificate();

  Error initialize(X509 *certificate, bool freeCertificate);

  inline bool initialized() const { return _certficate && _subjectAltNames; }

  Error commonName(char *buffer, UInt32 size) const;

  Error subjectAltName(char *buffer, UInt32 size, UInt32 *position);

  /**
   * Determine how many DNS subject alternative names are present in the certificate
   *
   * @return The number of DNS SANs in the context's certificate
   */
  inline UInt32 numSubjectAltNames() { return _numSubjectAltNames; }

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
  inline void *operator new(size_t size, X509Certificate *memory) noexcept { return memory; }

 private:
  X509 *_certficate;
  STACK_OF(GENERAL_NAME) * _subjectAltNames;
  UInt32 _numSubjectAltNames;
  bool _freeCertificate;

  ESB_DISABLE_AUTO_COPY(X509Certificate);
};

class TLSContextPointer;

class TLSContext : public ReferenceCount {
 public:
  class Params {
   public:
    Params()
        : _privateKeyPath(NULL),
          _certificatePath(NULL),
          _caCertificatePath(NULL),
          _maxVerifyDepth(5),
          _verifyPeerCertificate(true) {}

    virtual ~Params() {}

    Params &reset();

    inline const char *privateKeyPath() const { return _privateKeyPath; }

    inline Params &privateKeyPath(const char *privateKeyPath) {
      _privateKeyPath = privateKeyPath;
      return *this;
    }

    inline const char *certificatePath() const { return _certificatePath; }

    inline Params &certificatePath(const char *certificatePath) {
      _certificatePath = certificatePath;
      return *this;
    }

    inline const char *caCertificatePath() const { return _caCertificatePath; }

    inline Params &caCertificatePath(const char *caCertificatePath) {
      _caCertificatePath = caCertificatePath;
      return *this;
    }

    inline int maxVerifyDepth() const { return _maxVerifyDepth; }

    inline Params &maxVerifyDepth(int maxVerifyDepth) {
      _maxVerifyDepth = maxVerifyDepth;
      return *this;
    }

    inline bool verifyPeerCertificate() const { return _verifyPeerCertificate; }

    inline Params &verifyPeerCertificate(bool verifyPeerCertificate) {
      _verifyPeerCertificate = verifyPeerCertificate;
      return *this;
    }

   private:
    const char *_privateKeyPath;
    const char *_certificatePath;
    const char *_caCertificatePath;
    int _maxVerifyDepth;
    bool _verifyPeerCertificate;

    ESB_DISABLE_AUTO_COPY(Params);
  };

  static Error Create(TLSContextPointer &pointer, const Params &params, TLSContext *memory,
                      CleanupHandler *cleanupHandler);

  virtual ~TLSContext();

  virtual CleanupHandler *cleanupHandler() { return _cleanupHandler; }

  /**
   * Get the local certificate associated with the context
   *
   * @param cert will point to the peer certificate on success
   * @return ESB_SUCCESS if succesful, ESB_CANNOT_FIND if the peer has not (yet) sent its certificate, another error
   * code otherwise.
   */
  inline X509Certificate &certificate() { return _certificate; }

  inline bool verifyPeerCertificate() { return _verifyPeerCertificate; }

  inline SSL_CTX *rawContext() { return _context; }

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
  inline void *operator new(size_t size, TLSContext *memory) noexcept { return memory; }

 private:
  TLSContext(CleanupHandler *handler, SSL_CTX *context, bool verifyPeerCertificate);

  CleanupHandler *_cleanupHandler;
  SSL_CTX *_context;
  X509Certificate _certificate;
  bool _verifyPeerCertificate;

  ESB_DISABLE_AUTO_COPY(TLSContext);
};

ESB_SMART_POINTER(TLSContext, TLSContextPointer, SmartPointer);

}  // namespace ESB

#endif
