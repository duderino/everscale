#ifndef ES_VIRTUAL_SERVICE_H
#define ES_VIRTUAL_SERVICE_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ES {

class VirtualService {
 public:
  VirtualService();

  virtual ~VirtualService();

  ESB_DISABLE_AUTO_COPY(VirtualService);
};

}  // namespace ES

#endif
