#ifndef ES_CONFIG_H
#define ES_CONFIG_H

#ifndef ESB_BUDDY_CACHE_ALLOCATOR_H
#include <ESBBuddyCacheAllocator.h>
#endif

namespace ES {

class Config {
 public:
  Config();

  virtual ~Config();

  ESB_DEFAULT_FUNCS(Config);
};

}  // namespace ES

#endif
