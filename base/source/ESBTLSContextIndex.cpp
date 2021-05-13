#ifndef ESB_TLS_CONTEXT_INDEX_H
#include <ESBTLSContextIndex.h>
#endif

namespace ESB {

TLSContextIndex::TLSContextIndex(UInt32 numBuckets, UInt32 numLocks, Allocator &allocator)
    : _contexts(numBuckets, numLocks, allocator),
      _deadContexts(),
      _cleanupHandler(_deadContexts),
      _allocator(allocator) {}

TLSContextIndex::~TLSContextIndex() {
  _contexts.clear();
  for (EmbeddedListElement *e = _deadContexts.removeFirst(); e; e = _deadContexts.removeFirst()) {
    _allocator.deallocate(e);
  }
}

void TLSContextIndex::TLSContextCleanupHandler::destroy(Object *object) {
  TLSContext *context = (TLSContext *)object;
  context->~TLSContext();
  _deadContexts.addLast(context);
}

Error TLSContextIndex::indexContext(const TLSContext::Params &params, TLSContextPointer *out, bool maskSanConflicts) {
  TLSContext *memory = (TLSContext *)_deadContexts.removeLast();
  if (!memory) {
    memory = (TLSContext *)_allocator.allocate(sizeof(TLSContext));
    if (!memory) {
      return ESB_OUT_OF_MEMORY;
    }
  }

  TLSContextPointer pointer;
  Error error = TLSContext::Create(pointer, params, memory, &_cleanupHandler);
  if (ESB_SUCCESS != error) {
    _deadContexts.addLast(memory);
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

  if (0 == pointer->certificate().numSubjectAltNames()) {
    error = pointer->certificate().commonName(buffer, sizeof(buffer));
    switch (error) {
      case ESB_CANNOT_FIND:
        // most likely the context has no certificate
        return ESB_CANNOT_CONVERT;
      case ESB_SUCCESS:
        break;
      default:
        return error;
    }

    SplitFqdn(buffer, &hostname, &hostnameSize, &domain);
    error = _contexts.insert(domain, hostname, hostnameSize, pointer, maskSanConflicts);
    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  // If SANs, index all SANs but not the common name

  UInt32 position = 0U;
  while (true) {
    error = pointer->certificate().subjectAltName(buffer, sizeof(buffer), &position);
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

Error TLSContextIndex::matchContext(const char *fqdn, TLSContextPointer &pointer) {
  const char *hostname = NULL;
  UInt32 hostnameSize = 0U;
  const char *domain = NULL;

  SplitFqdn(fqdn, &hostname, &hostnameSize, &domain);
  if (!domain || !*domain || !hostname || !*hostname || 0 == hostnameSize) {
    return ESB_CANNOT_FIND;
  }

  return _contexts.match(domain, hostname, hostnameSize, pointer);
}

void TLSContextIndex::clear() { _contexts.clear(); }

}  // namespace ESB
