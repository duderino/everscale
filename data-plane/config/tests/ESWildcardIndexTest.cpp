#ifndef ES_WILDCARD_INDEX_H
#include <ESWildcardIndex.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#include <gtest/gtest.h>

using namespace ES;

TEST(WildcardIndexNodeTest, Find) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", &baz));

  int *value = (int *)node->find("foo");
  EXPECT_EQ(&foo, value);

  value = (int *)node->find("bar");
  EXPECT_EQ(&bar, value);

  value = (int *)node->find("baz");
  EXPECT_EQ(&baz, value);

  delete node;
}

TEST(WildcardIndexNodeTest, EnforceUnique) {
  char key[ESB_MAX_HOSTNAME + 1];
  memset(key, 'a', sizeof(key));
  key[ESB_MAX_HOSTNAME] = 0;
  WildcardIndexNode *node = WildcardIndexNode::Create(key);
  int foo = 1;
  int bar = 2;
  int baz = 3;
  int qux = 4;

  char wildcard[ESB_UINT8_MAX + 1];
  memset(wildcard, 'b', sizeof(wildcard));
  wildcard[ESB_UINT8_MAX] = 0;

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert(wildcard, &baz));
  EXPECT_EQ(ESB_SUCCESS, node->insert("qux", &qux));

  EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, node->insert("foo", &foo));
  EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, node->insert("bar", &bar));
  EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, node->insert(wildcard, &baz));
  EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, node->insert("qux", &qux));

  delete node;
}

TEST(WildcardIndexNodeTest, Create) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", &baz));

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &bar, true));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &baz, true));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", &foo, true));

  int *value = (int *)node->find("foo");
  EXPECT_EQ(&bar, value);

  value = (int *)node->find("bar");
  EXPECT_EQ(&baz, value);

  value = (int *)node->find("baz");
  EXPECT_EQ(&foo, value);

  delete node;
}

TEST(WildcardIndexNodeTest, RemoveFirst) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", &baz));

  node->remove("foo");

  int *value = (int *)node->find("foo");
  EXPECT_EQ(NULL, value);

  value = (int *)node->find("bar");
  EXPECT_EQ(&bar, value);

  value = (int *)node->find("baz");
  EXPECT_EQ(&baz, value);

  delete node;
}

TEST(WildcardIndexNodeTest, RemoveMiddle) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", &baz));

  node->remove("bar");

  int *value = (int *)node->find("foo");
  EXPECT_EQ(&foo, value);

  value = (int *)node->find("bar");
  EXPECT_EQ(NULL, value);

  value = (int *)node->find("baz");
  EXPECT_EQ(&baz, value);

  delete node;
}

TEST(WildcardIndexNodeTest, RemoveLast) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", &baz));

  node->remove("baz");

  int *value = (int *)node->find("foo");
  EXPECT_EQ(&foo, value);

  value = (int *)node->find("bar");
  EXPECT_EQ(&bar, value);

  value = (int *)node->find("baz");
  EXPECT_EQ(NULL, value);

  delete node;
}

TEST(WildcardIndexNodeTest, Clear) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", &baz));

  node->clear();

  int *value = (int *)node->find("foo");
  EXPECT_EQ(NULL, value);

  value = (int *)node->find("bar");
  EXPECT_EQ(NULL, value);

  value = (int *)node->find("baz");
  EXPECT_EQ(NULL, value);

  delete node;
}

TEST(WildcardIndexNodeTest, LargeKey) {
  char key[ESB_MAX_HOSTNAME + 1];
  memset(key, 'a', sizeof(key));
  key[ESB_MAX_HOSTNAME] = 0;
  WildcardIndexNode *node = WildcardIndexNode::Create(key);
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", &baz));

  int *value = (int *)node->find("foo");
  EXPECT_EQ(&foo, value);

  value = (int *)node->find("bar");
  EXPECT_EQ(&bar, value);

  value = (int *)node->find("baz");
  EXPECT_EQ(&baz, value);

  delete node;
}

TEST(WildcardIndexNodeTest, LargeKeyAndWildcard) {
  char key[ESB_MAX_HOSTNAME + 1];
  memset(key, 'a', sizeof(key));
  key[ESB_MAX_HOSTNAME] = 0;
  WildcardIndexNode *node = WildcardIndexNode::Create(key);
  int foo = 1;
  int bar = 2;
  int baz = 3;
  int qux = 4;

  char wildcard[ESB_UINT8_MAX + 1];
  memset(wildcard, 'b', sizeof(wildcard));
  wildcard[ESB_UINT8_MAX] = 0;

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert(wildcard, &baz));
  EXPECT_EQ(ESB_SUCCESS, node->insert("qux", &qux));

  int *value = (int *)node->find("foo");
  EXPECT_EQ(&foo, value);

  value = (int *)node->find("bar");
  EXPECT_EQ(&bar, value);

  value = (int *)node->find(wildcard);
  EXPECT_EQ(&baz, value);

  value = (int *)node->find("qux");
  EXPECT_EQ(&qux, value);

  delete node;
}

TEST(WildcardIndexNodeTest, RemoveLarge) {
  char key[ESB_MAX_HOSTNAME + 1];
  memset(key, 'a', sizeof(key));
  key[ESB_MAX_HOSTNAME] = 0;
  WildcardIndexNode *node = WildcardIndexNode::Create(key);
  int foo = 1;
  int bar = 2;
  int baz = 3;
  int qux = 4;
  char buffer[ESB_UINT8_MAX];
  memset(buffer, 'a', sizeof(buffer));

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert(buffer, sizeof(buffer), &qux));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", &baz));

  node->remove(buffer, sizeof(buffer));

  int *value = (int *)node->find("foo");
  EXPECT_EQ(&foo, value);

  value = (int *)node->find("bar");
  EXPECT_EQ(&bar, value);

  value = (int *)node->find(buffer);
  EXPECT_EQ(NULL, value);

  value = (int *)node->find("baz");
  EXPECT_EQ(&baz, value);

  delete node;
}

TEST(WildcardIndexNodeTest, InvalidKey) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");
  char buffer[ESB_UINT8_MAX + 1];
  memset(buffer, 'a', sizeof(buffer));

  EXPECT_EQ(ESB_OVERFLOW, node->insert(buffer, sizeof(buffer), NULL));
  EXPECT_EQ(ESB_UNDERFLOW, node->insert(buffer, 0, NULL, false));
  EXPECT_EQ(ESB_UNDERFLOW, node->insert("", NULL));
  EXPECT_EQ(ESB_NULL_POINTER, node->insert(NULL, 0, NULL, false));
  EXPECT_EQ(ESB_NULL_POINTER, node->insert(NULL, NULL));

  delete node;
}

TEST(WildcardIndexNodeTest, RemoveMissingKey) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", &baz));

  EXPECT_EQ(ESB_CANNOT_FIND, node->remove("qux"));

  delete node;
}

TEST(WildcardIndexNodeTest, Update) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", &baz));

  int *value = NULL;
  EXPECT_EQ(ESB_SUCCESS, node->update("foo", &bar, (void **)&value));
  EXPECT_EQ(&foo, value);

  value = NULL;
  EXPECT_EQ(ESB_SUCCESS, node->update("bar", &baz, (void **)&value));
  EXPECT_EQ(&bar, value);

  value = NULL;
  EXPECT_EQ(ESB_SUCCESS, node->update("baz", &foo, (void **)&value));
  EXPECT_EQ(&baz, value);

  value = (int *)node->find("foo");
  EXPECT_EQ(&bar, value);

  value = (int *)node->find("bar");
  EXPECT_EQ(&baz, value);

  value = (int *)node->find("baz");
  EXPECT_EQ(&foo, value);

  delete node;
}

TEST(WildcardIndexNodeTest, UpdateLarge) {
  char key[ESB_MAX_HOSTNAME + 1];
  memset(key, 'a', sizeof(key));
  key[ESB_MAX_HOSTNAME] = 0;
  WildcardIndexNode *node = WildcardIndexNode::Create(key);
  int foo = 1;
  int bar = 2;
  int baz = 3;
  int qux = 4;
  char buffer[ESB_UINT8_MAX];
  memset(buffer, 'a', sizeof(buffer));

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert(buffer, sizeof(buffer), &qux));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", &baz));

  int *value = NULL;
  EXPECT_EQ(ESB_SUCCESS, node->update("foo", &bar, (void **)&value));
  EXPECT_EQ(&foo, value);

  value = NULL;
  EXPECT_EQ(ESB_SUCCESS, node->update("bar", &baz, (void **)&value));
  EXPECT_EQ(&bar, value);

  value = NULL;
  EXPECT_EQ(ESB_SUCCESS, node->update(buffer, sizeof(buffer), &foo, (void **)&value));
  EXPECT_EQ(&qux, value);

  value = NULL;
  EXPECT_EQ(ESB_SUCCESS, node->update("baz", &qux, (void **)&value));
  EXPECT_EQ(&baz, value);

  value = (int *)node->find("foo");
  EXPECT_EQ(&bar, value);

  value = (int *)node->find("bar");
  EXPECT_EQ(&baz, value);

  value = (int *)node->find(buffer, sizeof(buffer));
  EXPECT_EQ(&foo, value);

  value = (int *)node->find("baz");
  EXPECT_EQ(&qux, value);

  delete node;
}

TEST(WildcardIndexNodeTest, UpdateMissingKey) {
  WildcardIndexNode *node = WildcardIndexNode::Create("foo.bar.baz");
  int foo = 1;
  int bar = 2;
  int baz = 3;

  int *value = NULL;
  EXPECT_EQ(ESB_CANNOT_FIND, node->update("foo", &bar, (void **)&value));
  EXPECT_EQ(NULL, value);

  value = NULL;
  EXPECT_EQ(ESB_CANNOT_FIND, node->update("bar", &baz, (void **)&value));
  EXPECT_EQ(NULL, value);

  value = NULL;
  EXPECT_EQ(ESB_CANNOT_FIND, node->update("baz", &foo, (void **)&value));
  EXPECT_EQ(NULL, value);

  delete node;
}

TEST(WildcardIndexNodeTest, Iterate) {
  char nodeKey[ESB_MAX_HOSTNAME + 1];
  memset(nodeKey, 'a', sizeof(nodeKey));
  nodeKey[ESB_MAX_HOSTNAME] = 0;
  WildcardIndexNode *node = WildcardIndexNode::Create(nodeKey);
  int foo = 1;
  int bar = 2;
  int baz = 3;
  int qux = 4;
  char buffer[ESB_UINT8_MAX];
  memset(buffer, 'a', sizeof(buffer));

  EXPECT_EQ(ESB_SUCCESS, node->insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, node->insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, node->insert(buffer, sizeof(buffer), &qux));
  EXPECT_EQ(ESB_SUCCESS, node->insert("baz", &baz));

  const WildcardIndexNode::Marker *marker = node->firstMarker();
  const char *key = NULL;
  ESB::UInt32 keySize;
  void *value = NULL;

  EXPECT_TRUE(node->hasNext(marker));
  EXPECT_EQ(ESB_SUCCESS, node->next(&key, &keySize, &value, &marker));
  EXPECT_EQ(3, keySize);
  EXPECT_TRUE(0 == memcmp(key, "foo", 3));

  EXPECT_TRUE(node->hasNext(marker));
  EXPECT_EQ(ESB_SUCCESS, node->next(&key, &keySize, &value, &marker));
  EXPECT_EQ(3, keySize);
  EXPECT_TRUE(0 == memcmp(key, "bar", 3));

  // insertion order is different here than iteration order.
  // The large key (buffer) is inserted into an overflow area
  // The the subsequent key (baz) is inserted into the regular area
  // iteration iterates through the regular area first, then considers the overflow area.

  EXPECT_TRUE(node->hasNext(marker));
  EXPECT_EQ(ESB_SUCCESS, node->next(&key, &keySize, &value, &marker));
  EXPECT_EQ(3, keySize);
  EXPECT_TRUE(0 == memcmp(key, "baz", 3));

  EXPECT_TRUE(node->hasNext(marker));
  EXPECT_EQ(ESB_SUCCESS, node->next(&key, &keySize, &value, &marker));
  EXPECT_EQ(sizeof(buffer), keySize);
  EXPECT_TRUE(0 == memcmp(key, buffer, sizeof(buffer)));

  EXPECT_FALSE(node->hasNext(marker));
  EXPECT_EQ(ESB_CANNOT_FIND, node->next(&key, &keySize, &value, &marker));

  delete node;
}