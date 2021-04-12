#ifndef ESB_TLS_CONTEXT_H
#include <ESBTLSContext.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_TLS_SOCKET_H
#include <ESBTLSSocket.h>
#endif

#include <openssl/err.h>
#include <openssl/x509v3.h>

namespace ESB {

X509Certificate::X509Certificate()
    : _certficate(NULL), _subjectAltNames(NULL), _numSubjectAltNames(0U), _freeCertificate(false) {}

X509Certificate::~X509Certificate() {
  if (_subjectAltNames) {
    sk_GENERAL_NAME_pop_free(_subjectAltNames, GENERAL_NAME_free);
    _subjectAltNames = NULL;
  }
  if (_freeCertificate && _certficate) {
    X509_free(_certficate);
    _certficate = NULL;
  }
}

Error X509Certificate::initialize(X509 *certificate, bool freeCertificate) {
  if (initialized()) {
    return ESB_INVALID_STATE;
  }

  _certficate = certificate;
  _freeCertificate = freeCertificate;

  _subjectAltNames = (STACK_OF(GENERAL_NAME) *)X509_get_ext_d2i(_certficate, NID_subject_alt_name, NULL, NULL);
  if (!_subjectAltNames) {
    _numSubjectAltNames = 0U;
    return ESB_SUCCESS;
  }

  UInt32 numSans = sk_GENERAL_NAME_num(_subjectAltNames);
  UInt32 dnsSans = 0U;

  for (UInt32 i = 0; i < numSans; ++i) {
    const GENERAL_NAME *san = sk_GENERAL_NAME_value(_subjectAltNames, i);
    if (GEN_DNS == san->type) {
      ++dnsSans;
    }
  }

  _numSubjectAltNames = dnsSans;
  return ESB_SUCCESS;
}

Error X509Certificate::commonName(char *buffer, UInt32 size) const {
  if (!buffer) {
    return ESB_NULL_POINTER;
  }

  if (!_certficate) {
    return ESB_CANNOT_FIND;
  }

  X509_NAME *subjectName = X509_get_subject_name(_certficate);
  if (!subjectName) {
    return ESB_CANNOT_FIND;
  }

  int lastpos = X509_NAME_get_index_by_NID(subjectName, NID_commonName, -1);
  if (0 > lastpos) {
    return ESB_CANNOT_FIND;
  }

  X509_NAME_ENTRY *entry = X509_NAME_get_entry(subjectName, lastpos);
  if (!entry) {
    return ESB_CANNOT_FIND;
  }

  ASN1_STRING *data = X509_NAME_ENTRY_get_data(entry);
  if (!data) {
    return ESB_CANNOT_FIND;
  }

  int length = ASN1_STRING_length(data);
  const unsigned char *cn = ASN1_STRING_get0_data(data);

  if (size <= length) {
    return ESB_OVERFLOW;
  }

  memcpy(buffer, cn, length);
  buffer[length] = 0;
  return ESB_SUCCESS;
}

Error X509Certificate::subjectAltName(char *buffer, UInt32 size, UInt32 *position) {
  if (!buffer || !position) {
    return ESB_NULL_POINTER;
  }

  if (!_certficate) {
    return ESB_INVALID_STATE;
  }

  if (!_subjectAltNames) {
    return ESB_CANNOT_FIND;
  }

  UInt32 numSans = sk_GENERAL_NAME_num(_subjectAltNames);
  if (numSans <= *position) {
    return ESB_CANNOT_FIND;
  }

  for (UInt32 i = *position; i < numSans; ++i) {
    const GENERAL_NAME *san = sk_GENERAL_NAME_value(_subjectAltNames, i);
    if (GEN_DNS != san->type) {
      *position = i + 1;
      continue;
    }

    int length = ASN1_STRING_length(san->d.dNSName);
    const char *str = (const char *)ASN1_STRING_get0_data(san->d.dNSName);

    if (size <= length) {
      return ESB_OVERFLOW;
    }

    memcpy(buffer, str, length);
    buffer[length] = 0;
    *position = i + 1;
    return ESB_SUCCESS;
  }

  return ESB_CANNOT_FIND;
}

Error TLSContext::Create(TLSContextPointer &pointer, const Params &params, TLSContext *memory,
                         CleanupHandler *cleanupHandler) {
  SSL_CTX *context = SSL_CTX_new(TLS_method());
  if (!context) {
    ESB_LOG_TLS_ERROR("Cannot create TLS context");
    return ESB_OUT_OF_MEMORY;
  }

  X509 *certificate = NULL;

  if (params.privateKeyPath() && params.certificatePath()) {
    if (1 != SSL_CTX_use_PrivateKey_file(context, params.privateKeyPath(), SSL_FILETYPE_PEM)) {
      ESB_LOG_TLS_ERROR("Cannot load private key into TLS context");
      SSL_CTX_free(context);
      return ESB_GENERAL_TLS_ERROR;
    }

    if (1 != SSL_CTX_use_certificate_file(context, params.certificatePath(), SSL_FILETYPE_PEM)) {
      ESB_LOG_TLS_ERROR("Cannot load certificate into TLS context");
      SSL_CTX_free(context);
      return ESB_GENERAL_TLS_ERROR;
    }

    if (1 != SSL_CTX_check_private_key(context)) {
      ESB_LOG_TLS_ERROR("TLS certificate is not valid for private key");
      SSL_CTX_free(context);
      return ESB_GENERAL_TLS_ERROR;
    }

    certificate = SSL_CTX_get0_certificate(context);
    if (!certificate) {
      ESB_LOG_TLS_ERROR("TLS context has no X509 certificate");
      SSL_CTX_free(context);
      return ESB_GENERAL_TLS_ERROR;
    }
  }

  if (params.caCertificatePath()) {
    if (!SSL_CTX_load_verify_locations(context, params.caCertificatePath(), NULL)) {
      ESB_LOG_TLS_ERROR("Cannot load CA certificate into TLS context");
      return ESB_GENERAL_TLS_ERROR;
    }
    SSL_CTX_set_verify_depth(context, params.maxVerifyDepth());
  }

  // TODO use  int SSL_CTX_set_cipher_list(SSL_CTX *ctx, const char *str);

  pointer = new (memory) TLSContext(cleanupHandler, context, params.verifyPeerCertificate());
  if (pointer.isNull()) {
    ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "Cannot create server context");
    SSL_CTX_free(context);
    return ESB_OUT_OF_MEMORY;
  }

  if (certificate) {
    Error error = pointer->certificate().initialize(certificate, false);
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot initialize server context certificate");
      pointer = NULL;
      return error;
    }
  }

  return ESB_SUCCESS;
}

TLSContext::TLSContext(CleanupHandler *handler, SSL_CTX *contex, bool verifyPeerCertificate)
    : _cleanupHandler(handler), _context(contex), _certificate(), _verifyPeerCertificate(verifyPeerCertificate) {}

TLSContext::~TLSContext() {
  if (_context) {
    SSL_CTX_free(_context);
    _context = NULL;
  }
}

TLSContext::Params &TLSContext::Params::reset() {
  _privateKeyPath = NULL;
  _certificatePath = NULL;
  _caCertificatePath = NULL;
  _maxVerifyDepth = 5;
  _verifyPeerCertificate = true;
  return *this;
}
}  // namespace ESB
