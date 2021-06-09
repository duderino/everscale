#ifndef ESB_COMMON_H
#define ESB_COMMON_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_ERRNO_H
#include <ESBError.h>
#endif

namespace ESB {

#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define ESB_PLACEMENT_NEW(CLASS)                                               \
 public:                                                                       \
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept { \
    void *block = NULL;                                                        \
    return ESB_SUCCESS == allocator.allocate(size, &block) ? block : NULL;     \
  }                                                                            \
  inline void *operator new(size_t size, CLASS *block) noexcept { return block; }

#define ESB_DEFAULT_FUNCS(CLASS)                                                          \
 public:                                                                                  \
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept {            \
    void *block = NULL;                                                                   \
    return ESB_SUCCESS == allocator.allocate(size, &block) ? block : NULL;                \
  }                                                                                       \
  inline void *operator new(size_t size, CLASS *block) noexcept { return block; }         \
  inline void *operator new(size_t size, unsigned char *block) noexcept { return block; } \
                                                                                          \
 private:                                                                                 \
  CLASS(const CLASS &);                                                                   \
  void operator=(const CLASS &);

typedef struct {
  unsigned char *_data;
  UInt32 _capacity;
} SizedBuffer;

}  // namespace ESB

#endif
