#ifndef ESB_UNIQUE_ID_H
#define ESB_UNIQUE_ID_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

namespace ESB {

#define ESB_UUID_PRESENTATION_SIZE 37

/** UniqueId is Universally Unique Identifier (UUID) implementation.
 *
 *  @ingroup util
 */
class UniqueId {
 public:
  static Error Parse(const char *buffer, UInt128 *uuid);
  static Error Format(char *buffer, Size size, UInt128 uuid);
  static Error Generate(UInt128 *uuid);

  UniqueId() : _uuid(0) {}
  UniqueId(UInt128 uuid) : _uuid(uuid) {}
  UniqueId(const UniqueId &uuid) : _uuid(uuid._uuid) {}
  virtual ~UniqueId() {}

  inline void set(UInt128 uuid) { _uuid = uuid; }

  inline UInt32 value() const { return _uuid; }

  inline UniqueId &operator=(const UniqueId &uuid) {
    _uuid = uuid._uuid;
    return *this;
  }

  inline bool operator==(const UniqueId &uuid) const { return _uuid == uuid._uuid; }

  inline bool operator>(const UniqueId &uuid) const { return _uuid > uuid._uuid; }

  inline bool operator>=(const UniqueId &uuid) const { return _uuid >= uuid._uuid; }

  inline bool operator<(const UniqueId &uuid) const { return _uuid < uuid._uuid; }

  inline bool operator<=(const UniqueId &uuid) const { return _uuid <= uuid._uuid; }

  inline int compare(const UniqueId &uuid) const { return _uuid == uuid._uuid ? 0 : _uuid < uuid._uuid ? -1 : 1; }

 private:
  UInt128 _uuid;

  ESB_PLACEMENT_NEW(UniqueId);
};

}  // namespace ESB

#endif
