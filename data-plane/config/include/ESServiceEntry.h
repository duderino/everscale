#ifndef ES_SERVICE_ENTRY_H
#define ES_SERVICE_ENTRY_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ES {

class ServiceEntry {
 public:
  ServiceEntry();

  virtual ~ServiceEntry();

  ESB_DISABLE_AUTO_COPY(ServiceEntry);
};

}  // namespace ES

#endif
