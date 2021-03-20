#ifndef ESB_WILDCARD_INDEX_H
#include <ESBWildcardIndex.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#include <gtest/gtest.h>

namespace ESB {
class TestCleanupHandler : public CleanupHandler {
 public:
  TestCleanupHandler() : _calls(0) {}
  virtual ~TestCleanupHandler() {}

  virtual void destroy(Object *object) {
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
  TestObject(int value) : _value(value) {}
  virtual ~TestObject() { Destructions++; }

  inline int value() { return _value; }

  virtual CleanupHandler *cleanupHandler() { return &TestCleanupHandler; }

 private:
  int _value;

  ESB_DISABLE_AUTO_COPY(TestObject);
};

ESB_SMART_POINTER(TestObject, TestObjectPointer, SmartPointer);
}  // namespace ESB

using namespace ESB;

TEST(WildcardIndexNodeTest, Find) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));
  }

  {
    TestObjectPointer ptr;
    EXPECT_EQ(ESB_SUCCESS, node->find("foo", 3, ptr));
    EXPECT_EQ(ptr->value(), 1);

    EXPECT_EQ(ESB_SUCCESS, node->find("bar", 3, ptr));
    EXPECT_EQ(ptr->value(), 2);

    EXPECT_EQ(ESB_SUCCESS, node->find("baz", 3, ptr));
    EXPECT_EQ(ptr->value(), 3);

    EXPECT_EQ(cleanups, TestCleanupHandler.calls());
    EXPECT_EQ(destructions, Destructions);
  }

  delete node;

  EXPECT_EQ(cleanups + 3, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 3, Destructions);
}

TEST(WildcardIndexNodeTest, EnforceUnique) {
  char key[ESB_MAX_HOSTNAME + 1];
  memset(key, 'a', sizeof(key));
  key[ESB_MAX_HOSTNAME] = 0;
  WildcardIndexNode *node = WildcardIndexNode::Create(key);

  char wildcard[ESB_UINT8_MAX];
  memset(wildcard, 'b', sizeof(wildcard));

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);
    SmartPointer qux = new (SystemAllocator::Instance()) TestObject(4);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert(wildcard, sizeof(wildcard), baz));
    EXPECT_EQ(ESB_SUCCESS, node->insert("qux", 3, qux));

    EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, node->insert(wildcard, sizeof(wildcard), baz));
    EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, node->insert("qux", 3, qux));
  }

  delete node;

  EXPECT_EQ(cleanups + 4, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 4, Destructions);
}

TEST(WildcardIndexNodeTest, Create) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));
  }

  EXPECT_EQ(cleanups, TestCleanupHandler.calls());
  EXPECT_EQ(destructions, Destructions);

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(4);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(5);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(6);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo, true));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar, true));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz, true));
  }

  EXPECT_EQ(cleanups + 3, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 3, Destructions);

  {
    TestObjectPointer ptr;
    EXPECT_EQ(ESB_SUCCESS, node->find("foo", 3, ptr));
    EXPECT_EQ(ptr->value(), 4);

    EXPECT_EQ(ESB_SUCCESS, node->find("bar", 3, ptr));
    EXPECT_EQ(ptr->value(), 5);

    EXPECT_EQ(ESB_SUCCESS, node->find("baz", 3, ptr));
    EXPECT_EQ(ptr->value(), 6);
  }

  EXPECT_EQ(cleanups + 3, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 3, Destructions);

  delete node;

  EXPECT_EQ(cleanups + 6, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 6, Destructions);
}

TEST(WildcardIndexNodeTest, RemoveFirst) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));
  }

  node->remove("foo", 3);

  EXPECT_EQ(cleanups + 1, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 1, Destructions);

  {
    TestObjectPointer ptr;
    EXPECT_EQ(ESB_CANNOT_FIND, node->find("foo", 3, ptr));

    EXPECT_EQ(ESB_SUCCESS, node->find("bar", 3, ptr));
    EXPECT_EQ(ptr->value(), 2);

    EXPECT_EQ(ESB_SUCCESS, node->find("baz", 3, ptr));
    EXPECT_EQ(ptr->value(), 3);
  }

  delete node;

  EXPECT_EQ(cleanups + 3, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 3, Destructions);
}

TEST(WildcardIndexNodeTest, RemoveMiddle) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));
  }

  node->remove("bar", 3);

  EXPECT_EQ(cleanups + 1, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 1, Destructions);

  {
    TestObjectPointer ptr;
    EXPECT_EQ(ESB_SUCCESS, node->find("foo", 3, ptr));
    EXPECT_EQ(ptr->value(), 1);

    EXPECT_EQ(ESB_CANNOT_FIND, node->find("bar", 3, ptr));

    EXPECT_EQ(ESB_SUCCESS, node->find("baz", 3, ptr));
    EXPECT_EQ(ptr->value(), 3);
  }

  delete node;

  EXPECT_EQ(cleanups + 3, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 3, Destructions);
}

TEST(WildcardIndexNodeTest, RemoveLast) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));
  }

  node->remove("baz", 3);

  EXPECT_EQ(cleanups + 1, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 1, Destructions);

  {
    TestObjectPointer ptr;
    EXPECT_EQ(ESB_SUCCESS, node->find("foo", 3, ptr));
    EXPECT_EQ(ptr->value(), 1);

    EXPECT_EQ(ESB_SUCCESS, node->find("bar", 3, ptr));
    EXPECT_EQ(ptr->value(), 2);

    EXPECT_EQ(ESB_CANNOT_FIND, node->find("baz", 3, ptr));
  }

  delete node;

  EXPECT_EQ(cleanups + 3, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 3, Destructions);
}

TEST(WildcardIndexNodeTest, Empty) {
  char key[ESB_MAX_HOSTNAME + 1];
  memset(key, 'a', sizeof(key));
  key[ESB_MAX_HOSTNAME] = 0;
  WildcardIndexNode *node = WildcardIndexNode::Create(key);

  char wildcard[ESB_UINT8_MAX];
  memset(wildcard, 'b', sizeof(wildcard));

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  EXPECT_TRUE(node->empty());

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);
    SmartPointer qux = new (SystemAllocator::Instance()) TestObject(4);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert(wildcard, sizeof(wildcard), baz));
    EXPECT_EQ(ESB_SUCCESS, node->insert("qux", 3, qux));
  }

  node->remove("foo", 3);
  EXPECT_FALSE(node->empty());
  node->remove("bar", 3);
  EXPECT_FALSE(node->empty());
  node->remove("qux", 3);
  EXPECT_FALSE(node->empty());
  node->remove(wildcard, sizeof(wildcard));
  EXPECT_TRUE(node->empty());

  delete node;

  EXPECT_EQ(cleanups + 4, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 4, Destructions);
}

TEST(WildcardIndexNodeTest, Clear) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));
  }

  EXPECT_EQ(cleanups, TestCleanupHandler.calls());
  EXPECT_EQ(destructions, Destructions);

  node->clear();

  EXPECT_EQ(cleanups + 3, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 3, Destructions);

  {
    TestObjectPointer ptr;
    EXPECT_EQ(ESB_CANNOT_FIND, node->find("foo", 3, ptr));
    EXPECT_EQ(ESB_CANNOT_FIND, node->find("bar", 3, ptr));
    EXPECT_EQ(ESB_CANNOT_FIND, node->find("baz", 3, ptr));
  }

  delete node;

  EXPECT_EQ(cleanups + 3, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 3, Destructions);
}

TEST(WildcardIndexNodeTest, ClearLarge) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");
  char buffer[ESB_UINT8_MAX];
  memset(buffer, 'a', sizeof(buffer));

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);
    SmartPointer qux = new (SystemAllocator::Instance()) TestObject(4);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert(buffer, sizeof(buffer), qux));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));
  }

  EXPECT_EQ(cleanups, TestCleanupHandler.calls());
  EXPECT_EQ(destructions, Destructions);

  node->clear();

  EXPECT_EQ(cleanups + 4, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 4, Destructions);

  {
    TestObjectPointer ptr;
    EXPECT_EQ(ESB_CANNOT_FIND, node->find("foo", 3, ptr));
    EXPECT_EQ(ESB_CANNOT_FIND, node->find("bar", 3, ptr));
    EXPECT_EQ(ESB_CANNOT_FIND, node->find("baz", 3, ptr));
    EXPECT_EQ(ESB_CANNOT_FIND, node->find(buffer, sizeof(buffer), ptr));
  }

  delete node;

  EXPECT_EQ(cleanups + 4, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 4, Destructions);
}

TEST(WildcardIndexNodeTest, LargeKey) {
  char key[ESB_MAX_HOSTNAME + 1];
  memset(key, 'a', sizeof(key));
  key[ESB_MAX_HOSTNAME] = 0;
  WildcardIndexNode *node = WildcardIndexNode::Create(key);

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));
  }

  {
    TestObjectPointer ptr;
    EXPECT_EQ(ESB_SUCCESS, node->find("foo", 3, ptr));
    EXPECT_EQ(ptr->value(), 1);

    EXPECT_EQ(ESB_SUCCESS, node->find("bar", 3, ptr));
    EXPECT_EQ(ptr->value(), 2);

    EXPECT_EQ(ESB_SUCCESS, node->find("baz", 3, ptr));
    EXPECT_EQ(ptr->value(), 3);
  }

  EXPECT_EQ(cleanups, TestCleanupHandler.calls());
  EXPECT_EQ(destructions, Destructions);

  delete node;

  EXPECT_EQ(cleanups + 3, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 3, Destructions);
}

TEST(WildcardIndexNodeTest, LargeKeyAndWildcard) {
  char key[ESB_MAX_HOSTNAME + 1];
  memset(key, 'a', sizeof(key));
  key[ESB_MAX_HOSTNAME] = 0;
  WildcardIndexNode *node = WildcardIndexNode::Create(key);

  char wildcard[ESB_UINT8_MAX];
  memset(wildcard, 'b', sizeof(wildcard));

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);
    SmartPointer qux = new (SystemAllocator::Instance()) TestObject(4);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert(wildcard, sizeof(wildcard), baz));
    EXPECT_EQ(ESB_SUCCESS, node->insert("qux", 3, qux));

    EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, node->insert(wildcard, sizeof(wildcard), baz));
    EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, node->insert("qux", 3, qux));
  }

  {
    TestObjectPointer ptr;
    EXPECT_EQ(ESB_SUCCESS, node->find("foo", 3, ptr));
    EXPECT_EQ(ptr->value(), 1);

    EXPECT_EQ(ESB_SUCCESS, node->find("bar", 3, ptr));
    EXPECT_EQ(ptr->value(), 2);

    EXPECT_EQ(ESB_SUCCESS, node->find(wildcard, sizeof(wildcard), ptr));
    EXPECT_EQ(ptr->value(), 3);

    EXPECT_EQ(ESB_SUCCESS, node->find("qux", 3, ptr));
    EXPECT_EQ(ptr->value(), 4);
  }

  EXPECT_EQ(cleanups, TestCleanupHandler.calls());
  EXPECT_EQ(destructions, Destructions);

  delete node;

  EXPECT_EQ(cleanups + 4, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 4, Destructions);
}

TEST(WildcardIndexNodeTest, RemoveLarge) {
  char key[ESB_MAX_HOSTNAME];
  memset(key, 'a', sizeof(key));
  WildcardIndexNode *node = WildcardIndexNode::Create(key);

  char wildcard[ESB_UINT8_MAX];
  memset(wildcard, 'b', sizeof(wildcard));

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);
    SmartPointer qux = new (SystemAllocator::Instance()) TestObject(4);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert(wildcard, sizeof(wildcard), baz));
    EXPECT_EQ(ESB_SUCCESS, node->insert("qux", 3, qux));
  }

  node->remove(wildcard, sizeof(wildcard));

  {
    TestObjectPointer ptr;
    EXPECT_EQ(ESB_SUCCESS, node->find("foo", 3, ptr));
    EXPECT_EQ(ptr->value(), 1);

    EXPECT_EQ(ESB_SUCCESS, node->find("bar", 3, ptr));
    EXPECT_EQ(ptr->value(), 2);

    EXPECT_EQ(ESB_CANNOT_FIND, node->find(wildcard, sizeof(wildcard), ptr));

    EXPECT_EQ(ESB_SUCCESS, node->find("qux", 3, ptr));
    EXPECT_EQ(ptr->value(), 4);
  }

  EXPECT_EQ(cleanups + 1, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 1, Destructions);

  delete node;

  EXPECT_EQ(cleanups + 4, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 4, Destructions);
}

TEST(WildcardIndexNodeTest, InvalidArgs) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");
  char buffer[ESB_UINT8_MAX + 1];
  memset(buffer, 'a', sizeof(buffer));

  SmartPointer ptr = new (SystemAllocator::Instance()) TestObject(42);
  EXPECT_EQ(ESB_OVERFLOW, node->insert(buffer, sizeof(buffer), ptr));
  EXPECT_EQ(ESB_UNDERFLOW, node->insert(buffer, 0, ptr, false));
  EXPECT_EQ(ESB_UNDERFLOW, node->insert("", 0, ptr));
  EXPECT_EQ(ESB_NULL_POINTER, node->insert(NULL, 0, ptr, false));
  ptr = NULL;
  EXPECT_EQ(ESB_INVALID_ARGUMENT, node->insert("foo", 3, ptr));

  delete node;
}

TEST(WildcardIndexNodeTest, RemoveMissingKey) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));
  }

  EXPECT_EQ(ESB_CANNOT_FIND, node->remove("qux", 3));

  delete node;
}

TEST(WildcardIndexNodeTest, DestructorOnlyDecrements) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
  SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
  SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));

  delete node;

  EXPECT_EQ(cleanups, TestCleanupHandler.calls());
  EXPECT_EQ(destructions, Destructions);
}

TEST(WildcardIndexNodeTest, Update) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));
  }

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(4);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(5);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(6);

    TestObjectPointer old;
    EXPECT_EQ(ESB_SUCCESS, node->update("foo", 3, foo, &old));
    EXPECT_EQ(old->value(), 1);
    EXPECT_EQ(ESB_SUCCESS, node->update("bar", 3, bar, &old));
    EXPECT_EQ(old->value(), 2);
    EXPECT_EQ(ESB_SUCCESS, node->update("baz", 3, baz, &old));
    EXPECT_EQ(old->value(), 3);
  }

  EXPECT_EQ(cleanups + 3, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 3, Destructions);

  {
    TestObjectPointer ptr;
    EXPECT_EQ(ESB_SUCCESS, node->find("foo", 3, ptr));
    EXPECT_EQ(ptr->value(), 4);

    EXPECT_EQ(ESB_SUCCESS, node->find("bar", 3, ptr));
    EXPECT_EQ(ptr->value(), 5);

    EXPECT_EQ(ESB_SUCCESS, node->find("baz", 3, ptr));
    EXPECT_EQ(ptr->value(), 6);
  }

  delete node;

  EXPECT_EQ(cleanups + 6, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 6, Destructions);
}

TEST(WildcardIndexNodeTest, UpdateLarge) {
  char key[ESB_MAX_HOSTNAME + 1];
  memset(key, 'a', sizeof(key));
  key[ESB_MAX_HOSTNAME] = 0;
  WildcardIndexNode *node = WildcardIndexNode::Create(key);
  char buffer[ESB_UINT8_MAX];
  memset(buffer, 'a', sizeof(buffer));

  int cleanups = TestCleanupHandler.calls();
  int destructions = Destructions;

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);
    SmartPointer qux = new (SystemAllocator::Instance()) TestObject(4);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert(buffer, sizeof(buffer), qux));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));
  }

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(5);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(6);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(7);
    SmartPointer qux = new (SystemAllocator::Instance()) TestObject(8);

    TestObjectPointer old;
    EXPECT_EQ(ESB_SUCCESS, node->update("foo", 3, foo, &old));
    EXPECT_EQ(old->value(), 1);
    EXPECT_EQ(ESB_SUCCESS, node->update("bar", 3, bar, &old));
    EXPECT_EQ(old->value(), 2);
    EXPECT_EQ(ESB_SUCCESS, node->update("baz", 3, baz, &old));
    EXPECT_EQ(old->value(), 3);
    EXPECT_EQ(ESB_SUCCESS, node->update(buffer, sizeof(buffer), qux, &old));
    EXPECT_EQ(old->value(), 4);
  }

  EXPECT_EQ(cleanups + 4, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 4, Destructions);

  {
    TestObjectPointer ptr;
    EXPECT_EQ(ESB_SUCCESS, node->find("foo", 3, ptr));
    EXPECT_EQ(ptr->value(), 5);

    EXPECT_EQ(ESB_SUCCESS, node->find("bar", 3, ptr));
    EXPECT_EQ(ptr->value(), 6);

    EXPECT_EQ(ESB_SUCCESS, node->find("baz", 3, ptr));
    EXPECT_EQ(ptr->value(), 7);

    EXPECT_EQ(ESB_SUCCESS, node->find(buffer, sizeof(buffer), ptr));
    EXPECT_EQ(ptr->value(), 8);
  }

  delete node;

  EXPECT_EQ(cleanups + 8, TestCleanupHandler.calls());
  EXPECT_EQ(destructions + 8, Destructions);
}

TEST(WildcardIndexNodeTest, UpdateMissingKey) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");

  {
    char buffer[ESB_UINT8_MAX];
    memset(buffer, 'a', sizeof(buffer));

    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);
    SmartPointer qux = new (SystemAllocator::Instance()) TestObject(4);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert(buffer, sizeof(buffer), qux));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));
  }

  TestObjectPointer value = new (SystemAllocator::Instance()) TestObject(42);
  TestObjectPointer old;
  EXPECT_EQ(ESB_CANNOT_FIND, node->update("qux", 3, value, &old));
  EXPECT_EQ(ESB_CANNOT_FIND, node->update("quux", 4, value, &old));

  delete node;
}

TEST(WildcardIndexNodeTest, Iterate) {
  char nodeKey[ESB_MAX_HOSTNAME + 1];
  memset(nodeKey, 'a', sizeof(nodeKey));
  nodeKey[ESB_MAX_HOSTNAME] = 0;
  WildcardIndexNode *node = WildcardIndexNode::Create(nodeKey);
  char buffer[ESB_UINT8_MAX];
  memset(buffer, 'a', sizeof(buffer));

  {
    SmartPointer foo = new (SystemAllocator::Instance()) TestObject(1);
    SmartPointer bar = new (SystemAllocator::Instance()) TestObject(2);
    SmartPointer baz = new (SystemAllocator::Instance()) TestObject(3);
    SmartPointer qux = new (SystemAllocator::Instance()) TestObject(4);

    EXPECT_EQ(ESB_SUCCESS, node->insert("foo", 3, foo));
    EXPECT_EQ(ESB_SUCCESS, node->insert("bar", 3, bar));
    EXPECT_EQ(ESB_SUCCESS, node->insert(buffer, sizeof(buffer), qux));
    EXPECT_EQ(ESB_SUCCESS, node->insert("baz", 3, baz));
  }

  const char *key = NULL;
  ESB::UInt32 keySize;
  TestObjectPointer ptr;
  const WildcardIndexNode::Iterator *it = NULL;

  it = node->first();
  EXPECT_FALSE(node->last(it));
  EXPECT_EQ(ESB_SUCCESS, node->key(it, &key, &keySize));
  EXPECT_EQ(3, keySize);
  EXPECT_TRUE(0 == memcmp(key, "foo", 3));
  EXPECT_EQ(ESB_SUCCESS, node->value(it, ptr));
  EXPECT_EQ(1, ptr->value());

  EXPECT_EQ(ESB_SUCCESS, node->next(&it));
  EXPECT_FALSE(node->last(it));
  EXPECT_EQ(ESB_SUCCESS, node->key(it, &key, &keySize));
  EXPECT_EQ(3, keySize);
  EXPECT_TRUE(0 == memcmp(key, "bar", 3));
  EXPECT_EQ(ESB_SUCCESS, node->value(it, ptr));
  EXPECT_EQ(2, ptr->value());

  EXPECT_EQ(ESB_SUCCESS, node->next(&it));
  EXPECT_FALSE(node->last(it));
  EXPECT_EQ(ESB_SUCCESS, node->key(it, &key, &keySize));
  EXPECT_EQ(sizeof(buffer), keySize);
  EXPECT_TRUE(0 == memcmp(key, buffer, sizeof(buffer)));
  EXPECT_EQ(ESB_SUCCESS, node->value(it, ptr));
  EXPECT_EQ(4, ptr->value());

  EXPECT_EQ(ESB_SUCCESS, node->next(&it));
  EXPECT_FALSE(node->last(it));
  EXPECT_EQ(ESB_SUCCESS, node->key(it, &key, &keySize));
  EXPECT_EQ(3, keySize);
  EXPECT_TRUE(0 == memcmp(key, "baz", 3));
  EXPECT_EQ(ESB_SUCCESS, node->value(it, ptr));
  EXPECT_EQ(3, ptr->value());

  // end of iteration

  EXPECT_EQ(ESB_SUCCESS, node->next(&it));
  EXPECT_TRUE(node->last(it));
  EXPECT_EQ(ESB_CANNOT_FIND, node->next(&it));
  EXPECT_EQ(ESB_CANNOT_FIND, node->key(it, &key, &keySize));
  EXPECT_EQ(ESB_CANNOT_FIND, node->value(it, ptr));

  delete node;
}

class WildcardIndexTest : public ::testing::Test {
 public:
  WildcardIndexTest() : _index(42, 3, ESB::SystemAllocator::Instance()) {}

  virtual void SetUp() {
    TestObjectPointer one = new (SystemAllocator::Instance()) TestObject(1);
    TestObjectPointer two = new (SystemAllocator::Instance()) TestObject(2);
    TestObjectPointer three = new (SystemAllocator::Instance()) TestObject(3);
    TestObjectPointer four = new (SystemAllocator::Instance()) TestObject(4);
    TestObjectPointer five = new (SystemAllocator::Instance()) TestObject(5);

    EXPECT_EQ(ESB_SUCCESS, _index.insert("d1.com", "foo", 3, one));
    EXPECT_EQ(ESB_SUCCESS, _index.insert("d1.com", "b*r", 3, two));
    EXPECT_EQ(ESB_SUCCESS, _index.insert("d1.com", "b*", 2, three));
    EXPECT_EQ(ESB_SUCCESS, _index.insert("d2.com", "foo", 3, four));
    EXPECT_EQ(ESB_SUCCESS, _index.insert("d2.com", "q*x", 3, five));
  }

  virtual void TearDown() { _index.clear(); }

 protected:
  WildcardIndex _index;
};

TEST_F(WildcardIndexTest, Find) {
  TestObjectPointer value;
  EXPECT_EQ(ESB_SUCCESS, _index.find("d1.com", "foo", 3, value));
  EXPECT_EQ(1, value->value());
  EXPECT_EQ(ESB_SUCCESS, _index.find("d1.com", "b*r", 3, value));
  EXPECT_EQ(2, value->value());
  EXPECT_EQ(ESB_SUCCESS, _index.find("d1.com", "b*", 2, value));
  EXPECT_EQ(3, value->value());
  EXPECT_EQ(ESB_SUCCESS, _index.find("d2.com", "foo", 3, value));
  EXPECT_EQ(4, value->value());
  EXPECT_EQ(ESB_SUCCESS, _index.find("d2.com", "q*x", 3, value));
  EXPECT_EQ(5, value->value());
}

TEST_F(WildcardIndexTest, Match) {
  TestObjectPointer value;
  EXPECT_EQ(ESB_SUCCESS, _index.match("d1.com", "foo", 3, value));
  EXPECT_EQ(1, value->value());
  EXPECT_EQ(ESB_SUCCESS, _index.match("d1.com", "bar", 3, value));
  EXPECT_EQ(2, value->value());
  EXPECT_EQ(ESB_SUCCESS, _index.match("d1.com", "baz", 3, value));
  EXPECT_EQ(3, value->value());
  EXPECT_EQ(ESB_SUCCESS, _index.match("d2.com", "foo", 3, value));
  EXPECT_EQ(4, value->value());
  EXPECT_EQ(ESB_SUCCESS, _index.match("d2.com", "qux", 3, value));
  EXPECT_EQ(5, value->value());
  EXPECT_EQ(ESB_SUCCESS, _index.match("d2.com", "quux", 4, value));
  EXPECT_EQ(5, value->value());
}

TEST_F(WildcardIndexTest, Remove) {
  EXPECT_EQ(ESB_SUCCESS, _index.remove("d1.com", "foo", 3));
  EXPECT_EQ(ESB_SUCCESS, _index.remove("d1.com", "b*r", 3));
  EXPECT_EQ(ESB_SUCCESS, _index.remove("d1.com", "b*", 2));
  EXPECT_EQ(ESB_SUCCESS, _index.remove("d2.com", "foo", 3));
  EXPECT_EQ(ESB_SUCCESS, _index.remove("d2.com", "q*x", 3));

  TestObjectPointer value;
  EXPECT_EQ(ESB_CANNOT_FIND, _index.find("d1.com", "foo", 3, value));
  EXPECT_EQ(ESB_CANNOT_FIND, _index.find("d1.com", "b*r", 3, value));
  EXPECT_EQ(ESB_CANNOT_FIND, _index.find("d1.com", "b*", 2, value));
  EXPECT_EQ(ESB_CANNOT_FIND, _index.find("d2.com", "foo", 3, value));
  EXPECT_EQ(ESB_CANNOT_FIND, _index.find("d2.com", "q*x", 3, value));
}

TEST_F(WildcardIndexTest, Update) {
  {
    TestObjectPointer value = new (SystemAllocator::Instance()) TestObject(42);
    TestObjectPointer old;

    EXPECT_EQ(ESB_SUCCESS, _index.update("d1.com", "foo", 3, value, &old));
    EXPECT_EQ(1, old->value());
    EXPECT_EQ(ESB_SUCCESS, _index.update("d1.com", "b*r", 3, value, &old));
    EXPECT_EQ(2, old->value());
    EXPECT_EQ(ESB_SUCCESS, _index.update("d1.com", "b*", 2, value, &old));
    EXPECT_EQ(3, old->value());
    EXPECT_EQ(ESB_SUCCESS, _index.update("d2.com", "foo", 3, value, &old));
    EXPECT_EQ(4, old->value());
    EXPECT_EQ(ESB_SUCCESS, _index.update("d2.com", "q*x", 3, value, &old));
    EXPECT_EQ(5, old->value());
  }

  TestObjectPointer value;
  EXPECT_EQ(ESB_SUCCESS, _index.find("d1.com", "foo", 3, value));
  EXPECT_EQ(42, value->value());
  EXPECT_EQ(ESB_SUCCESS, _index.find("d1.com", "b*r", 3, value));
  EXPECT_EQ(42, value->value());
  EXPECT_EQ(ESB_SUCCESS, _index.find("d1.com", "b*", 2, value));
  EXPECT_EQ(42, value->value());
  EXPECT_EQ(ESB_SUCCESS, _index.find("d2.com", "foo", 3, value));
  EXPECT_EQ(42, value->value());
  EXPECT_EQ(ESB_SUCCESS, _index.find("d2.com", "q*x", 3, value));
  EXPECT_EQ(42, value->value());
}

TEST_F(WildcardIndexTest, UpdateMiss) {
  TestObjectPointer value = new (SystemAllocator::Instance()) TestObject(42);
  TestObjectPointer old;

  EXPECT_EQ(ESB_CANNOT_FIND, _index.update("d1.com", "qux", 3, value, &old));
  EXPECT_TRUE(old.isNull());
  EXPECT_EQ(ESB_CANNOT_FIND, _index.update("d3.com", "foo", 3, value, &old));
  EXPECT_TRUE(old.isNull());
}

TEST_F(WildcardIndexTest, Clear) {
  _index.clear();

  TestObjectPointer value;
  EXPECT_EQ(ESB_CANNOT_FIND, _index.find("d1.com", "foo", 3, value));
  EXPECT_EQ(ESB_CANNOT_FIND, _index.find("d1.com", "b*r", 3, value));
  EXPECT_EQ(ESB_CANNOT_FIND, _index.find("d1.com", "b*", 2, value));
  EXPECT_EQ(ESB_CANNOT_FIND, _index.find("d2.com", "foo", 3, value));
  EXPECT_EQ(ESB_CANNOT_FIND, _index.find("d2.com", "q*x", 3, value));
}

TEST_F(WildcardIndexTest, Uniqueness) {
  TestObjectPointer value = new (SystemAllocator::Instance()) TestObject(42);
  EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, _index.insert("d1.com", "foo", 3, value));
  EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, _index.insert("d1.com", "b*r", 3, value));
  EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, _index.insert("d1.com", "b*", 2, value));
  EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, _index.insert("d2.com", "foo", 3, value));
  EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, _index.insert("d2.com", "q*x", 3, value));
}