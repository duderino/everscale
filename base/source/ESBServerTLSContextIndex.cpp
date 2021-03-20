#ifndef ESB_SERVER_TLS_CONTEXT_INDEX_H
#include <ESBServerTLSContextIndex.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#include <openssl/err.h>
#include <openssl/x509v3.h>

namespace ESB {

Error ServerTLSContext::Create(SmartPointer &pointer, const char *privateKeyPath, const char *certificatePath,
                               ServerTLSContext *memory, CleanupHandler *cleanupHandler) {
  if (!privateKeyPath || !certificatePath) {
    return ESB_NULL_POINTER;
  }

  SSL_CTX *context = SSL_CTX_new(TLS_method());

  if (!context) {
    ESB_LOG_TLS_ERROR("Cannot create server context");
    return ESB_OUT_OF_MEMORY;
  }

  if (!SSL_CTX_use_PrivateKey_file(context, privateKeyPath, SSL_FILETYPE_PEM)) {
    ESB_LOG_TLS_ERROR("Cannot load private key into server context");
    SSL_CTX_free(context);
    return ESB_GENERAL_TLS_ERROR;
  }

  // TODO use OPENSSL_EXPORT int SSL_use_certificate(SSL *ssl, X509 *x509); for non-file based.
  if (!SSL_CTX_use_certificate_file(context, certificatePath, SSL_FILETYPE_PEM)) {
    ESB_LOG_TLS_ERROR("Cannot load certificate into server context");
    SSL_CTX_free(context);
    return ESB_GENERAL_TLS_ERROR;
  }

  // TODO Ensure private key and certificate match int SSL_CTX_check_private_key(const SSL_CTX *ctx);
  // TODO use  int SSL_CTX_set_cipher_list(SSL_CTX *ctx, const char *str);
  // TODO use SSL_CTX_set_client_CA_list or SSL_CTX_load_verify_locations for mTLS}

  pointer = new (memory) ServerTLSContext(cleanupHandler, context);

  if (pointer.isNull()) {
    ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "Cannot create server context");
    SSL_CTX_free(context);
    return ESB_OUT_OF_MEMORY;
  }

  return ESB_SUCCESS;
}

ServerTLSContext::ServerTLSContext(CleanupHandler *handler, SSL_CTX *contex)
    : _cleanupHandler(handler), _context(contex), _subjectAltNames(NULL) {}

ServerTLSContext::~ServerTLSContext() {
  if (_context) {
    SSL_CTX_free(_context);
    _context = NULL;
  }
  if (_subjectAltNames) {
    sk_GENERAL_NAME_pop_free(_subjectAltNames, GENERAL_NAME_free);
    _subjectAltNames = NULL;
  }
}

Error ServerTLSContext::commonName(char *buffer, UInt32 size) const {
  if (!buffer) {
    return ESB_NULL_POINTER;
  }

  if (!_context) {
    return ESB_INVALID_STATE;
  }

  X509 *cert = SSL_CTX_get0_certificate(_context);
  if (!cert) {
    return ESB_CANNOT_FIND;
  }

  X509_NAME *subjectName = X509_get_subject_name(cert);
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

Error ServerTLSContext::subjectAltName(char *buffer, UInt32 size, UInt32 *position) {
  if (!buffer || !position) {
    return ESB_NULL_POINTER;
  }

  if (!_subjectAltNames) {
    Error error = loadSubjectAltNames();
    if (ESB_SUCCESS != error) {
      return error;
    }
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

Error ServerTLSContext::loadSubjectAltNames() {
  if (_subjectAltNames) {
    return ESB_SUCCESS;
  }

  X509 *cert = SSL_CTX_get0_certificate(_context);
  _subjectAltNames = (STACK_OF(GENERAL_NAME) *)X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
  if (!_subjectAltNames) {
    return 0U;
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

ServerTLSContextIndex::ServerTLSContextIndex(UInt32 numBuckets, UInt32 numLocks, Allocator &allocator)
    : _contexts(numBuckets, numLocks, allocator),
      _deadContexts(),
      _defaultContext(NULL),
      _cleanupHandler(_deadContexts),
      _allocator(allocator) {}

ServerTLSContextIndex::~ServerTLSContextIndex() {
  _contexts.clear();
  for (EmbeddedListElement *e = _deadContexts.removeFirst(); e; e = _deadContexts.removeFirst()) {
    _allocator.deallocate(e);
  }
}

Error ServerTLSContextIndex::indexDefaultContext(const char *privateKeyPath, const char *certificatePath) {
  return indexContext(privateKeyPath, certificatePath, &_defaultContext);
}

void ServerTLSContextIndex::ServerTLSContextCleanupHandler::destroy(Object *object) {
  ServerTLSContext *context = (ServerTLSContext *)object;
  context->~ServerTLSContext();
  _deadContexts.addLast(context);
}

Error ServerTLSContextIndex::indexContext(const char *privateKeyPath, const char *certificatePath,
                                          ServerTLSContextPointer *out, bool maskSanConflicts) {
  if (!privateKeyPath || !certificatePath) {
    return ESB_NULL_POINTER;
  }

  ServerTLSContext *memory = (ServerTLSContext *)_deadContexts.removeLast();
  if (!memory) {
    memory = (ServerTLSContext *)_allocator.allocate(sizeof(ServerTLSContext));
    if (!memory) {
      return ESB_OUT_OF_MEMORY;
    }
  }

  ServerTLSContextPointer pointer;
  Error error = ServerTLSContext::Create(pointer, privateKeyPath, certificatePath, memory, &_cleanupHandler);
  if (ESB_SUCCESS != error) {
    return error;
  }

  if (out) {
    *out = pointer;
  }

  char buffer[ESB_MAX_HOSTNAME];
  const char *hostname = NULL;
  UInt32 hostnameSize = 0U;
  const char *domain = NULL;

  // If no SANs, index by common name

  if (0 == pointer->numSubjectAltNames()) {
    error = pointer->commonName(buffer, sizeof(buffer));
    if (ESB_SUCCESS != error) {
      return error;
    }

    SplitFqdn(buffer, &hostname, &hostnameSize, &domain);
    return _contexts.insert(domain, hostname, hostnameSize, pointer, maskSanConflicts);
  }

  // If SANs, index all SANs but not the common name

  UInt32 position = 0U;
  while (true) {
    error = pointer->subjectAltName(buffer, sizeof(buffer), &position);
    switch (error) {
      case ESB_CANNOT_FIND:
        return ESB_SUCCESS;
      case ESB_SUCCESS:
        SplitFqdn(buffer, &hostname, &hostnameSize, &domain);
        error = _contexts.insert(domain, hostname, hostnameSize, pointer, maskSanConflicts);
        if (ESB_SUCCESS != error) {
          return error;
        }
        break;
      default:
        return ESB_GENERAL_TLS_ERROR;
    }
  }
}

}  // namespace ESB
