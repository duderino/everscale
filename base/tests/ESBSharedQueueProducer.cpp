#ifndef ESB_SHARED_QUEUE_PRODUCER_H
#include <ESBSharedQueueProducer.h>
#endif

#ifndef ESTF_RAND_H
#include <ESTFRand.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

namespace ESB {

SharedQueueProducer::SharedQueueProducer(int id, SharedQueue &queue, UInt32 items)
    : _id(id), _items(items), _queue(queue) {}

SharedQueueProducer::~SharedQueueProducer() {}

bool SharedQueueProducer::run(ESTF::ResultCollector *collector) {
  UInt32 i = 0;
  Error error;
  ESTF::Rand rand;

  while (i < _items) {
    if (1 == rand.generateRandom(1, 2)) {
      ESTF_ASSERT(collector, ESB_SUCCESS == _queue.push(&_id));
      ++i;
      continue;
    }

    error = _queue.tryPush(&_id);

    ESTF_ASSERT(collector, ESB_SUCCESS == error || ESB_AGAIN == error);

    if (ESB_SUCCESS == error) {
      ++i;
    }
  }

  return true;
}

bool SharedQueueProducer::setup() { return true; }

bool SharedQueueProducer::tearDown() { return true; }

ESTF::ComponentPtr SharedQueueProducer::clone() {
  ESTF::ComponentPtr component(new SharedQueueProducer(_id, _queue, _items));

  return component;
}

}  // namespace ESB
