#ifndef ES_TLS_CONTEXT_INDEX_CONFIG_H
#define ES_TLS_CONTEXT_INDEX_CONFIG_H

#ifndef ES_ENTITY_H
#include <ESEntity.h>
#endif

namespace ES {

class TLSContextIndexConfig : public Entity {
 public:
  static ESB::Error Build(const ESB::AST::Map &map, ESB::Allocator &allocator, Entity **entity);

  virtual ~TLSContextIndexConfig();

  virtual Type type() const;

 private:
  // Use Build()
  TLSContextIndexConfig(ESB::Allocator &allocator, ESB::UniqueId &uuid);

  ESB_DEFAULT_FUNCS(TLSContextIndexConfig);
};

}  // namespace ES

#endif
