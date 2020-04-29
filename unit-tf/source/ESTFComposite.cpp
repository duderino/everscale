#ifndef ESTF_COMPOSITE_H
#include "ESTFComposite.h"
#endif

namespace ESTF {

Composite::Composite() : Component(), _children() {}

Composite::~Composite() { _children.clear(); }

bool Composite::run(ResultCollector *collector) {
  std::list<ComponentPtr>::iterator it = _children.begin();
  std::list<ComponentPtr>::iterator end = _children.end();

  for (; it != end; ++it) {
    if (true != (*it)->run(collector)) {
      return false;
    }
  }

  return true;
}

bool Composite::setup() {
  std::list<ComponentPtr>::iterator it = _children.begin();
  std::list<ComponentPtr>::iterator end = _children.end();

  for (; it != end; ++it) {
    if (true != (*it)->setup()) {
      return false;
    }
  }

  return true;
}

bool Composite::tearDown() {
  std::list<ComponentPtr>::iterator it = _children.begin();
  std::list<ComponentPtr>::iterator end = _children.end();

  for (; it != end; ++it) {
    if (true != (*it)->tearDown()) {
      return false;
    }
  }

  return true;
}

ComponentPtr Composite::clone() {
  std::list<ComponentPtr>::iterator it = _children.begin();
  std::list<ComponentPtr>::iterator end = _children.end();
  CompositePtr composite = new Composite();
  ComponentPtr object;

  if (composite.isNull()) {
    assert(0 == "alloc failed");
    return composite;
  }

  for (; it != end; ++it) {
    object = (*it)->clone();

    composite->add(object);
  }

  return composite;
}

void Composite::add(ComponentPtr &component) { _children.push_back(component); }

void Composite::remove(const ComponentPtr &component) {
  std::list<ComponentPtr>::iterator it = _children.begin();
  std::list<ComponentPtr>::iterator end = _children.end();

  for (; it != end; ++it) {
    if ((*it) == component) {
      _children.erase(it);
      return;
    }
  }
}

void Composite::clear() { _children.clear(); }

int Composite::size() { return _children.size(); }

}  // namespace ESTF
