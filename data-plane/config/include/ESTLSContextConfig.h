#ifndef ES_TLS_CONTEXT_CONFIG_H
#define ES_TLS_CONTEXT_CONFIG_H

#ifndef ES_ENTITY_H
#include <ESEntity.h>
#endif

#ifndef ESB_TLS_CONTEXT_H
#include <ESBTLSContext.h>
#endif

namespace ES {

class TLSContextConfig : public Entity {
 public:
  static ESB::Error Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Entity **entity);

  virtual ~TLSContextConfig();

  virtual Type type() const;

 private:
  // Use Build()
  TLSContextConfig(ESB::Allocator &allocator, ESB::UniqueId &uuid, char *keyPath, char *certPath, char *caPath,
                   ESB::UInt32 certificateChainDepth, ESB::TLSContext::PeerVerification peerVerification);

  char *_keyPath;
  char *_certPath;
  char *_caPath;
  ESB::UInt32 _certificateChainDepth;
  ESB::TLSContext::PeerVerification _peerVerification;

  ESB_DEFAULT_FUNCS(TLSContextConfig);
};

}  // namespace ES

#endif
