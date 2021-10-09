#ifndef ES_TLS_CONTEXT_INDEX_CONFIG_H
#include <ESTLSContextIndexConfig.h>
#endif

namespace ES {

ESB::Error TLSContextIndexConfig::Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Entity **entity) {
  return ESB_NOT_IMPLEMENTED;
}

TLSContextIndexConfig::TLSContextIndexConfig(ESB::Allocator &allocator, ESB::UniqueId &uuid)
    : Entity(allocator, uuid) {}

TLSContextIndexConfig::~TLSContextIndexConfig() {}

Entity::Type TLSContextIndexConfig::type() const { return Entity::TLS_IDX; }

}  // namespace ES
