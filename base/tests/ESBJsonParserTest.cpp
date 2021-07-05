#ifndef ESB_JSON_PARSER_H
#include <ESBJsonParser.h>
#endif

#ifndef ESB_BUDDY_CACHE_ALLOCATOR_H
#include <ESBBuddyCacheAllocator.h>
#endif

#ifndef ESB_BUFFERED_FILE_H
#include <ESBBufferedFile.h>
#endif

#ifndef ESB_AST_COUNTING_CALLBACKS_H
#include "ASTCountingCallbacks.h"
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(JsonParser, SmallDoc) {
  BuddyCacheAllocator allocator(16384, SystemAllocator::Instance(), SystemAllocator::Instance());

  {
    AST::CountingCallbacks callbacks;
    JsonParser parser(callbacks, allocator);
    BufferedFile file("doc1.json", BufferedFile::READ_ONLY);
    unsigned char buffer[128];

    while (true) {
      Size bytesRead = 0;
      Error error = file.read(buffer, sizeof(buffer), &bytesRead);

      if (ESB_SUCCESS == error) {
        ASSERT_EQ(ESB_SUCCESS, parser.parse(buffer, bytesRead));
        continue;
      }

      if (ESB_BREAK == error) {
        ASSERT_EQ(ESB_SUCCESS, parser.parse(buffer, bytesRead));
        ASSERT_EQ(ESB_SUCCESS, parser.end());
        break;
      }

      // Intentionally fail
      ASSERT_EQ(ESB_SUCCESS, error);
    }

    // Assert that all elements were seen
    ASSERT_EQ(4, callbacks.onMapStarts());
    ASSERT_EQ(17, callbacks.onMapKeys());
    ASSERT_EQ(callbacks.onMapEnds(), callbacks.onMapStarts());
    ASSERT_EQ(2, callbacks.onArrayStarts());
    ASSERT_EQ(callbacks.onArrayEnds(), callbacks.onArrayStarts());
    ASSERT_EQ(1, callbacks.onNulls());
    ASSERT_EQ(1, callbacks.onBooleans());
    ASSERT_EQ(1, callbacks.onIntegers());
    ASSERT_EQ(1, callbacks.onDoubles());
    ASSERT_EQ(10, callbacks.onStrings());
  }

#ifndef ESB_NO_ALLOC
  // Assert that only the buddy allocator cache was used
  ASSERT_LT(1024, allocator.cacheBytes());
  ASSERT_EQ(0, allocator.failoverBytes());
#endif
}

TEST(JsonParser, Large) {
  // Note that the cache is the same size as the small test case but still does not spillover to the failover allocator
  BuddyCacheAllocator allocator(16384, SystemAllocator::Instance(), SystemAllocator::Instance());

  {
    AST::CountingCallbacks callbacks;
    JsonParser parser(callbacks, allocator);
    BufferedFile file("doc2.json", BufferedFile::READ_ONLY);
    unsigned char buffer[128];

    while (true) {
      Size bytesRead = 0;
      Error error = file.read(buffer, sizeof(buffer), &bytesRead);

      if (ESB_SUCCESS == error) {
        ASSERT_EQ(ESB_SUCCESS, parser.parse(buffer, bytesRead));
        continue;
      }

      if (ESB_BREAK == error) {
        ASSERT_EQ(ESB_SUCCESS, parser.parse(buffer, bytesRead));
        ASSERT_EQ(ESB_SUCCESS, parser.end());
        break;
      }

      // Intentionally fail
      ASSERT_EQ(ESB_SUCCESS, error);
    }

    // Assert that all elements were seen
    ASSERT_EQ(60, callbacks.onMapStarts());
    ASSERT_EQ(255, callbacks.onMapKeys());
    ASSERT_EQ(callbacks.onMapEnds(), callbacks.onMapStarts());
    ASSERT_EQ(31, callbacks.onArrayStarts());
    ASSERT_EQ(callbacks.onArrayEnds(), callbacks.onArrayStarts());
    ASSERT_EQ(15, callbacks.onNulls());
    ASSERT_EQ(15, callbacks.onBooleans());
    ASSERT_EQ(15, callbacks.onIntegers());
    ASSERT_EQ(15, callbacks.onDoubles());
    ASSERT_EQ(150, callbacks.onStrings());
  }

#ifndef ESB_NO_ALLOC
  // Assert that only the buddy allocator cache was used
  ASSERT_LT(1024, allocator.cacheBytes());
  ASSERT_EQ(0, allocator.failoverBytes());
#endif
}

TEST(JsonParser, LargeFailover) {
  // Note that the cache is smaller for this test case, which forces failover.
  BuddyCacheAllocator allocator(8192, SystemAllocator::Instance(), SystemAllocator::Instance());

  {
    AST::CountingCallbacks callbacks;
    JsonParser parser(callbacks, allocator);
    BufferedFile file("doc2.json", BufferedFile::READ_ONLY);
    unsigned char buffer[128];

    while (true) {
      Size bytesRead = 0;
      Error error = file.read(buffer, sizeof(buffer), &bytesRead);

      if (ESB_SUCCESS == error) {
        ASSERT_EQ(ESB_SUCCESS, parser.parse(buffer, bytesRead));
        continue;
      }

      if (ESB_BREAK == error) {
        ASSERT_EQ(ESB_SUCCESS, parser.parse(buffer, bytesRead));
        ASSERT_EQ(ESB_SUCCESS, parser.end());
        break;
      }

      // Intentionally fail
      ASSERT_EQ(ESB_SUCCESS, error);
    }

    // Assert that all elements were seen
    ASSERT_EQ(60, callbacks.onMapStarts());
    ASSERT_EQ(255, callbacks.onMapKeys());
    ASSERT_EQ(callbacks.onMapEnds(), callbacks.onMapStarts());
    ASSERT_EQ(31, callbacks.onArrayStarts());
    ASSERT_EQ(callbacks.onArrayEnds(), callbacks.onArrayStarts());
    ASSERT_EQ(15, callbacks.onNulls());
    ASSERT_EQ(15, callbacks.onBooleans());
    ASSERT_EQ(15, callbacks.onIntegers());
    ASSERT_EQ(15, callbacks.onDoubles());
    ASSERT_EQ(150, callbacks.onStrings());
  }

#ifndef ESB_NO_ALLOC
  // Assert that the failover allocator was used
  ASSERT_LT(1024, allocator.cacheBytes());
  ASSERT_LT(1024, allocator.failoverBytes());
#endif
}