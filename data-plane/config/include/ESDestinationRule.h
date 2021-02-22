#ifndef ES_DESTINATION_RULE_H
#define ES_DESTINATION_RULE_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ES {

class DestinationRule {
 public:
  DestinationRule();

  virtual ~DestinationRule();

  ESB_DISABLE_AUTO_COPY(DestinationRule);
};

}  // namespace ES

#endif
