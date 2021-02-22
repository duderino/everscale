#ifndef ES_GATEWAY_H
#define ES_GATEWAY_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ES {

class Gateway {
 public:
  Gateway();

  virtual ~Gateway();

  ESB_DISABLE_AUTO_COPY(Gateway);
};

}  // namespace ES

#endif
