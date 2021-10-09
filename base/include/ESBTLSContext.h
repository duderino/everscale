#ifndef ESB_TLS_CONTEXT_H
#define ESB_TLS_CONTEXT_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

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

 private:
  X509 *_certficate;
  STACK_OF(GENERAL_NAME) * _subjectAltNames;
  UInt32 _numSubjectAltNames;
  bool _freeCertificate;

  ESB_DEFAULT_FUNCS(X509Certificate);
};

class TLSContextPointer;

class TLSContext : public ReferenceCount {
 public:
  enum PeerVerification { VERIFY_NONE = 0, VERIFY_ALWAYS = 1, VERIFY_IF_CERT = 2 };

  class Params {
   public:
    Params()
        : _privateKeyPath(NULL),
          _certificatePath(NULL),
          _caCertificatePath(NULL),
          _maxVerifyDepth(5),
          _verifyPeerCertificate(VERIFY_ALWAYS) {}

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

    inline PeerVerification verifyPeerCertificate() const { return _verifyPeerCertificate; }

    inline Params &verifyPeerCertificate(PeerVerification verifyPeerCertificate) {
      _verifyPeerCertificate = verifyPeerCertificate;
      return *this;
    }

   private:
    const char *_privateKeyPath;
    const char *_certificatePath;
    const char *_caCertificatePath;
    UInt32 _maxVerifyDepth;
    PeerVerification _verifyPeerCertificate;

    ESB_DEFAULT_FUNCS(Params);
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

  inline PeerVerification verifyPeerCertificate() { return _verifyPeerCertificate; }

  inline SSL_CTX *rawContext() { return _context; }

 private:
  TLSContext(CleanupHandler *handler, SSL_CTX *context, PeerVerification verifyPeerCertificate);

  CleanupHandler *_cleanupHandler;
  SSL_CTX *_context;
  X509Certificate _certificate;
  PeerVerification _verifyPeerCertificate;

  ESB_DEFAULT_FUNCS(TLSContext);
};

ESB_SMART_POINTER(TLSContext, TLSContextPointer, SmartPointer);

}  // namespace ESB

#endif
