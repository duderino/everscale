#ifndef ESB_SHARED_QUEUE_CONSUMER_H
#include <ESBSharedQueueConsumer.h>
#endif

#ifndef ESTF_RAND_H
#include <ESTFRand.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

namespace ESB {

SharedQueueConsumer::SharedQueueConsumer(SharedQueue &queue, UInt32 items)
    : _items(items), _queue(queue) {}

SharedQueueConsumer::~SharedQueueConsumer() {}

bool SharedQueueConsumer::run(ESTF::ResultCollector *collector) {
  int *item = 0;
  UInt32 i = 0;
  Error error;
  ESTF::Rand rand;

  while (i < _items) {
    item = 0;

    if (1 == rand.generateRandom(1, 2)) {
      ESTF_ASSERT(collector, ESB_SUCCESS == _queue.pop((void **)&item));
      ESTF_ASSERT(collector, item);

      if (item) ++i;

      continue;
    }

    error = _queue.tryPop((void **)&item);

    ESTF_ASSERT(collector, ESB_SUCCESS == error || ESB_AGAIN == error);

    if (ESB_SUCCESS == error) {
      ESTF_ASSERT(collector, item);

      if (item) ++i;
    }
  }

  return true;
}

bool SharedQueueConsumer::setup() { return true; }

bool SharedQueueConsumer::tearDown() { return true; }

ESTF::ComponentPtr SharedQueueConsumer::clone() {
  ESTF::ComponentPtr component(new SharedQueueConsumer(_queue, _items));

  return component;
}

}  // namespace ESB
