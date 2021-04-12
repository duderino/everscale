#ifndef ESB_SHARED_EMBEDDED_MAP_H
#include <ESBSharedEmbeddedMap.h>
#endif

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_RAND_H
#include <ESBRand.h>
#endif

#ifndef ESB_MAP_H
#include <ESBMap.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

class SocketAddressCallbacks : public EmbeddedMapCallbacks {
 public:
  SocketAddressCallbacks(Allocator &allocator) : _allocator(allocator) {}

  virtual int compare(const void *f, const void *s) const {
    SocketAddress *first = (SocketAddress *)f;
    SocketAddress *second = (SocketAddress *)s;

    // TODO not IPv6 safe
    return memcmp(first->primitiveAddress(), second->primitiveAddress(), sizeof(SocketAddress::Address));
  }

  virtual UInt64 hash(const void *key) const {
    SocketAddress *addr = (SocketAddress *)key;
    // TODO not IPv6 safe, for IPv6 take the low order bits of the address
    ESB::UInt64 hash = addr->primitiveAddress()->sin_addr.s_addr;
    hash |= (UInt64)addr->primitiveAddress()->sin_port << 32;
    hash |= (UInt64)addr->type() << 48;
    return hash;
  }

  virtual void cleanup(EmbeddedMapElement *element) {
    element->~EmbeddedMapElement();
    _allocator.deallocate(element);
  }

 private:
  // Disabled
  SocketAddressCallbacks(const SocketAddressCallbacks &);
  SocketAddressCallbacks &operator=(const SocketAddressCallbacks &);

  Allocator &_allocator;
};

class FauxConnection : public EmbeddedMapElement {
 public:
  FauxConnection(SocketAddress &address, CleanupHandler &cleanupHandler)
      : _cleanupHandler(cleanupHandler), _address(address) {}

  inline SocketAddress &getAddress() { return _address; }

  virtual const void *key() const { return &_address; }

  virtual CleanupHandler *cleanupHandler() { return &_cleanupHandler; }

 private:
  CleanupHandler &_cleanupHandler;
  SocketAddress _address;
};

static void randomSocketAddress(SocketAddress *out, Rand &rand) {
  char dottedIP[16];
  snprintf(dottedIP, sizeof(dottedIP), "%d:%d:%d:%d", rand.generate(0, 255), rand.generate(0, 255),
           rand.generate(0, 255), rand.generate(0, 255));
  SocketAddress randAddr(dottedIP, (UInt16)rand.generate(1, 65536), SocketAddress::TCP);
  *out = randAddr;
}

TEST(SharedEmbeddedMap, InsertFindRemove) {
  Rand rand;
  const UInt32 buckets = 541;
  const UInt32 locks = 11;
  Allocator &allocator = SystemAllocator::Instance();
  SocketAddressCallbacks callbacks(allocator);
  SharedEmbeddedMap map(callbacks, buckets, locks, allocator);
  SocketAddress addresses[10000];
  const UInt32 numAddresses = sizeof(addresses) / sizeof(SocketAddress);
  const double expectedElementsPerBucket = ((double)numAddresses) / buckets;

  //
  // The randomly generated socket addresses can put duplicates in the
  // SharedEmbeddedMap.  Removing an element with duplicates from a
  // SharedEmbeddedMap only removes the first occurrence.  This map tracks the
  // number of occurrences of each socket address so we can tell when an
  // element should no longer be present in the SharedEmbeddedMap.
  //
  Map occurrences(callbacks);

  // Insert

  for (int i = 0; i < numAddresses; ++i) {
    randomSocketAddress(&addresses[i], rand);
    Error error = map.insert(new (allocator) FauxConnection(addresses[i], allocator.cleanupHandler()));
    EXPECT_EQ(ESB_SUCCESS, error);

    if (occurrences.find(&addresses[i])) {
      UWord value = (UWord)occurrences.find(&addresses[i]);
      occurrences.update(&addresses[i], (void *)(value + 1), NULL);
    } else {
      occurrences.insert(&addresses[i], (void *)1);
    }
  }

  EXPECT_EQ(numAddresses, map.size());
  double chiSquared = 0.0;
  EXPECT_TRUE(map.validate(&chiSquared));
  fprintf(stderr, "chi^2 %f for expected %f elements per bucket\n", chiSquared, expectedElementsPerBucket);

  // Find returns inserted element

  for (int i = 0; i < numAddresses; ++i) {
    FauxConnection *connection = (FauxConnection *)map.find(&addresses[i]);
    EXPECT_TRUE(NULL != connection);
    EXPECT_TRUE(0 == memcmp(addresses[i].primitiveAddress(), connection->getAddress().primitiveAddress(),
                            sizeof(SocketAddress::Address)));
  }

  // Delete even elements

  for (int i = 0; i < numAddresses; i += 2) {
    FauxConnection *connection = (FauxConnection *)map.remove(&addresses[i]);
    EXPECT_TRUE(NULL != connection);
    EXPECT_TRUE(0 == memcmp(addresses[i].primitiveAddress(), connection->getAddress().primitiveAddress(),
                            sizeof(SocketAddress::Address)));
    EXPECT_TRUE(connection->cleanupHandler());
    connection->cleanupHandler()->destroy(connection);

    UWord value = (UWord)occurrences.find(&addresses[i]);
    occurrences.update(&addresses[i], (void *)(value - 1), NULL);
  }

  EXPECT_EQ(numAddresses / 2, map.size());
  EXPECT_TRUE(map.validate(&chiSquared));
  fprintf(stderr, "chi^2 %f for expected %f elements per bucket\n", chiSquared, expectedElementsPerBucket);

  // Find only finds elements that have were not deleted or have no duplicates

  for (int i = 0; i < 10; ++i) {
    if (0 < ((UWord)occurrences.find(&addresses[i]))) {
      FauxConnection *connection = (FauxConnection *)map.find(&addresses[i]);
      EXPECT_TRUE(NULL != connection);
      EXPECT_TRUE(0 == memcmp(addresses[i].primitiveAddress(), connection->getAddress().primitiveAddress(),
                              sizeof(SocketAddress::Address)));
    } else {
      FauxConnection *connection = (FauxConnection *)map.find(&addresses[i]);
      if (connection) {
        fprintf(stderr, "WTF: %d\n", i);
      }
      EXPECT_EQ(NULL, connection);
    }
  }

  // Let the map go out of scope with half the elements.  It should cleanup
  // automatically using cleanup callbacks.  If it doesn't the leak detector
  // should catch it.
}
