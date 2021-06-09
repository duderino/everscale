#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(DiscardAllocator, AllocMultipleChunks) {
  uint chunkSize = 4096;
  DiscardAllocator allocator(chunkSize, sizeof(ESB::Word), 1, SystemAllocator::Instance(), true);

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      void *block = NULL;
      Error error = allocator.allocate(chunkSize / 10, &block);
      ASSERT_EQ(ESB_SUCCESS, error);
      ASSERT_TRUE(block);
    }
  }
}

TEST(DiscardAllocator, AllocExtraBigChunks) {
  uint chunkSize = 4096;
  DiscardAllocator allocator(chunkSize, sizeof(ESB::Word), 1, SystemAllocator::Instance(), true);

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      void *block = NULL;
      Error error = allocator.allocate(chunkSize * 2, &block);
      ASSERT_EQ(ESB_SUCCESS, error);
      ASSERT_TRUE(block);
    }
  }
}

TEST(DiscardAllocator, AllocMultipleChunksCacheLineAligned) {
  uint chunkSize = 4096;
  DiscardAllocator allocator(chunkSize, ESB_CACHE_LINE_SIZE, 1, SystemAllocator::Instance(), true);

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      void *block = NULL;
      Error error = allocator.allocate(chunkSize / 10, &block);
      ASSERT_EQ(ESB_SUCCESS, error);
      ASSERT_TRUE(block);
    }
  }
}

TEST(DiscardAllocator, AllocExtraBigChunksCacheLineAligned) {
  uint chunkSize = 4096;
  DiscardAllocator allocator(chunkSize, ESB_CACHE_LINE_SIZE, 1, SystemAllocator::Instance(), true);

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      void *block = NULL;
      Error error = allocator.allocate(chunkSize * 2, &block);
      ASSERT_EQ(ESB_SUCCESS, error);
      ASSERT_TRUE(block);
    }
  }
}

TEST(DiscardAllocator, Alignment) {
  ASSERT_EQ(0, ESB_ALIGN(0, 64));
  ASSERT_EQ(64, ESB_ALIGN(1, 64));
  ASSERT_EQ(64, ESB_ALIGN(63, 64));
  ASSERT_EQ(64, ESB_ALIGN(64, 64));
  ASSERT_EQ(128, ESB_ALIGN(65, 64));
  ASSERT_EQ(0, ESB_WORD_ALIGN(0));
  ASSERT_EQ(sizeof(ESB::Word), ESB_WORD_ALIGN(1));
  ASSERT_EQ(64 + sizeof(ESB::Word), ESB_WORD_ALIGN(65));
}
