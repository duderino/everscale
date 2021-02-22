#ifndef ES_CONFIG_H
#define ES_CONFIG_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ES {

class Config {
 public:
  Config();

  virtual ~Config();

  ESB_DISABLE_AUTO_COPY(Config);
};

}  // namespace ES

#endif
