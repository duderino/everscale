#ifndef ESTF_COMPONENT_THREAD_H
#include <ESTFComponentThread.h>
#endif

namespace ESTF {

ComponentThread::ComponentThread() : _component(), _collector(0), _result(false) {}

ComponentThread::~ComponentThread() {}

void ComponentThread::setComponent(ComponentPtr &component) { _component = component; }

void ComponentThread::setCollector(ResultCollector *collector) { _collector = collector; }

bool ComponentThread::getResult() { return _result; }

ResultCollector *ComponentThread::getCollector() { return _collector; }

void ComponentThread::run() { _result = _component->run(_collector); }
}  // namespace ESTF
