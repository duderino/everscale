#ifndef ESB_SERVER_TLS_CONTEXT_INDEX_H
#include <ESBServerTLSContextIndex.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ESB {

ServerTLSContextIndex::ServerTLSContextIndex(UInt32 numBuckets, UInt32 numLocks, Allocator &allocator)
    : TLSContextIndex(numBuckets, numLocks, allocator), _defaultContext(NULL) {}

ServerTLSContextIndex::~ServerTLSContextIndex() {}

/**
 * This callback is invoked for every server-side handshake and can swap in a more specific match for any presented SNI.
 */
static int tlsext_servername_callback(SSL *ssl, int *ad, void *arg) {
  assert(ssl);
  assert(arg);
  assert(ad);
  if (!ssl || !arg || !ad) {
    ESB_LOG_ERROR_ERRNO(ESB_INVALID_STATE, "SNI callback not passed all arguments");
    return SSL_TLSEXT_ERR_NOACK;
  }

  // Get the SNI.  If no SNI just use the default cert/context
  const char *servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
  if (!servername || !*servername) {
    ESB_LOG_DEBUG("No SNI received, using default cert");
    return SSL_TLSEXT_ERR_NOACK;
  }

  // Look for more specific matches for the SNI
  ServerTLSContextIndex *index = (ServerTLSContextIndex *)arg;
  TLSContextPointer match;
  Error error = index->matchContext(servername, match);
  if (ESB_SUCCESS != error || match.isNull()) {
    ESB_LOG_INFO_ERRNO(error, "SNI callback cannot match SNI %s", servername);
    *ad = SSL_AD_UNRECOGNIZED_NAME;
    return SSL_TLSEXT_ERR_ALERT_FATAL;
  }

  // The default cert/context is the best match for the SNI, so use it.
  if (match->rawContext() == SSL_get_SSL_CTX(ssl)) {
    ESB_LOG_DEBUG("SNI callback reused default cert for SNI %s", servername);
    return SSL_TLSEXT_ERR_OK;
  }

  //
  // Swap in a better cert/context for the SNI.  Note that SSL_CTX has its own internal reference count.  Even if
  // SSL_CTX_free() is called on it, it will remain valid until the last SSL * that uses it is freed.  So calling
  // SSL_CTX_free() on the SSL_CTX * only means that it cannot be used for new connections.
  //

  if (!SSL_set_SSL_CTX(ssl, match->rawContext())) {
    ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "SNI callback cannot replace context for SNI %s", servername);
    *ad = SSL_AD_INTERNAL_ERROR;
    return SSL_TLSEXT_ERR_ALERT_FATAL;
  }

  if (match->verifyPeerCertificate()) {
    SSL_set_verify(ssl, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
  }

  return SSL_TLSEXT_ERR_OK;
}

Error ServerTLSContextIndex::indexDefaultContext(const TLSContext::Params &params) {
  if (!_defaultContext.isNull()) {
    return ESB_UNIQUENESS_VIOLATION;
  }

  Error error = indexContext(params, &_defaultContext);
  if (ESB_SUCCESS != error) {
    return error;
  }

  // Set callback to be invoked when an SNI is received.  This callback will match the SNI against the index and
  // potentially swap in a more specific cert.  Both of these should never fail.
  if (0 >= SSL_CTX_set_tlsext_servername_callback(_defaultContext->rawContext(), tlsext_servername_callback)) {
    return ESB_GENERAL_TLS_ERROR;
  }

  if (0 >= SSL_CTX_set_tlsext_servername_arg(_defaultContext->rawContext(), this)) {
    return ESB_GENERAL_TLS_ERROR;
  }

  return ESB_SUCCESS;
}

void ServerTLSContextIndex::clear() {
  TLSContextIndex::clear();
  _defaultContext = NULL;
}

}  // namespace ESB
