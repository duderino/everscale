#ifndef ESB_SHARED_EMBEDDED_LIST_H
#include <ESBSharedEmbeddedList.h>
#endif

namespace ESB {

SharedEmbeddedList::SharedEmbeddedList() : _list(), _lock() {}

SharedEmbeddedList::~SharedEmbeddedList() {}

}  // namespace ESB
