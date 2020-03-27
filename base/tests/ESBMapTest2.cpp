#ifndef ESB_MAP_H
#include <ESBMap.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

struct StringComparator : public Comparator {
  int compare(const void *first, const void *second) const {
    return strcmp((const char *)first, (const char *)second);
  }
};

static StringComparator StringComparator;

TEST(Map, Insert) {
  Map map(StringComparator);

  Error error = map.insert("foo", (void *)"bar");
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_EQ(1, map.size());

  const char *value = (const char *)map.find("foo");
  EXPECT_TRUE(0 == strcmp(value, "bar"));

  error = map.insert("foo", (void *)"baz");
  EXPECT_EQ(ESB_UNIQUENESS_VIOLATION, error);
  EXPECT_EQ(1, map.size());

  value = (const char *)map.find("foo");
  EXPECT_TRUE(0 == strcmp(value, "bar"));
}

TEST(Map, Update) {
  Map map(StringComparator);

  Error error = map.insert("foo", (void *)"bar");
  EXPECT_EQ(ESB_SUCCESS, error);

  const char *value = (const char *)map.find("foo");
  EXPECT_TRUE(0 == strcmp(value, "bar"));

  const char *old = NULL;
  error = map.update("foo", (void *)"baz", (void **)&old);
  EXPECT_EQ(ESB_SUCCESS, error);
  EXPECT_TRUE(0 == strcmp(old, "bar"));

  value = (const char *)map.find("foo");
  EXPECT_TRUE(0 == strcmp(value, "baz"));
}

TEST(Map, Remove) {
  Map map(StringComparator);

  Error error = map.insert("foo", (void *)"bar");
  EXPECT_EQ(ESB_SUCCESS, error);

  const char *value = (const char *)map.find("foo");
  EXPECT_TRUE(0 == strcmp(value, "bar"));

  error = map.remove("foo");
  EXPECT_EQ(ESB_SUCCESS, error);

  value = (const char *)map.find("foo");
  EXPECT_TRUE(NULL == value);
}

TEST(Map, ForwardIterate) {
  Map map(StringComparator);

  Error error = map.insert("foo", (void *)"foo");
  EXPECT_EQ(ESB_SUCCESS, error);
  error = map.insert("bar", (void *)"bar");
  EXPECT_EQ(ESB_SUCCESS, error);
  error = map.insert("baz", (void *)"baz");
  EXPECT_EQ(ESB_SUCCESS, error);

  MapIterator it = map.minimumIterator();
  EXPECT_FALSE(it.isNull());
  EXPECT_TRUE(0 == strcmp("bar", (const char *)it.key()));
  EXPECT_TRUE(0 == strcmp("bar", (const char *)it.value()));
  EXPECT_TRUE(it.hasNext());
  EXPECT_FALSE(it.next().isNull());
  EXPECT_FALSE(it.hasPrevious());
  EXPECT_TRUE(it.previous().isNull());

  ++it;
  EXPECT_FALSE(it.isNull());
  EXPECT_TRUE(0 == strcmp("baz", (const char *)it.key()));
  EXPECT_TRUE(0 == strcmp("baz", (const char *)it.value()));
  EXPECT_TRUE(it.hasNext());
  EXPECT_FALSE(it.next().isNull());
  EXPECT_TRUE(it.hasPrevious());
  EXPECT_FALSE(it.previous().isNull());

  ++it;
  EXPECT_FALSE(it.isNull());
  EXPECT_TRUE(0 == strcmp("foo", (const char *)it.key()));
  EXPECT_TRUE(0 == strcmp("foo", (const char *)it.value()));
  EXPECT_FALSE(it.hasNext());
  EXPECT_TRUE(it.next().isNull());
  EXPECT_TRUE(it.hasPrevious());
  EXPECT_FALSE(it.previous().isNull());

  ++it;
  EXPECT_TRUE(it.isNull());
}

TEST(Map, ReverseIterate) {
  Map map(StringComparator);

  Error error = map.insert("foo", (void *)"foo");
  EXPECT_EQ(ESB_SUCCESS, error);
  error = map.insert("bar", (void *)"bar");
  EXPECT_EQ(ESB_SUCCESS, error);
  error = map.insert("baz", (void *)"baz");
  EXPECT_EQ(ESB_SUCCESS, error);

  MapIterator it = map.maximumIterator();
  EXPECT_FALSE(it.isNull());
  EXPECT_TRUE(0 == strcmp("foo", (const char *)it.key()));
  EXPECT_TRUE(0 == strcmp("foo", (const char *)it.value()));
  EXPECT_FALSE(it.hasNext());
  EXPECT_TRUE(it.next().isNull());
  EXPECT_TRUE(it.hasPrevious());
  EXPECT_FALSE(it.previous().isNull());

  --it;
  EXPECT_FALSE(it.isNull());
  EXPECT_TRUE(0 == strcmp("baz", (const char *)it.key()));
  EXPECT_TRUE(0 == strcmp("baz", (const char *)it.value()));
  EXPECT_TRUE(it.hasNext());
  EXPECT_FALSE(it.next().isNull());
  EXPECT_TRUE(it.hasPrevious());
  EXPECT_FALSE(it.previous().isNull());

  --it;
  EXPECT_FALSE(it.isNull());
  EXPECT_TRUE(0 == strcmp("bar", (const char *)it.key()));
  EXPECT_TRUE(0 == strcmp("bar", (const char *)it.value()));
  EXPECT_TRUE(it.hasNext());
  EXPECT_FALSE(it.next().isNull());
  EXPECT_FALSE(it.hasPrevious());
  EXPECT_TRUE(it.previous().isNull());

  --it;
  EXPECT_TRUE(it.isNull());
}
