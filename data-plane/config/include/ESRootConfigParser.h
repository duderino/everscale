#ifndef ES_ROOT_CONFIG_PARSER_H
#define ES_ROOT_CONFIG_PARSER_H

#ifndef ESB_JSON_PARSER_H
#include <ESBJsonParser.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

namespace ES {

class RootConfigParser : public ESB::JsonParser {
 public:
  RootConfigParser(ESB::Allocator &allocator);

  virtual ~RootConfigParser();

  //
  // ESB::JsonParser
  //

  virtual ParseControl onMapStart();
  virtual ParseControl onMapKey(const unsigned char *key, ESB::UInt32 length);
  virtual ParseControl onMapEnd();
  virtual ParseControl onArrayStart();
  virtual ParseControl onArrayEnd();
  virtual ParseControl onNull();
  virtual ParseControl onBoolean(bool value);
  virtual ParseControl onInteger(ESB::Int64 value);
  virtual ParseControl onDouble(double value);
  virtual ParseControl onString(const unsigned char *value, ESB::UInt32 length);

 private:
  ESB::EmbeddedList _stack;

  ESB_DEFAULT_FUNCS(RootConfigParser);
};

}  // namespace ES

#endif
