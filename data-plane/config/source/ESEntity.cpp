#ifndef ES_ENTITY_H
#include <ESEntity.h>
#endif

namespace ES {

// Must be in same order as Entity::Type
static const char *EntityTypeStrings[] = {"",
                                          "TLS_CTX",
                                          "TLS_IDX",
                                          "SEC_RULE",
                                          "MAX_LIMIT",
                                          "RATE_LIMIT",
                                          "TOP_N_LIMIT",
                                          "CLUSTER",
                                          "CLEAR_PORT",
                                          "TLS_PORT",
                                          "INBOUND_CONNECTION_CIDR_MAP",
                                          "INBOUND_CONNECTION_GEO_MAP",
                                          "INBOUND_CONNECTION_RULE_LIST",
                                          "INBOUND_CONNECTION_EXTENSION",
                                          "INBOUND_CONNECTION_CLEANUP",
                                          "INBOUND_REQUEST_PATH_MAP",
                                          "INBOUND_REQUEST_FQDN_MAP",
                                          "INBOUND_REQUEST_HEADER_MAP",
                                          "INBOUND_REQUEST_RULE_LIST",
                                          "INBOUND_REQUEST_EXTENSION",
                                          "INBOUND_REQUEST_LOAD_BALANCER",
                                          "INBOUND_REQUEST_CLEANUP",
                                          "OUTBOUND_CONNECTION_CIDR_MAP",
                                          "OUTBOUND_CONNECTION_RULE_LIST",
                                          "OUTBOUND_CONNECTION_EXTENSION",
                                          "OUTBOUND_CONNECTION_CLEANUP",
                                          "OUTBOUND_REQUEST_RULE_LIST",
                                          "OUTBOUND_REQUEST_EXTENSION",
                                          "OUTBOUND_REQUEST_CLEANUP",
                                          "OUTBOUND_RESPONSE_HEADER_MAP",
                                          "OUTBOUND_RESPONSE_RULE_LIST",
                                          "OUTBOUND_RESPONSE_EXTENSION",
                                          "OUTBOUND_RESPONSE_CLEANUP",
                                          "INBOUND_RESPONSE_RULE_LIST",
                                          "INBOUND_RESPONSE_EXTENSION",
                                          "INBOUND_RESPONSE_CLEANUP"};

static Entity::Type ParseEntityType(const char *str) {
  if (!str) {
    return Entity::UNKNOWN;
  }

  for (int i = 0; i < sizeof(EntityTypeStrings) / sizeof(char *); ++i) {
    if (0 == strcasecmp(str, EntityTypeStrings[i])) {
      return (Entity::Type)i;
    }
  }

  return Entity::UNKNOWN;
}

Entity::Entity(ESB::Allocator &allocator, ESB::UniqueId &uuid) : _uuid(uuid), _allocator(allocator) {}

Entity::~Entity() {}

ESB::CleanupHandler *Entity::cleanupHandler() { return &_allocator.cleanupHandler(); }

const void *Entity::key() const { return &_uuid; }

ESB::Error Entity::Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Entity **entity) {
  if (!entity) {
    return ESB_NULL_POINTER;
  }

  ESB::UniqueId id = 0;
  ESB::Error error = map.find("id", id);
  if (ESB_SUCCESS != error) {
    return error;
  }

  Type type = UNKNOWN;

  {
    const char *str = NULL;
    error = map.find("type", &str);
    if (ESB_SUCCESS != error) {
      return error;
    }

    type = ParseEntityType(str);
    if (UNKNOWN == type) {
      return ESB_INVALID_FIELD;
    }
  }

  switch (type) {
    case TLS_CTX:
      return TLSContextEntity::Build(map, allocator, id, entity);
    case TLS_IDX:
      return TLSContextIndexEntity::Build(map, allocator, id, entity);
    default:
      return ESB_NOT_IMPLEMENTED;
  }
}

TLSContextEntity::TLSContextEntity(ESB::Allocator &allocator, ESB::UniqueId &uuid, char *keyPath, char *certPath,
                                   char *caPath, ESB::UInt32 certificateChainDepth,
                                   ESB::TLSContext::PeerVerification peerVerification)
    : Entity(allocator, uuid),
      _keyPath(keyPath),
      _certPath(certPath),
      _caPath(caPath),
      _certificateChainDepth(certificateChainDepth),
      _peerVerification(peerVerification) {}

TLSContextEntity::~TLSContextEntity() {
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

Entity::Type TLSContextEntity::type() const { return Entity::TLS_CTX; }

ESB::Error TLSContextEntity::Build(const ESB::AST::Map &map, ESB::Allocator &allocator, ESB::UniqueId &uuid,
                                   Entity **entity) {
  char *keyPath = NULL, *certPath = NULL, *caPath = NULL;
  ESB::UInt32 certificateChainDepth = ESB::TLSContext::DefaultCertificateChainDepth;

  ESB::Error error = map.find("certificate_chain_depth", &certificateChainDepth, true);
  if (ESB_SUCCESS != error) {
    return error;
  }

  ESB::TLSContext::PeerVerification peerVerification = ESB::TLSContext::DefaultPeerVerification;

  {
    const char *str = NULL;
    switch (error = map.find("peer_verification", &str)) {
      case ESB_SUCCESS:
        if (0 == strcasecmp("VERIFY_ALWAYS", str)) {
          peerVerification = ESB::TLSContext::VERIFY_ALWAYS;
        } else if (0 == strcasecmp("VERIFY_NONE", str)) {
          peerVerification = ESB::TLSContext::VERIFY_NONE;
        } else if (0 == strcasecmp("VERIFY_IF_CERT", str)) {
          peerVerification = ESB::TLSContext::VERIFY_IF_CERT;
        } else {
          return ESB_INVALID_FIELD;
        }
      case ESB_MISSING_FIELD:
        break;
      default:
        return error;
    }
  }

  error = map.findAndDuplicate(allocator, "key_path", &keyPath, true);
  if (ESB_SUCCESS != error) {
    return error;
  }

  error = map.findAndDuplicate(allocator, "cert_path", &certPath, true);
  if (ESB_SUCCESS != error) {
    if (keyPath) allocator.deallocate(keyPath);
    return error;
  }

  error = map.findAndDuplicate(allocator, "ca_path", &caPath, true);
  if (ESB_SUCCESS != error) {
    if (keyPath) allocator.deallocate(keyPath);
    if (certPath) allocator.deallocate(certPath);
    return error;
  }

  if ((keyPath && !certPath) || (!keyPath && certPath) || (!keyPath && !certPath && !caPath)) {
    if (keyPath) allocator.deallocate(keyPath);
    if (certPath) allocator.deallocate(certPath);
    if (caPath) allocator.deallocate(caPath);
    return ESB_MISSING_FIELD;
  }

  TLSContextEntity *config = new (allocator)
      TLSContextEntity(allocator, uuid, keyPath, certPath, caPath, certificateChainDepth, peerVerification);

  if (!config) {
    if (keyPath) allocator.deallocate(keyPath);
    if (certPath) allocator.deallocate(certPath);
    if (caPath) allocator.deallocate(caPath);
    return ESB_OUT_OF_MEMORY;
  }

  *entity = config;
  return ESB_SUCCESS;
}

TLSContextIndexEntity::TLSContextIndexEntity(ESB::Allocator &allocator, ESB::UniqueId &id,
                                             ESB::UniqueId &defaultContext, ESB::UniqueId *contexts,
                                             ESB::UInt32 numContexts)
    : Entity(allocator, id), _defaultContext(defaultContext), _contexts(contexts), _numContexts(numContexts) {}

TLSContextIndexEntity::~TLSContextIndexEntity() {
  if (_contexts) {
    _allocator.deallocate(_contexts);
    _contexts = NULL;
  }
}

ESB::Error TLSContextIndexEntity::Build(const ESB::AST::Map &map, ESB::Allocator &allocator, ESB::UniqueId &id,
                                        Entity **entity) {
  const ESB::AST::List *contexts = NULL;
  ESB::Error error = map.find("contexts", &contexts);
  switch (error) {
    case ESB_SUCCESS:
    case ESB_MISSING_FIELD:
      break;
    default:
      return error;
  }

  ESB::UInt32 numContexts = 0;
  if (contexts) {
    numContexts = contexts->size();
  }

  ESB::UniqueId defaultContext;
  error = map.find("default_context", defaultContext);
  if (ESB_SUCCESS != error) {
    return error;
  }

  ESB::UniqueId *contextsCopy = NULL;
  if (0 < numContexts) {
    error = allocator.allocate(sizeof(ESB::UniqueId) * numContexts, (void **)&contextsCopy);
    if (ESB_SUCCESS != error) {
      return error;
    }

    const ESB::AST::Element *current = contexts->first();
    for (ESB::UInt32 i = 0; i < numContexts; ++i) {
      if (ESB::AST::Element::STRING != current->type()) {
        return ESB_INVALID_FIELD;
      }
      ESB::AST::String *str = (ESB::AST::String *)current;

      ESB::UniqueId contextId;
      error = ESB::UniqueId::Parse(str->value(), contextId);
      if (ESB_SUCCESS != error) {
        return ESB_INVALID_FIELD;
      }
      contextsCopy[i] = contextId;

      current = (ESB::AST::Element *)current->next();
    }
  }

  TLSContextIndexEntity *idx =
      new (allocator) TLSContextIndexEntity(allocator, id, defaultContext, contextsCopy, numContexts);
  if (ESB_SUCCESS != error) {
    if (contextsCopy) {
      allocator.deallocate(contextsCopy);
    }
    return error;
  }

  *entity = idx;
  return ESB_SUCCESS;
}

Entity::Type TLSContextIndexEntity::type() const { return Entity::TLS_IDX; }

}  // namespace ES
