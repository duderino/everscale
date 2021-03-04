#ifndef ESB_SMART_POINTER_H
#include <ESBSmartPointer.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#include <gtest/gtest.h>

namespace ESB {
class TestCleanupHandler : public CleanupHandler {
 public:
  TestCleanupHandler() : _calls(0) {}
  virtual ~TestCleanupHandler() {}

  virtual void destroy(Object* object) {
    ++_calls;
    object->~Object();
    SystemAllocator::Instance().deallocate(object);
  }

  inline int calls() const { return _calls; }

 private:
  int _calls;

  ESB_DISABLE_AUTO_COPY(TestCleanupHandler);
};

static TestCleanupHandler TestCleanupHandler;
static int Destructions = 0;

class TestObject : public ReferenceCount {
 public:
  TestObject() {}
  virtual ~TestObject() { Destructions++; }

  inline bool test() { return true; }

  virtual CleanupHandler* cleanupHandler() { return &TestCleanupHandler; }

  ESB_DISABLE_AUTO_COPY(TestObject);
};

ESB_SMART_POINTER(TestObject, TestObjectPointer, SmartPointer);
}  // namespace ESB

using namespace ESB;

TEST(SmartPointer, SharedInt) {
  SharedInt si(0);
  EXPECT_EQ(1, si.inc());
  EXPECT_EQ(1, si.get());
  EXPECT_EQ(0, si.dec());
  EXPECT_EQ(0, si.get());
}

TEST(SmartPointer, Null) {
  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  SmartPointer ptr = new (SystemAllocator::Instance()) TestObject();
  ptr = NULL;

  EXPECT_EQ(cleanups + 1, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 1, Destructions);
}

TEST(SmartPointer, Descope) {
  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  { SmartPointer ptr = new (SystemAllocator::Instance()) TestObject(); }

  EXPECT_EQ(cleanups + 1, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 1, Destructions);
}

TEST(SmartPointer, Clobber) {
  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  SmartPointer ptr = new (SystemAllocator::Instance()) TestObject();
  ptr = new (SystemAllocator::Instance()) TestObject();

  EXPECT_EQ(cleanups + 1, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 1, Destructions);
}

TEST(SmartPointer, IncIncDecDec) {
  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  SmartPointer ptr = new (SystemAllocator::Instance()) TestObject();
  SmartPointer ptr2 = ptr;

  ptr = NULL;
  EXPECT_EQ(cleanups, TestCleanupHandler.calls());
  EXPECT_EQ(destructions, Destructions);

  ptr2 = NULL;
  EXPECT_EQ(cleanups + 1, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 1, Destructions);
}

TEST(SmartPointer, Subclass) {
  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  TestObjectPointer ptr;
  EXPECT_TRUE(ptr.isNull());

  ptr = new (SystemAllocator::Instance()) TestObject();
  EXPECT_TRUE(ptr->test());
  EXPECT_TRUE((*ptr).test());
  EXPECT_FALSE(ptr.isNull());

  TestObjectPointer ptr2(ptr);
  EXPECT_TRUE(ptr2->test());
  EXPECT_TRUE((*ptr2).test());
  EXPECT_FALSE(ptr2.isNull());

  ptr = NULL;
  EXPECT_EQ(cleanups, TestCleanupHandler.calls());
  EXPECT_EQ(destructions, Destructions);

  ptr2 = NULL;
  EXPECT_EQ(cleanups + 1, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 1, Destructions);
}
