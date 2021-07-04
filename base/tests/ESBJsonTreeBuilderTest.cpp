#ifndef ESB_JSON_PARSER_H
#include <ESBJsonParser.h>
#endif

#ifndef ESB_BUDDY_CACHE_ALLOCATOR_H
#include <ESBBuddyCacheAllocator.h>
#endif

#ifndef ESB_BUFFERED_FILE_H
#include <ESBBufferedFile.h>
#endif

#ifndef ESB_JSON_TREE_BUILDER_H
#include <ESBJsonTreeBuilder.h>
#endif

#ifndef ESB_JSON_COUNTING_CALLBACKS_H
#include "ESBJsonCountingCallbacks.h"
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(JsonTreeBuilder, SmallDoc) {
  JsonTreeBuilder treeBuilder;
  JsonParser parser(treeBuilder);
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

  ASSERT_EQ(ESB_SUCCESS, treeBuilder.result());

  JsonCountingCallbacks callbacks;
  treeBuilder.traverse(callbacks);

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

TEST(JsonTreeBuilder, Large) {
  JsonTreeBuilder treeBuilder;
  JsonParser parser(treeBuilder);
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

  ASSERT_EQ(ESB_SUCCESS, treeBuilder.result());

  JsonCountingCallbacks callbacks;
  treeBuilder.traverse(callbacks);

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
