#ifndef ES_RULE_H
#include <ESRule.h>
#endif

namespace ES {

Rule::Rule(ESB::Allocator &allocator) : _allocator(allocator) {}

Rule::~Rule() {}

ESB::CleanupHandler *Rule::cleanupHandler() { return &_allocator.cleanupHandler(); }

}  // namespace ES
