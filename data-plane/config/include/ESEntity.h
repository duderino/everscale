#ifndef ES_ENTITY_H
#define ES_ENTITY_H

#ifndef ESB_UNIQUE_ID_H
#include <ESBUniqueId.h>
#endif

#ifndef ESB_EMBEDDED_MAP_ELEMENT_H
#include <ESBEmbeddedMapElement.h>
#endif

#ifndef ESB_AST_MAP_H
#include <ASTMap.h>
#endif

#ifndef ESB_TLS_CONTEXT_H
#include <ESBTLSContext.h>
#endif

namespace ES {

class Entity : public ESB::EmbeddedMapElement {
 public:
  enum Type {
    UNKNOWN,
    TLS_CTX,
    TLS_IDX,
    SEC_RULE,
    MAX_LIMIT,
    RATE_LIMIT,
    TOP_N_LIMIT,
    CLUSTER,
    CLEAR_PORT,
    TLS_PORT,
    INBOUND_CONNECTION_CIDR_MAP,
    INBOUND_CONNECTION_GEO_MAP,
    INBOUND_CONNECTION_RULE_LIST,
    INBOUND_CONNECTION_EXTENSION,
    INBOUND_CONNECTION_CLEANUP,
    INBOUND_REQUEST_PATH_MAP,
    INBOUND_REQUEST_FQDN_MAP,
    INBOUND_REQUEST_HEADER_MAP,
    INBOUND_REQUEST_RULE_LIST,
    INBOUND_REQUEST_EXTENSION,
    INBOUND_REQUEST_LOAD_BALANCER,
    INBOUND_REQUEST_CLEANUP,
    OUTBOUND_CONNECTION_CIDR_MAP,
    OUTBOUND_CONNECTION_RULE_LIST,
    OUTBOUND_CONNECTION_EXTENSION,
    OUTBOUND_CONNECTION_CLEANUP,
    OUTBOUND_REQUEST_RULE_LIST,
    OUTBOUND_REQUEST_EXTENSION,
    OUTBOUND_REQUEST_CLEANUP,
    OUTBOUND_RESPONSE_HEADER_MAP,
    OUTBOUND_RESPONSE_RULE_LIST,
    OUTBOUND_RESPONSE_EXTENSION,
    OUTBOUND_RESPONSE_CLEANUP,
    INBOUND_RESPONSE_RULE_LIST,
    INBOUND_RESPONSE_EXTENSION,
    INBOUND_RESPONSE_CLEANUP
  };

  static ESB::Error Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Entity **condition);

  virtual ~Entity();

  virtual ESB::CleanupHandler *cleanupHandler();
  virtual const void *key() const;

  virtual Type type() const = 0;

  inline const ESB::UniqueId &id() const { return _uuid; }

 protected:
  // Use Build()
  Entity(ESB::Allocator &allocator, ESB::UniqueId &uuid);

  ESB::UniqueId _uuid;
  ESB::Allocator &_allocator;

  ESB_DEFAULT_FUNCS(Entity);
};

class TLSContextEntity : public Entity {
 public:
  static ESB::Error Build(const ESB::AST::Map &map, ESB::Allocator &allocator, ESB::UniqueId &uuid, Entity **entity);

  virtual ~TLSContextEntity();

  virtual Type type() const;

  inline const char *keyPath() const { return _keyPath; }

  inline const char *certPath() const { return _certPath; }

  inline const char *caPath() const { return _caPath; }

  inline ESB::UInt32 certificateChainDepth() const { return _certificateChainDepth; }

  inline ESB::TLSContext::PeerVerification peerVerification() const { return _peerVerification; }

 private:
  // Use Build()
  TLSContextEntity(ESB::Allocator &allocator, ESB::UniqueId &uuid, char *keyPath, char *certPath, char *caPath,
                   ESB::UInt32 certificateChainDepth, ESB::TLSContext::PeerVerification peerVerification);

  char *_keyPath;
  char *_certPath;
  char *_caPath;
  ESB::UInt32 _certificateChainDepth;
  ESB::TLSContext::PeerVerification _peerVerification;

  ESB_DEFAULT_FUNCS(TLSContextEntity);
};

}  // namespace ES

#endif
