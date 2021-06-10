#ifndef ESB_BUDDY_CACHE_ALLOCATOR_H
#include <ESBBuddyCacheAllocator.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(BuddyCacheAllocator, FailoverAndReset) {
  BuddyCacheAllocator allocator(16384, SystemAllocator::Instance(), SystemAllocator::Instance());

  // Completely fill the cache, without fragmentation and accounting for overhead.
  void *allocations[16];
  memset(allocations, 0, sizeof(allocations));
  const UInt32 numAllocations = sizeof(allocations) / sizeof(void *);
  const Size allocationSize = 1024 - BuddyAllocator::Overhead();

  for (int i = 0; i < numAllocations; ++i) {
    ASSERT_EQ(ESB_SUCCESS, allocator.allocate(allocationSize, &allocations[i]));
    ASSERT_TRUE(allocations[i]);
  }

#ifndef ESB_NO_ALLOC
  ASSERT_EQ(numAllocations * allocationSize, allocator.cacheBytes());
  ASSERT_EQ(0, allocator.failoverBytes());
#endif

  // Additional allocations use failover cache
  void *failoverAllocation = NULL;
  ASSERT_EQ(ESB_SUCCESS, allocator.allocate(allocationSize, &failoverAllocation));
  ASSERT_TRUE(failoverAllocation);

#ifndef ESB_NO_ALLOC
  ASSERT_EQ(numAllocations * allocationSize, allocator.cacheBytes());
  ASSERT_EQ(allocationSize, allocator.failoverBytes());
#endif

  // Return 1 block to the cache, 4 smaller blocks can now be allocated from the free cache

  ASSERT_EQ(ESB_SUCCESS, allocator.deallocate(allocations[7]));
  allocations[7] = NULL;

  void *supplementalAllocations[4];
  memset(supplementalAllocations, 0, sizeof(supplementalAllocations));
  const UInt32 numSupplementalAllocations = sizeof(supplementalAllocations) / sizeof(void *);
  const Size supplementalAllocationSize = 256 - BuddyAllocator::Overhead();

  for (int i = 0; i < numSupplementalAllocations; ++i) {
    ASSERT_EQ(ESB_SUCCESS, allocator.allocate(supplementalAllocationSize, &supplementalAllocations[i]));
    ASSERT_TRUE(supplementalAllocations[i]);
  }

#ifndef ESB_NO_ALLOC
  ASSERT_EQ(numAllocations * allocationSize + numSupplementalAllocations * supplementalAllocationSize,
            allocator.cacheBytes());
  ASSERT_EQ(allocationSize, allocator.failoverBytes());
#endif

  // Cleanup
  ASSERT_EQ(ESB_SUCCESS, allocator.deallocate(failoverAllocation));
  for (int i = 0; i < numAllocations; ++i) {
    if (allocations[i]) {
      ASSERT_EQ(ESB_SUCCESS, allocator.deallocate(allocations[i]));
    }
  }

#ifndef ESB_NO_ALLOC
  // Allocator will refuse to be destroyed since there are still outstanding allocations
  ASSERT_EQ(ESB_IN_USE, allocator.reset());
#endif

  for (int i = 0; i < numSupplementalAllocations; ++i) {
    if (supplementalAllocations[i]) {
      ASSERT_EQ(ESB_SUCCESS, allocator.deallocate(supplementalAllocations[i]));
    }
  }

  // Now allocator can be reset
  ASSERT_EQ(ESB_SUCCESS, allocator.reset());
}
