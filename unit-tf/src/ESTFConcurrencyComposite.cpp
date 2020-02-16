#ifndef ESTF_CONCURRENCY_COMPOSITE_H
#include <ESTFConcurrencyComposite.h>
#endif

#ifndef ESTF_COMPONENT_THREAD_H
#include <ESTFComponentThread.h>
#endif

namespace ESTF {

ConcurrencyComposite::ConcurrencyComposite() : Composite() {}

ConcurrencyComposite::~ConcurrencyComposite() {}

bool ConcurrencyComposite::run(ResultCollector *collector) {
  int numThreads = _children.size();
  bool result = true;
  ResultCollector *collectors = 0;
  ComponentThread *threads = 0;
  std::list<ComponentPtr>::iterator it = _children.begin();
  std::list<ComponentPtr>::iterator end = _children.end();

  collectors = new ResultCollector[numThreads];

  if (!collectors) {
    return false;
  }

  threads = new ComponentThread[numThreads];

  if (!threads) {
    delete[] collectors;
    return false;
  }

  for (int i = 0; i < numThreads && it != end; ++i, ++it) {
    threads[i].setComponent(*it);
    threads[i].setCollector(&collectors[i]);
  }

  for (int i = 0; i < numThreads; ++i) {
    threads[i].spawn();
  }

  for (int i = 0; i < numThreads; ++i) {
    threads[i].join();
  }

  for (int i = 0; i < numThreads; ++i) {
    if (true == threads[i].getResult()) {
      collector->merge(threads[i].getCollector());
    } else {
      result = false;
    }
  }

  delete[] collectors;
  delete[] threads;

  return result;
}

ComponentPtr ConcurrencyComposite::clone() {
  //
  //  could replace the new ConcurrencyComposite with a template
  //  method and just inherit the rest of this method...
  //
  std::list<ComponentPtr>::iterator it = _children.begin();
  std::list<ComponentPtr>::iterator end = _children.end();
  CompositePtr composite = new ConcurrencyComposite();
  ComponentPtr object;

  if (composite.isNull()) {
    ESTF_NATIVE_ASSERT(0 == "alloc failed");
    return composite;
  }

  for (; it != end; ++it) {
    object = (*it)->clone();

    composite->add(object);
  }

  return composite;
}

}  // namespace ESTF
