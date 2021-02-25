#ifndef ESB_COMPACT_STRING_MAP_H
#include <ESBCompactStringMap.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(CompactStringMapTest, Find) {
  CompactStringMap map;
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, map.insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, map.insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, map.insert("baz", &baz));

  int *value = (int *)map.find("foo");
  EXPECT_EQ(&foo, value);

  value = (int *)map.find("bar");
  EXPECT_EQ(&bar, value);

  value = (int *)map.find("baz");
  EXPECT_EQ(&baz, value);
}

TEST(CompactStringMapTest, EnforceUnique) {
  CompactStringMap map;
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, map.insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, map.insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, map.insert("baz", &baz));

  EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, map.insert("foo", &foo));
  EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, map.insert("bar", &bar));
  EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, map.insert("baz", &baz));
}

TEST(CompactStringMapTest, Create) {
  CompactStringMap map;
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, map.insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, map.insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, map.insert("baz", &baz));

  EXPECT_EQ(ESB_SUCCESS, map.insert("foo", &bar, true));
  EXPECT_EQ(ESB_SUCCESS, map.insert("bar", &baz, true));
  EXPECT_EQ(ESB_SUCCESS, map.insert("baz", &foo, true));

  int *value = (int *)map.find("foo");
  EXPECT_EQ(&bar, value);

  value = (int *)map.find("bar");
  EXPECT_EQ(&baz, value);

  value = (int *)map.find("baz");
  EXPECT_EQ(&foo, value);
}

TEST(CompactStringMapTest, RemoveFirst) {
  CompactStringMap map;
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, map.insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, map.insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, map.insert("baz", &baz));

  map.remove("foo");

  int *value = (int *)map.find("foo");
  EXPECT_EQ(NULL, value);

  value = (int *)map.find("bar");
  EXPECT_EQ(&bar, value);

  value = (int *)map.find("baz");
  EXPECT_EQ(&baz, value);
}

TEST(CompactStringMapTest, RemoveMiddle) {
  CompactStringMap map;
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, map.insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, map.insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, map.insert("baz", &baz));

  map.remove("bar");

  int *value = (int *)map.find("foo");
  EXPECT_EQ(&foo, value);

  value = (int *)map.find("bar");
  EXPECT_EQ(NULL, value);

  value = (int *)map.find("baz");
  EXPECT_EQ(&baz, value);
}

TEST(CompactStringMapTest, RemoveLast) {
  CompactStringMap map;
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, map.insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, map.insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, map.insert("baz", &baz));

  map.remove("baz");

  int *value = (int *)map.find("foo");
  EXPECT_EQ(&foo, value);

  value = (int *)map.find("bar");
  EXPECT_EQ(&bar, value);

  value = (int *)map.find("baz");
  EXPECT_EQ(NULL, value);
}

TEST(CompactStringMapTest, Clear) {
  CompactStringMap map;
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, map.insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, map.insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, map.insert("baz", &baz));

  map.clear();

  int *value = (int *)map.find("foo");
  EXPECT_EQ(NULL, value);

  value = (int *)map.find("bar");
  EXPECT_EQ(NULL, value);

  value = (int *)map.find("baz");
  EXPECT_EQ(NULL, value);
}

TEST(CompactStringMapTest, Realloc) {
  CompactStringMap map(64);
  int foo = 1;
  int bar = 2;
  int baz = 3;
  int qux = 4;
  char buffer[ESB_UINT8_MAX];
  memset(buffer, 'a', sizeof(buffer));

  EXPECT_EQ(ESB_SUCCESS, map.insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, map.insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, map.insert(buffer, sizeof(buffer), &qux));
  EXPECT_EQ(ESB_SUCCESS, map.insert("baz", &baz));

  int *value = (int *)map.find("foo");
  EXPECT_EQ(&foo, value);

  value = (int *)map.find("bar");
  EXPECT_EQ(&bar, value);

  value = (int *)map.find(buffer);
  EXPECT_EQ(&qux, value);

  value = (int *)map.find("baz");
  EXPECT_EQ(&baz, value);
}

TEST(CompactStringMapTest, RemoveLarge) {
  CompactStringMap map(64);
  int foo = 1;
  int bar = 2;
  int baz = 3;
  int qux = 4;
  char buffer[ESB_UINT8_MAX];
  memset(buffer, 'a', sizeof(buffer));

  EXPECT_EQ(ESB_SUCCESS, map.insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, map.insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, map.insert(buffer, sizeof(buffer), &qux));
  EXPECT_EQ(ESB_SUCCESS, map.insert("baz", &baz));

  map.remove(buffer, sizeof(buffer));

  int *value = (int *)map.find("foo");
  EXPECT_EQ(&foo, value);

  value = (int *)map.find("bar");
  EXPECT_EQ(&bar, value);

  value = (int *)map.find(buffer);
  EXPECT_EQ(NULL, value);

  value = (int *)map.find("baz");
  EXPECT_EQ(&baz, value);
}

TEST(CompactStringMapTest, InvalidKey) {
  CompactStringMap map;
  char buffer[ESB_UINT8_MAX + 1];
  memset(buffer, 'a', sizeof(buffer));

  EXPECT_EQ(ESB_OVERFLOW, map.insert(buffer, sizeof(buffer), NULL));
  EXPECT_EQ(ESB_UNDERFLOW, map.insert(buffer, 0, NULL, false));
  EXPECT_EQ(ESB_UNDERFLOW, map.insert("", NULL));
  EXPECT_EQ(ESB_NULL_POINTER, map.insert(NULL, 0, NULL, false));
  EXPECT_EQ(ESB_NULL_POINTER, map.insert(NULL, NULL));
}

TEST(CompactStringMapTest, RemoveMissingKey) {
  CompactStringMap map;
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, map.insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, map.insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, map.insert("baz", &baz));

  EXPECT_EQ(ESB_CANNOT_FIND, map.remove("qux"));
}

TEST(CompactStringMapTest, Update) {
  CompactStringMap map;
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, map.insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, map.insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, map.insert("baz", &baz));

  int *value = NULL;
  EXPECT_EQ(ESB_SUCCESS, map.update("foo", &bar, (void **)&value));
  EXPECT_EQ(&foo, value);

  value = NULL;
  EXPECT_EQ(ESB_SUCCESS, map.update("bar", &baz, (void **)&value));
  EXPECT_EQ(&bar, value);

  value = NULL;
  EXPECT_EQ(ESB_SUCCESS, map.update("baz", &foo, (void **)&value));
  EXPECT_EQ(&baz, value);

  value = (int *)map.find("foo");
  EXPECT_EQ(&bar, value);

  value = (int *)map.find("bar");
  EXPECT_EQ(&baz, value);

  value = (int *)map.find("baz");
  EXPECT_EQ(&foo, value);
}

TEST(CompactStringMapTest, UpdateMissingKey) {
  CompactStringMap map;
  int foo = 1;
  int bar = 2;
  int baz = 3;

  int *value = NULL;
  EXPECT_EQ(ESB_CANNOT_FIND, map.update("foo", &bar, (void **)&value));
  EXPECT_EQ(NULL, value);

  value = NULL;
  EXPECT_EQ(ESB_CANNOT_FIND, map.update("bar", &baz, (void **)&value));
  EXPECT_EQ(NULL, value);

  value = NULL;
  EXPECT_EQ(ESB_CANNOT_FIND, map.update("baz", &foo, (void **)&value));
  EXPECT_EQ(NULL, value);
}

TEST(CompactStringMapTest, Iterate) {
  CompactStringMap map;
  int foo = 1;
  int bar = 2;
  int baz = 3;

  EXPECT_EQ(ESB_SUCCESS, map.insert("foo", &foo));
  EXPECT_EQ(ESB_SUCCESS, map.insert("bar", &bar));
  EXPECT_EQ(ESB_SUCCESS, map.insert("baz", &baz));

  UInt32 marker = map.firstMarker();
  const char *key = NULL;
  UInt32 keySize;
  void *value = NULL;

  EXPECT_TRUE(map.hasNext(marker));
  EXPECT_EQ(ESB_SUCCESS, map.next(&key, &keySize, &value, &marker));
  EXPECT_EQ(3, keySize);
  EXPECT_TRUE(0 == memcmp(key, "foo", 3));

  EXPECT_TRUE(map.hasNext(marker));
  EXPECT_EQ(ESB_SUCCESS, map.next(&key, &keySize, &value, &marker));
  EXPECT_EQ(3, keySize);
  EXPECT_TRUE(0 == memcmp(key, "bar", 3));

  EXPECT_TRUE(map.hasNext(marker));
  EXPECT_EQ(ESB_SUCCESS, map.next(&key, &keySize, &value, &marker));
  EXPECT_EQ(3, keySize);
  EXPECT_TRUE(0 == memcmp(key, "baz", 3));

  EXPECT_FALSE(map.hasNext(marker));
  EXPECT_EQ(ESB_CANNOT_FIND, map.next(&key, &keySize, &value, &marker));
}