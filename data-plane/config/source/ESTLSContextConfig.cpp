#ifndef ES_TLS_CONTEXT_CONFIG_H
#include <ESTLSContextConfig.h>
#endif

namespace ES {

TLSContextConfig::TLSContextConfig(ESB::Allocator &allocator, ESB::UniqueId &uuid, char *keyPath, char *certPath,
                                   char *caPath, ESB::UInt32 certificateChainDepth,
                                   ESB::TLSContext::PeerVerification peerVerification)
    : Entity(allocator, uuid),
      _keyPath(keyPath),
      _certPath(certPath),
      _caPath(caPath),
      _certificateChainDepth(certificateChainDepth),
      _peerVerification(peerVerification) {}

TLSContextConfig::~TLSContextConfig() {
  if (_keyPath) {
    _allocator.deallocate(_keyPath);
    _keyPath = NULL;
  }

  if (_certPath) {
    _allocator.deallocate(_certPath);
    _certPath = NULL;
  }

  if (_caPath) {
    _allocator.deallocate(_caPath);
    _caPath = NULL;
  }
}

Entity::Type TLSContextConfig::type() const { return Entity::TLS_CTX; }

ESB::Error TLSContextConfig::Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Entity **entity) {
  return ESB_NOT_IMPLEMENTED;
}

}  // namespace ES
