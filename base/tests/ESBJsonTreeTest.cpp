#ifndef ESB_JSON_PARSER_H
#include <ESBJsonParser.h>
#endif

#ifndef ESB_BUFFERED_FILE_H
#include <ESBBufferedFile.h>
#endif

#ifndef ESB_AST_TREE_H
#include <ASTTree.h>
#endif

#ifndef ESB_AST_COUNTING_CALLBACKS_H
#include "ASTCountingCallbacks.h"
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(Tree, SmallDoc) {
  AST::Tree tree;
  JsonParser parser(tree);
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

  ASSERT_EQ(ESB_SUCCESS, tree.result());

  AST::CountingCallbacks callbacks;
  tree.traverse(callbacks);

  // Assert that all elements were seen
  ASSERT_EQ(4, callbacks.onMapStarts());
  ASSERT_EQ(callbacks.onMapEnds(), callbacks.onMapStarts());
  ASSERT_EQ(2, callbacks.onArrayStarts());
  ASSERT_EQ(callbacks.onArrayEnds(), callbacks.onArrayStarts());
  ASSERT_EQ(1, callbacks.onNulls());
  ASSERT_EQ(1, callbacks.onBooleans());
  ASSERT_EQ(1, callbacks.onIntegers());
  ASSERT_EQ(1, callbacks.onDoubles());
  ASSERT_EQ(27, callbacks.onStrings());
}

TEST(Tree, Large) {
  AST::Tree tree;
  JsonParser parser(tree);
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

  ASSERT_EQ(ESB_SUCCESS, tree.result());

  AST::CountingCallbacks callbacks;
  tree.traverse(callbacks);

  // Assert that all elements were seen
  ASSERT_EQ(60, callbacks.onMapStarts());
  ASSERT_EQ(callbacks.onMapEnds(), callbacks.onMapStarts());
  ASSERT_EQ(31, callbacks.onArrayStarts());
  ASSERT_EQ(callbacks.onArrayEnds(), callbacks.onArrayStarts());
  ASSERT_EQ(15, callbacks.onNulls());
  ASSERT_EQ(15, callbacks.onBooleans());
  ASSERT_EQ(15, callbacks.onIntegers());
  ASSERT_EQ(15, callbacks.onDoubles());
  ASSERT_EQ(255 + 150, callbacks.onStrings());
}
