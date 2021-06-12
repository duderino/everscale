#ifndef ESB_BUDDY_ALLOCATOR_H
#include <ESBBuddyAllocator.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(BuddyAllocator, Allocate) {
  UInt32 allocSizes[] = {64, 4096, 8192, 64, 128, 1024, 2048, 256, 512};
  void *allocations[sizeof(allocSizes) / sizeof(UInt32)];
  memset(allocations, 0, sizeof(allocations));
  UInt32 numAllocations = sizeof(allocSizes) / sizeof(UInt32);

  BuddyAllocator allocator(16384, SystemAllocator::Instance());

  for (int i = 0; i < numAllocations; ++i) {
    ASSERT_EQ(ESB_SUCCESS, allocator.allocate(allocSizes[i] - ESB::BuddyAllocator::Overhead(), &allocations[i]));
    ASSERT_TRUE(&allocations[i]);
  }

  void *block = NULL;
  ASSERT_EQ(ESB_OUT_OF_MEMORY, allocator.allocate(1, &block));

  for (int i = 0; i < numAllocations; ++i) {
    ASSERT_EQ(ESB_SUCCESS, allocator.deallocate(allocations[i]));
  }
}

TEST(BuddyAllocator, Reset) {
  UInt32 allocSizes[] = {64, 4096, 8192, 64, 128, 1024, 2048, 256, 512};
  void *allocations[sizeof(allocSizes) / sizeof(UInt32)];
  memset(allocations, 0, sizeof(allocations));
  UInt32 numAllocations = sizeof(allocSizes) / sizeof(UInt32);

  BuddyAllocator allocator(16384, SystemAllocator::Instance());

  for (int i = 0; i < numAllocations; ++i) {
    ASSERT_EQ(ESB_SUCCESS, allocator.allocate(allocSizes[i] - ESB::BuddyAllocator::Overhead(), &allocations[i]));
    ASSERT_TRUE(&allocations[i]);
  }

  void *block = NULL;
  ASSERT_EQ(ESB_OUT_OF_MEMORY, allocator.allocate(1, &block));

  ASSERT_EQ(ESB_IN_USE, allocator.reset());

  for (int i = 0; i < numAllocations; ++i) {
    ASSERT_EQ(ESB_SUCCESS, allocator.deallocate(allocations[i]));
  }

  ASSERT_EQ(ESB_SUCCESS, allocator.reset());
}

TEST(BuddyAllocator, ReallocGrow) {
  UInt32 allocSizes[] = {64, 4096, 128, 256, 256, 128, 2048, 4096, 2048, 1024, 512, 256};
  void *allocations[sizeof(allocSizes) / sizeof(UInt32)];
  memset(allocations, 0, sizeof(allocations));
  UInt32 numAllocations = sizeof(allocSizes) / sizeof(UInt32);

  BuddyAllocator allocator(32768, SystemAllocator::Instance());

  // Allocate half, minus internal overhead
  for (int i = 0; i < numAllocations; ++i) {
    ASSERT_EQ(ESB_SUCCESS, allocator.allocate(allocSizes[i] - ESB::BuddyAllocator::Overhead(), &allocations[i]));
    ASSERT_TRUE(&allocations[i]);
  }

  ASSERT_TRUE(allocator.reallocates());

  // ~Double all allocations via realloc

  for (int i = 0; i < numAllocations; ++i) {
    UInt32 allocSize = allocSizes[i] * 2 - ESB::BuddyAllocator::Overhead();
    ASSERT_GE(allocSize, 1);
    ASSERT_EQ(ESB_SUCCESS, allocator.reallocate(allocations[i], allocSizes[i] * 2 - ESB::BuddyAllocator::Overhead(),
                                                &allocations[i]));
    ASSERT_TRUE(&allocations[i]);
  }

  void *block = NULL;
  ASSERT_EQ(ESB_OUT_OF_MEMORY, allocator.allocate(2048, &block));

  for (int i = 0; i < numAllocations; ++i) {
    ASSERT_EQ(ESB_SUCCESS, allocator.deallocate(allocations[i]));
  }
}

TEST(BuddyAllocator, ReallocShrink) {
  UInt32 allocSizes[] = {64, 4096, 128, 256, 256, 128, 2048, 4096, 2048, 1024, 512, 256};
  void *allocations[sizeof(allocSizes) / sizeof(UInt32)];
  memset(allocations, 0, sizeof(allocations));
  UInt32 numAllocations = sizeof(allocSizes) / sizeof(UInt32);

  BuddyAllocator allocator(32768, SystemAllocator::Instance());

  // Allocate half, minus internal overhead
  for (int i = 0; i < numAllocations; ++i) {
    ASSERT_EQ(ESB_SUCCESS, allocator.allocate(allocSizes[i] - ESB::BuddyAllocator::Overhead(), &allocations[i]));
    ASSERT_TRUE(&allocations[i]);
  }

  ASSERT_TRUE(allocator.reallocates());

  // ~Halve all allocations via realloc
  for (int i = 0; i < numAllocations; ++i) {
    ASSERT_EQ(ESB_SUCCESS, allocator.reallocate(allocations[i], allocSizes[i] / 2 - ESB::BuddyAllocator::Overhead(),
                                                &allocations[i]));
    ASSERT_TRUE(&allocations[i]);
  }

  // Use the other half

  void *moreAllocations[sizeof(allocSizes) / sizeof(UInt32)];
  for (int i = 0; i < numAllocations; ++i) {
    ASSERT_EQ(ESB_SUCCESS, allocator.allocate(allocSizes[i] - ESB::BuddyAllocator::Overhead(), &moreAllocations[i]));
    ASSERT_TRUE(&moreAllocations[i]);
  }

  for (int i = 0; i < numAllocations; ++i) {
    ASSERT_EQ(ESB_SUCCESS, allocator.deallocate(allocations[i]));
    ASSERT_EQ(ESB_SUCCESS, allocator.deallocate(moreAllocations[i]));
  }
}