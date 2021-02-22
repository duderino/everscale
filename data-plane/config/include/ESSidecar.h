#ifndef ES_SIDECAR_H
#define ES_SIDECAR_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ES {

class Sidecar {
 public:
  Sidecar();

  virtual ~Sidecar();

  ESB_DISABLE_AUTO_COPY(Sidecar);
};

}  // namespace ES

#endif
