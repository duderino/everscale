#ifndef ESB_STRING_H
#include <ESBString.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(String, WildcardMatch) {
  // -1 means no match
  // 0+ means match.  The smaller the value, the more specific the match.

  // Validation
  EXPECT_EQ(-1, StringWildcardMatch(NULL, "foo"));
  EXPECT_EQ(-1, StringWildcardMatch("foo", NULL));
  EXPECT_EQ(-1, StringWildcardMatch(NULL, NULL));

  // Mismatches
  EXPECT_EQ(-1, StringWildcardMatch("foo", "bar"));
  EXPECT_EQ(-1, StringWildcardMatch("foo", ""));
  EXPECT_EQ(-1, StringWildcardMatch("", "foo"));

  // Exact matches
  EXPECT_EQ(0, StringWildcardMatch("foo", "foo"));
  EXPECT_EQ(0, StringWildcardMatch("", ""));

  // Wildcard matches
  EXPECT_EQ(4, StringWildcardMatch("*", "foo"));

  EXPECT_EQ(1, StringWildcardMatch("foo*", "foo"));
  EXPECT_EQ(4, StringWildcardMatch("foo*", "foobar"));
  EXPECT_EQ(-1, StringWildcardMatch("foo*", "bar"));

  EXPECT_EQ(1, StringWildcardMatch("*bar", "bar"));
  EXPECT_EQ(4, StringWildcardMatch("*bar", "foobar"));
  EXPECT_EQ(-1, StringWildcardMatch("*bar", "foo"));

  EXPECT_EQ(4, StringWildcardMatch("foo*baz", "foobarbaz"));
  EXPECT_EQ(8, StringWildcardMatch("f*z", "foobarbaz"));

  // Exact matches are less than the most specific wildcard match
  EXPECT_LT(StringWildcardMatch("foo", "foo"), StringWildcardMatch("foo*", "foo"));
  EXPECT_LT(StringWildcardMatch("foo", "foo"), StringWildcardMatch("*foo", "foo"));
}
