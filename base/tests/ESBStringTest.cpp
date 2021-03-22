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

  // By size
  EXPECT_EQ(0, StringWildcardMatch("foo", 3, "foo.everscale.com", 3));

  // Exact matches are less than the most specific wildcard match
  EXPECT_LT(StringWildcardMatch("foo", "foo"), StringWildcardMatch("foo*", "foo"));
  EXPECT_LT(StringWildcardMatch("foo", "foo"), StringWildcardMatch("*foo", "foo"));
}

TEST(String, SplitFqdn) {
  const char *fqdn = "foo.bar.baz";
  const char *hostname = NULL;
  UInt32 hostnameSize = 0U;
  const char *domain = NULL;

  SplitFqdn(fqdn, &hostname, &hostnameSize, &domain);
  EXPECT_EQ(3, hostnameSize);
  EXPECT_EQ(0, memcmp(hostname, "foo", hostnameSize));
  EXPECT_EQ(0, strcmp(domain, "bar.baz"));
}

TEST(String, SplitFqdnHostnameOnly) {
  const char *fqdn = "foobarbaz";
  const char *hostname = NULL;
  UInt32 hostnameSize = 0U;
  const char *domain = NULL;

  SplitFqdn(fqdn, &hostname, &hostnameSize, &domain);
  EXPECT_EQ(9, hostnameSize);
  EXPECT_EQ(0, memcmp(hostname, "foobarbaz", hostnameSize));
  EXPECT_EQ(NULL, domain);
}

TEST(String, SplitFqdnWildcard) {
  const char *fqdn = "f*.bar.baz";
  const char *hostname = NULL;
  UInt32 hostnameSize = 0U;
  const char *domain = NULL;

  SplitFqdn(fqdn, &hostname, &hostnameSize, &domain);
  EXPECT_EQ(2, hostnameSize);
  EXPECT_EQ(0, memcmp(hostname, "f*", hostnameSize));
  EXPECT_EQ(0, strcmp(domain, "bar.baz"));
}
