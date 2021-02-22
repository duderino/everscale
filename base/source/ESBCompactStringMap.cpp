#ifndef ESB_COMPACT_STRING_MAP_H
#include <ESBCompactStringMap.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#error "Need string.h or equivalent"
#endif

namespace ESB {

CompactStringMap::CompactStringMap(UInt32 initialCapacity) {}

CompactStringMap::~CompactStringMap() {}

Error CompactStringMap::insert(const char *key, void *value, int *position) { return 0; }

Error CompactStringMap::insert(const char *key, int keySize, void *value, int *position) { return 0; }

Error CompactStringMap::remove(const char *key) { return 0; }

Error CompactStringMap::remove(const char *key, int keySize) { return 0; }

Error CompactStringMap::remove(int position) { return 0; }

void *CompactStringMap::find(const char *key) const { return nullptr; }

void *CompactStringMap::find(const char *key, int keySize) const { return nullptr; }

void *CompactStringMap::find(int position) const { return nullptr; }

Error CompactStringMap::update(const char *key, void *value, void **old) { return 0; }

Error CompactStringMap::update(const char *key, int keySize, void *value, void **old) { return 0; }

Error CompactStringMap::update(int position, void *value, void **old) { return 0; }

Error CompactStringMap::clear() { return 0; }

Error CompactStringMap::next(const char **key, int *keySize, void **value, UInt32 *marker) const { return 0; }

}  // namespace ESB
