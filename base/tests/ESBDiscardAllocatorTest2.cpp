#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(DiscardAllocator, AllocMultipleChunks) {
  uint chunkSize = 4906;
  DiscardAllocator allocator(chunkSize, SystemAllocator::GetInstance());

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      void *block = allocator.allocate(chunkSize / 10);
      EXPECT_TRUE(block);
    }
  }
}

TEST(DiscardAllocator, AllocExtraBigChunks) {
  uint chunkSize = 4906;
  DiscardAllocator allocator(chunkSize, SystemAllocator::GetInstance());

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 10; ++j) {
      void *block = allocator.allocate(chunkSize * 2);
      EXPECT_TRUE(block);
    }
  }
}
