#ifndef ESB_UNIQUE_ID_H
#include <ESBUniqueId.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(UniqueId, ParseFormat) {
  const char *input = "123e4567-e89b-12d3-a456-426614174000";
  char output[ESB_UUID_PRESENTATION_SIZE];
  UInt128 uuid = 0;

  ASSERT_EQ(ESB_SUCCESS, UniqueId::Parse(input, &uuid));
  ASSERT_EQ(ESB_SUCCESS, UniqueId::Format(output, sizeof(output), uuid));

  ASSERT_EQ(0, strcasecmp(input, output));
}

TEST(UniqueId, Compare) {
  const char *input = "123e4567-e89b-12d3-a456-426614174000";
  UInt128 uuid = 0;

  ASSERT_EQ(ESB_SUCCESS, UniqueId::Parse(input, &uuid));

  UniqueId middle(uuid);
  UniqueId greater(uuid + 42);
  UniqueId lesser(uuid - 42);

  ASSERT_FALSE(middle == greater);
  ASSERT_FALSE(middle == lesser);
  ASSERT_TRUE(middle == middle);

  ASSERT_TRUE(middle < greater);
  ASSERT_TRUE(middle <= greater);
  ASSERT_FALSE(middle > greater);
  ASSERT_FALSE(middle >= greater);
  ASSERT_FALSE(middle < middle);
  ASSERT_TRUE(middle <= middle);
  ASSERT_FALSE(middle > middle);
  ASSERT_TRUE(middle >= middle);

  ASSERT_TRUE(middle > lesser);
  ASSERT_TRUE(middle >= lesser);
  ASSERT_FALSE(middle < lesser);
  ASSERT_FALSE(middle <= lesser);
  ASSERT_FALSE(middle > middle);
  ASSERT_TRUE(middle >= middle);
  ASSERT_FALSE(middle < middle);
  ASSERT_TRUE(middle <= middle);

  ASSERT_EQ(-1, middle.compare(greater));
  ASSERT_EQ(1, greater.compare(middle));

  ASSERT_EQ(1, middle.compare(lesser));
  ASSERT_EQ(-1, lesser.compare(middle));

  ASSERT_EQ(0, middle.compare(middle));
}

TEST(UniqueId, Generate) {
  UniqueId uuids[3];
  const int length = sizeof(uuids) / sizeof(UniqueId);

  for (int i = 0; i < length; ++i) {
    UInt128 id = 0;
    ASSERT_EQ(ESB_SUCCESS, UniqueId::Generate(&id));
    uuids[i].set(id);
  }

  for (int i = 0; i < length; ++i) {
    for (int j = 0; j < length; ++j) {
      if (i == j) {
        ASSERT_EQ(0, uuids[i].compare(uuids[j]));
      } else {
        ASSERT_NE(0, uuids[i].compare(uuids[j]));
      }
    }
  }
}

TEST(UniqueId, ParseTooShort) {
  const char *input = "123e4567-e89b-12d3-a456-42661417400";
  UInt128 uuid = 0;

  ASSERT_EQ(ESB_CANNOT_PARSE, UniqueId::Parse(input, &uuid));
}

TEST(UniqueId, ParseBadChar) {
  const char *input = "123e4567-e89b-12d3-a456-42661417400Z";
  UInt128 uuid = 0;

  ASSERT_EQ(ESB_CANNOT_PARSE, UniqueId::Parse(input, &uuid));
}

TEST(UniqueId, ParseBadFormat) {
  const char *input = "123e4567-e-89b-12d3-a456-426614174000";
  UInt128 uuid = 0;

  ASSERT_EQ(ESB_CANNOT_PARSE, UniqueId::Parse(input, &uuid));
}