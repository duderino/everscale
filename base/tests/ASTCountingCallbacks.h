#ifndef ESB_SOCKET_TEST_H
#define ESB_SOCKET_TEST_H

#ifndef ESB_AST_CALLBACKS_H
#include <ASTCallbacks.h>
#endif

namespace ESB {
namespace AST {

class CountingCallbacks : public Callbacks {
 public:
  CountingCallbacks()
      : _onMapStarts(0),
        _onMapKeys(0),
        _onMapEnds(0),
        _onArrayStarts(0),
        _onArrayEnds(0),
        _onNulls(0),
        _onBooleans(0),
        _onIntegers(0),
        _onDoubles(0),
        _onStrings(0) {}
  virtual ~CountingCallbacks() {}

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

  virtual ParseControl onListStart() {
    ++_onArrayStarts;
    return CONTINUE;
  }

  virtual ParseControl onListEnd() {
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

  ESB_DEFAULT_FUNCS(CountingCallbacks);
};

}  // namespace AST
}  // namespace ESB

#endif