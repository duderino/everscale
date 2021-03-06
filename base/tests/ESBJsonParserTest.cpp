#ifndef ESB_JSON_PARSER_H
#include <ESBJsonParser.h>
#endif

#ifndef ESB_BUDDY_CACHE_ALLOCATOR_H
#include <ESBBuddyCacheAllocator.h>
#endif

#ifndef ESB_BUFFERED_FILE_H
#include <ESBBufferedFile.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

class CountingJsonParser : public JsonParser {
 public:
  CountingJsonParser(Allocator &allocator = SystemAllocator::Instance())
      : JsonParser(allocator),
        _onMapStarts(0),
        _onMapKeys(0),
        _onMapEnds(0),
        _onArrayStarts(0),
        _onArrayEnds(0),
        _onNulls(0),
        _onBooleans(0),
        _onIntegers(0),
        _onDoubles(0),
        _onStrings(0) {}
  virtual ~CountingJsonParser() {}

  virtual ParseControl onMapStart() {
    ++_onMapStarts;
    return CONTINUE;
  }

  virtual ParseControl onMapKey(const unsigned char *key, UInt32 length) {
    ++_onMapKeys;
    return CONTINUE;
  }

  virtual ParseControl onMapEnd() {
    ++_onMapEnds;
    return CONTINUE;
  }

  virtual ParseControl onArrayStart() {
    ++_onArrayStarts;
    return CONTINUE;
  }

  virtual ParseControl onArrayEnd() {
    ++_onArrayEnds;
    return CONTINUE;
  }

  virtual ParseControl onNull() {
    ++_onNulls;
    return CONTINUE;
  }

  virtual ParseControl onBoolean(bool value) {
    ++_onBooleans;
    return CONTINUE;
  }

  virtual ParseControl onInteger(Int64 value) {
    ++_onIntegers;
    return CONTINUE;
  }

  virtual ParseControl onDouble(double value) {
    ++_onDoubles;
    return CONTINUE;
  }

  virtual ParseControl onString(const unsigned char *value, UInt32 length) {
    ++_onStrings;
    return CONTINUE;
  }

  inline int onMapStarts() const { return _onMapStarts; }
  inline int onMapKeys() const { return _onMapKeys; }
  inline int onMapEnds() const { return _onMapEnds; }
  inline int onArrayStarts() const { return _onArrayStarts; }
  inline int onArrayEnds() const { return _onArrayEnds; }
  inline int onNulls() const { return _onNulls; }
  inline int onBooleans() const { return _onBooleans; }
  inline int onIntegers() const { return _onIntegers; }
  inline int onDoubles() const { return _onDoubles; }
  inline int onStrings() const { return _onStrings; }

 private:
  int _onMapStarts;
  int _onMapKeys;
  int _onMapEnds;
  int _onArrayStarts;
  int _onArrayEnds;
  int _onNulls;
  int _onBooleans;
  int _onIntegers;
  int _onDoubles;
  int _onStrings;

  ESB_DEFAULT_FUNCS(CountingJsonParser);
};

TEST(JsonParser, SmallDoc) {
  BuddyCacheAllocator allocator(16384, SystemAllocator::Instance(), SystemAllocator::Instance());

  {
    CountingJsonParser parser(allocator);
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
    ASSERT_EQ(4, parser.onMapStarts());
    ASSERT_EQ(17, parser.onMapKeys());
    ASSERT_EQ(parser.onMapEnds(), parser.onMapStarts());
    ASSERT_EQ(2, parser.onArrayStarts());
    ASSERT_EQ(parser.onArrayEnds(), parser.onArrayStarts());
    ASSERT_EQ(1, parser.onNulls());
    ASSERT_EQ(1, parser.onBooleans());
    ASSERT_EQ(1, parser.onIntegers());
    ASSERT_EQ(1, parser.onDoubles());
    ASSERT_EQ(10, parser.onStrings());
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
    CountingJsonParser parser(allocator);
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
    ASSERT_EQ(60, parser.onMapStarts());
    ASSERT_EQ(255, parser.onMapKeys());
    ASSERT_EQ(parser.onMapEnds(), parser.onMapStarts());
    ASSERT_EQ(31, parser.onArrayStarts());
    ASSERT_EQ(parser.onArrayEnds(), parser.onArrayStarts());
    ASSERT_EQ(15, parser.onNulls());
    ASSERT_EQ(15, parser.onBooleans());
    ASSERT_EQ(15, parser.onIntegers());
    ASSERT_EQ(15, parser.onDoubles());
    ASSERT_EQ(150, parser.onStrings());
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
    CountingJsonParser parser(allocator);
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
    ASSERT_EQ(60, parser.onMapStarts());
    ASSERT_EQ(255, parser.onMapKeys());
    ASSERT_EQ(parser.onMapEnds(), parser.onMapStarts());
    ASSERT_EQ(31, parser.onArrayStarts());
    ASSERT_EQ(parser.onArrayEnds(), parser.onArrayStarts());
    ASSERT_EQ(15, parser.onNulls());
    ASSERT_EQ(15, parser.onBooleans());
    ASSERT_EQ(15, parser.onIntegers());
    ASSERT_EQ(15, parser.onDoubles());
    ASSERT_EQ(150, parser.onStrings());
  }

#ifndef ESB_NO_ALLOC
  // Assert that the failover allocator was used
  ASSERT_LT(1024, allocator.cacheBytes());
  ASSERT_LT(1024, allocator.failoverBytes());
#endif
}