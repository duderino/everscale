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

ESB::Error Entity::Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Entity **condition) {
  if (!condition) {
    return ESB_NULL_POINTER;
  }

  ESB::UInt128 id = 0;

  {
    const char *str = NULL;
    ESB::Error error = map.find("id", &str);
    if (ESB_SUCCESS != error) {
      return error;
    }

    error = ESB::UniqueId::Parse(str, &id);
    if (ESB_SUCCESS != error) {
      return ESB_INVALID_FIELD;
    }
  }

  Type type = UNKNOWN;

  {
    const char *str = NULL;
    ESB::Error error = map.find("type", &str);
    if (ESB_SUCCESS != error) {
      return error;
    }

    type = ParseEntityType(str);
    if (UNKNOWN == type) {
      return ESB_INVALID_FIELD;
    }
  }

  switch (type) {
    default:
      return ESB_NOT_IMPLEMENTED;
  }
}

}  // namespace ES
