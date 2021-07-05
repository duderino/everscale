#ifndef ESB_AST_CALLBACKS_H
#define ESB_AST_CALLBACKS_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

namespace ESB {
namespace AST {

/**
 *  Parser callbacks for building abstract symbol trees
 *
 *  @ingroup ast
 */
class Callbacks {
 public:
  Callbacks();

  virtual ~Callbacks();

  typedef enum { BREAK = 0, CONTINUE = 1 } ParseControl;

  virtual ParseControl onMapStart() = 0;
  virtual ParseControl onMapEnd() = 0;
  virtual ParseControl onListStart() = 0;
  virtual ParseControl onListEnd() = 0;
  virtual ParseControl onNull() = 0;
  virtual ParseControl onBoolean(bool value) = 0;
  virtual ParseControl onInteger(Int64 value) = 0;
  virtual ParseControl onDecimal(double value) = 0;
  virtual ParseControl onString(const unsigned char *value, UInt32 length) = 0;

  ESB_DISABLE_AUTO_COPY(Callbacks);
};

}  // namespace AST
}  // namespace ESB

#endif
