#ifndef ESB_CLIENT_TLS_CONTEXT_INDEX_H
#include <ESBClientTLSContextIndex.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ESB {

ClientTLSContextIndex::ClientTLSContextIndex(UInt32 numBuckets, UInt32 numLocks, Allocator &allocator)
    : TLSContextIndex(numBuckets, numLocks, allocator), _defaultContext(NULL) {}

ClientTLSContextIndex::~ClientTLSContextIndex() {}

Error ClientTLSContextIndex::indexDefaultContext(const TLSContext::Params &params) {
  if (!_defaultContext.isNull()) {
    return ESB_UNIQUENESS_VIOLATION;
  }

  Error error = indexContext(params, &_defaultContext);
  switch (error) {
    case ESB_CANNOT_CONVERT:
      // OK, it means the default client context has no client certificate
    case ESB_SUCCESS:
      return ESB_SUCCESS;
    default:
      return error;
  }
}

void ClientTLSContextIndex::clear() {
  TLSContextIndex::clear();
  _defaultContext = NULL;
}

}  // namespace ESB
